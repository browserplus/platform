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

#include "api/IPCChannelServer.h"

using namespace bp::ipc;

void
ChannelServer::deliverChannel(void * ctx)
{
    std::pair<Connection *, ChannelServer *> * p =
        (std::pair<Connection *, ChannelServer *> *) ctx;
    Channel * chan = new Channel(p->first);
    p->second->m_listener->gotChannel(chan); 
    delete p;
}

void
ChannelServer::gotConnection(Connection * c)
{
    std::pair<Connection *, ChannelServer *> * p =
        new std::pair<Connection *, ChannelServer *>(c, this);
    m_hopper.invokeOnThread(deliverChannel, (void *) p);
}

ChannelServer::ChannelServer()
{
    m_hopper.initializeOnCurrentThread();
}

ChannelServer::~ChannelServer()
{
    stop();
}

bool
ChannelServer::setListener(IChannelServerListener * listener)
{
    if (!m_server.setListener(this)) return false;
    m_listener = listener;
    return true;
}

bool
ChannelServer::start(const std::string & location, std::string * error)
{
    return m_server.start(location, error);
}

void
ChannelServer::stop()
{
    m_server.stop();
    m_hopper.processOutstandingRequests();
}
