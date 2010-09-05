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

#ifndef __IPCCHANNELSERVER_H__
#define __IPCCHANNELSERVER_H__

#include "IPCServer.h"
#include "IPCChannel.h"
#include "BPUtils/bpthreadhopper.h"

namespace bp { namespace ipc
{
    class IChannelServerListener {
      public:
        virtual void gotChannel(Channel * c) = 0;
        virtual void serverEnded(
            bp::ipc::IServerListener::TerminationReason,
            const char *) const { };
        virtual ~IChannelServerListener() { }
    };

    class ChannelServer : public Server,
                                      public IServerListener {
      public:
        ChannelServer();
        ~ChannelServer();        

        // set the listener for the server.  must be called before start()
        // has been called, and may not be changed after.
        bool setListener(IChannelServerListener * listener);
        bool start(const std::string & location,
                   std::string * error = NULL);
        void stop();    
      private:
        Server m_server;
        IChannelServerListener * m_listener;
        void gotConnection(Connection * c);
        bp::thread::Hopper m_hopper;
        static void deliverChannel(void * ctx);
    };
} };

#endif
