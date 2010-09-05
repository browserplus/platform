/**
 * ***** BEGIN LICENSE BLOCK *****
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is BrowserPlus (tm).
 * 
 * The Initial Developer of the Original Code is Yahoo!.
 * Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

#include "SessionManager.h" 
#include "ActiveSession.h" 
#include "platform_utils/bpconfig.h"
#include "platform_utils/ProductPaths.h"

using namespace std;
using namespace std::tr1;


SessionManager::SessionManager(shared_ptr<ServiceRegistry> registry)
{
    m_serviceRegistry = registry;

    // Setup a config file reader.
    bp::config::ConfigReader configReader;
    {
        bp::file::Path configFilePath = bp::paths::getConfigFilePath();
        if (!configReader.load(configFilePath)) {
            BP_THROW_FATAL("couldn't read config file");
            return;
        }
    }
    if (!configReader.getStringValue("DistServer", m_primaryDistroServer))
    {
        BP_THROW_FATAL("no 'DistServer' key in config file");
    }
    
    (void) configReader.getArrayOfStrings("SecondaryDistServers",
                                          m_secondaryDistroServers);
}

SessionManager::~SessionManager()
{
}
    
void
SessionManager::onSessionEnd(bp::ipc::Channel * channel)
{
    std::map<bp::ipc::Channel * , shared_ptr<ActiveSession> >::iterator
        kill;
    kill = m_sessionList.find(channel);
    if (kill != m_sessionList.end()) {
        m_sessionList.erase(kill);
    }
    BPLOG_INFO_STRM("Session ended: " << channel);
}

void
SessionManager::gotChannel(bp::ipc::Channel * cptr) 
{
    BPLOG_DEBUG_STRM("new connection established: " << cptr);

    shared_ptr<ActiveSession> activeSession(
        new ActiveSession(cptr, m_serviceRegistry, m_primaryDistroServer,
                          m_secondaryDistroServers));
    cptr->setListener(activeSession.get());
    activeSession->setListener(this);
    
    m_sessionList[cptr] = activeSession;
}


unsigned int
SessionManager::numCurrentSessions()
{
    return m_sessionList.size();
}

std::vector<shared_ptr<ActiveSession> >
SessionManager::currentSessions()
{
    std::vector<shared_ptr<ActiveSession> > l;
    
    std::map<bp::ipc::Channel *, shared_ptr<ActiveSession> >::iterator it;
    for (it = m_sessionList.begin(); it != m_sessionList.end(); it++)
    {
        l.push_back(it->second);
    }

    return l;
}
