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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * IPCTestServer.h
 * A test IPCServer implementation which will handle a set of documented
 * "IPC commands" and behave predictably.
 *
 * Created by Lloyd Hilaiel on 7/30/08 (somewhere over the atlantic)
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "IPCTestServer.h"
#include <iostream>
#include "BPUtils/bpuuid.h"


IPCTestServer::IPCTestServer()
{
    // first we'll generate an ephemeral location 
    if (!bp::uuid::generate(m_location)) {
        BP_THROW_FATAL( "couldn't generate UUID" );
    }

    // we'll listen to the server
    m_server.setListener(this);

    // now let's connect up the server
    std::string errBuf;
    if (!m_server.start(m_location, &errBuf)) {
        BP_THROW_FATAL( "couldn't start server: " + errBuf );
    }
}

std::string
IPCTestServer::location()
{
    return m_location;
}

IPCTestServer::~IPCTestServer()
{
    m_server.stop();
    std::set<bp::ipc::Channel *>::iterator it;
    for (it = m_channels.begin(); it != m_channels.end(); it++) delete *it;
    m_channels.clear();
}

void
IPCTestServer::gotChannel(bp::ipc::Channel * c)
{
    c->setListener(this);
    m_channels.insert(c);
}

void 
IPCTestServer::serverEnded(bp::ipc::IServerListener::TerminationReason,
                           const char *)
{
}

void
IPCTestServer::channelEnded(bp::ipc::Channel * c,
                            bp::ipc::IConnectionListener::TerminationReason,
                            const char *)
{
    std::set<bp::ipc::Channel *>::iterator it;
    it = m_channels.find(c);
    if (it != m_channels.end()) m_channels.erase(it);
    delete c;
}

void
IPCTestServer::onMessage(bp::ipc::Channel *,
                         const bp::ipc::Message &)
{
}

bool
IPCTestServer::onQuery(bp::ipc::Channel *,
                       const bp::ipc::Query & query,
                       bp::ipc::Response & response)
{
    // for the "echo" command, send back the same thing we receive
    if (!query.command().compare("echo")) {
        if (query.payload()) response.setPayload(*(query.payload()));
        return true;
    }
    std::cout << "OOPS! I DON'T UNDERSTAND!" << std::endl;
    return false;
}

void
IPCTestServer::onResponse(bp::ipc::Channel *,
                          const bp::ipc::Response &)
{
}
