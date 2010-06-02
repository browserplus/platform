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

/*
 * An abstraction which opens up a named IPC channel that Services,
 * once spawned, will connect to.  Primary role in live is to listen
 * for incomming established IPC channels and to route them back to
 * to the correct listener.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/15
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "ServiceServer.h"
#include <sstream>
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"

using namespace std;
using namespace std::tr1;
using namespace ServiceRunner;

Connector::Connector() : m_server(new bp::ipc::ChannelServer)
{
    m_server->setListener(this);
    std::string err;
    m_ipcName = bp::paths::getEphemeralIPCName();

    if (!m_server->start(m_ipcName, &err))
    {
        // yikes!  that's fatal, we can't start our server
        std::stringstream ss;
        ss << "can't start ServiceConnector which listens on '"
           << m_ipcName
           << "' for spawned services to connect: "
           << err;
        BP_THROW_FATAL(ss.str());
    } else {
        BPLOG_INFO_STRM("server running on: " << m_ipcName);
    }
}

Connector::~Connector()
{
    m_server->stop();
}

void
Connector::setListener(weak_ptr<Controller> controller)
{
    m_listener = controller;
}

void
Connector::gotChannel(bp::ipc::Channel * c)
{
    BPLOG_DEBUG("received service connection, waiting for 'loaded' mesage");
    c->setListener(this);
    m_establishedChannel = c;
    // stop the server immediately, we're a one shot chicken
    m_server->stop();
}

        
void
Connector::serverEnded(bp::ipc::IServerListener::TerminationReason,
                       const char * err) const
{
	BPLOG_ERROR_STRM("Service IPC listening channel fell down: " 
		             << (err ? err : ""));

    // TODO: clean up and inform our listener!
}

void
Connector::channelEnded(bp::ipc::Channel *,
                        bp::ipc::IConnectionListener::TerminationReason,
                        const char *)
{
    BPLOG_ERROR("Service IPC channel ended unexpectedly");

    // TODO: we should inform our listener!
}

void
Connector::onMessage(bp::ipc::Channel * c,
                     const bp::ipc::Message & m)
{
    // verify the channel is what we expect
    if (c != m_establishedChannel) {
        BPLOG_ERROR_STRM("incoming messange on untracked channel, deleting. "
                         "This is an internal inconsistency.");
        c->disconnect();
        delete c;
        return;
    }

    m_establishedChannel = NULL;

    // now we own Channel 'c'.  we must either pass it off or clean it up
    // before returning from this function.
    // if 'c' is not set to NULL by the end of the function, we'll assume
    // an error occured and we must clean up the channel
    if (!m.command().compare("loaded"))
    {
        // cool, we got a loaded message, payload is a map
        // containing service and version, we'll extract that and
        // use it to call back into the appropriate Controller

        // first, validate
        if (!m.payload() ||
            !m.payload()->has("service", BPTString)  || 
            !m.payload()->has("version", BPTString) ||
            !m.payload()->has("apiVersion", BPTInteger))
        {
            BPLOG_ERROR_STRM("received malformed 'loaded' message from "
                             "service" << m.command());
        } else {
            std::string service = std::string(*(m.payload()->get("service")));
            std::string version = std::string(*(m.payload()->get("version")));
            unsigned int apiVersion =
                (unsigned int) (long long) *(m.payload()->get("apiVersion"));

            // do we have a listemer?
            shared_ptr<Controller> controller = m_listener.lock();

            if (controller == NULL)
            {
                // uh oh.  the controller who allocated this service
                // has gone away.
                BPLOG_WARN_STRM("instance of " << service << " v" << version
                                << " connected, but no listener "
                                << "exists.  cleaning up instance.");
            }
            else 
            {
                // wooty woot.  let's let the controller know dinner is
                // served, and a hot and tasty spawned service instance
                // is on the plate.  
                controller->onConnected(c, service, version, apiVersion);
                
                // we've successfully transfered ownership.
                c = NULL;
            }
        }
    }
    else 
    {
        BPLOG_ERROR_STRM("received unexpected message from service: "
                         << m.command());
    }

    // c will be non-null only in the case we couldn't handle the connection
    if (c != NULL) {
        BPLOG_ERROR_STRM("cleaning up unwanted service IPC connection");
        c->disconnect();
        delete c;
    }
}

bool
Connector::onQuery(bp::ipc::Channel *,
                   const bp::ipc::Query & query,
                   bp::ipc::Response &)
{
	BPLOG_ERROR_STRM("Got unexpected (early) query from service: "
		             << query.command());
    return false;
}

void
Connector::onResponse(bp::ipc::Channel *,
                      const bp::ipc::Response & response)
{
	BPLOG_ERROR_STRM("Got unexpected (early) response from service: "
		             << response.command());
}

