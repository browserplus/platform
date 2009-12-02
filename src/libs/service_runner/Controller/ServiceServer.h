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
 * Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
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

#ifndef __SERVICECONNECTOR_H__
#define __SERVICECONNECTOR_H__

#include <string>
#include <set>
#include "api/Controller.h"
#include "BPUtils/IPCChannelServer.h"


namespace ServiceRunner 
{
    class Connector : public bp::ipc::IChannelServerListener,
                      public bp::ipc::IChannelListener
    {
      public:
        Connector();
        ~Connector();        

        // set the controller that listens to this connector
        void setListener(std::tr1::weak_ptr<Controller> controller);

        std::string ipcName() { return m_ipcName; }
      private:
        std::tr1::weak_ptr<Controller> m_listener;
        std::tr1::shared_ptr<bp::ipc::ChannelServer> m_server;

        //////////
        // below is the implemented interface of
        // bp::ipc::IChannelServerListener
        void gotChannel(bp::ipc::Channel * c);
        void serverEnded(bp::ipc::IServerListener::TerminationReason,
                         const char *) const;
        //////////

        //////////
        // below is the implemented interface of
        // bp::ipc::IChannelListener
        // NOTE: we only implement this interface to listen for the
        //       first 'loaded' message that's sent from services
        //       once they're loaded.  at this point we may associate
        //       services correctly with the appropriate controller
        void channelEnded(
            bp::ipc::Channel * c,
            bp::ipc::IConnectionListener::TerminationReason why,
            const char * errorString);
        void onMessage(bp::ipc::Channel * c,
                       const bp::ipc::Message & m);
        bool onQuery(bp::ipc::Channel * c,
                     const bp::ipc::Query & query,
                     bp::ipc::Response & response);
        void onResponse(bp::ipc::Channel * c,
                        const bp::ipc::Response & response);
        //////////

        // a set of channels that have been established but we've not
        // yet recieved the initial 'loaded' message
        bp::ipc::Channel * m_establishedChannel;

        // the opaque "ipc name", over which which communication with 
        // services runs.
        std::string m_ipcName;
    };
};

#endif
