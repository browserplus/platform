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

#include "ActiveSession.h"
#include "BPUtils/IPCChannelServer.h"
#include "ServiceManager/ServiceManager.h"

#ifndef __SESSIONMANAGER_H__
#define __SESSIONMANAGER_H__

// A class which listens for events from the SMMSessionServer object
class SessionManager : public virtual bp::ipc::IChannelServerListener,
                       public virtual IActiveSessionListener
{
public:
    SessionManager(std::tr1::shared_ptr<ServiceRegistry> registry);
    ~SessionManager();

    unsigned int numCurrentSessions();
    std::vector<std::tr1::shared_ptr<ActiveSession> > currentSessions();
    
private:
    // IActiveSessionListener::onSessionEnd()
    void onSessionEnd(bp::ipc::Channel * channel);

    // IChannelServerListener::gotChannel()
    void gotChannel(bp::ipc::Channel * cptr);

    // IChannelServerListener::serverEnded()
    virtual void serverEnded(
        bp::ipc::IServerListener::TerminationReason tr,
        const char *e ) const 
	{ 
        BPLOG_WARN_STRM("IPC Server ended: ("
                        << tr << ") " << (e ? e : "")); 
	};

    std::map<bp::ipc::Channel *, std::tr1::shared_ptr<ActiveSession> >
        m_sessionList;

    std::tr1::shared_ptr<ServiceRegistry> m_serviceRegistry;

    std::string m_primaryDistroServer;
    std::list<std::string> m_secondaryDistroServers;
};

#endif
