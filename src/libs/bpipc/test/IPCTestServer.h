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

/**
 * IPCTestServer.h
 * A test IPCServer implementation which will handle a set of documented
 * "IPC commands" and behave predictably.
 *
 * Created by Lloyd Hilaiel on 7/30/08 (somewhere over the atlantic)
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __IPCTESTSERVER_H__
#define __IPCTESTSERVER_H__

#include <set>
#include <string>
#include "bpipc/IPCChannelServer.h"
#include "BPUtils/bprunloop.h"


/**
 * IPC test server is a small IPC server class that can be used for
 * testing the IPC implementation.  The server binds to an "ephemeral"
 * local ipc "port" (really it's a named pipe or a fifo), and after
 * instantiation, you may call "location()" to get a platform specific
 * string which represents where the servier is listenening
 * (and may then be passed to "IPCChannel::connect()")
 *
 * the server handles the following commands:
 *  1. query("echo") - the server will tell you right back what you say to it.
 *  2. message("quit") - will cause the server to terminate the connection
 */
class IPCTestServer : virtual public bp::ipc::IChannelServerListener,
                      virtual public bp::ipc::IChannelListener  
{
  public:
    IPCTestServer();
    std::string location();
    ~IPCTestServer();    

    ///// IChannelServerListener /////
    void gotChannel(bp::ipc::Channel * c);
    void serverEnded(bp::ipc::IServerListener::TerminationReason why,
                     const char * errorString) const;

    ///// IChannelListener /////
    virtual void channelEnded(
        bp::ipc::Channel * c,
        bp::ipc::IConnectionListener::TerminationReason why,
        const char * errorString);
    virtual void onMessage(bp::ipc::Channel * c,
                           const bp::ipc::Message & m);
    virtual bool onQuery(bp::ipc::Channel * c,
                         const bp::ipc::Query & query,
                         bp::ipc::Response & response);
    virtual void onResponse(bp::ipc::Channel * c,
                            const bp::ipc::Response & response);
  private:
    bp::ipc::ChannelServer m_server;
    std::string m_location;
    std::set<bp::ipc::Channel *> m_channels;
};

#endif
