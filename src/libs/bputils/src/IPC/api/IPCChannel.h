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
 *  IPCChannel.h - a high level abstraction to move messages between
 *                 processes running on the same machine.  This includes
 *                 an async single threaded model supporting both 
 *                 query/repsonse and fire & forget semantics.
 */

#ifndef __IPCCHANNEL_H__
#define __IPCCHANNEL_H__

#include "BPUtils/IPCConnection.h"
#include "BPUtils/IPCChannel.h"
#include "BPUtils/IPCMessage.h"
#include "BPUtils/bpthread.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpthreadhopper.h"

#include <string>

namespace bp { namespace ipc
{
    class IChannelListener {
      public:
        // see IConnectionListener for documentation of
        // TerminationReason
        virtual void channelEnded(
            class Channel * c,
            bp::ipc::IConnectionListener::TerminationReason why,
            const char * errorString) = 0;

        // invoked when a message is recieved from the channel
        virtual void onMessage(class Channel * c,
                               const bp::ipc::Message & m) = 0;

        // invoked when a request is recieved from the channel.
        // Return value:  if onQuery returns true, the optional output
        //                parameter "response" is assumed to be populated
        //                with the response and is sent to the peer.  If
        //                if false is returned, reponse is unused and
        //                it's assumed that the client will call
        //                ipc::Channel::sendResponse at a later point,
        //                and will correctly set the response-to
        //                header.
        virtual bool onQuery(class Channel * c,
                             const bp::ipc::Query & query,
                             bp::ipc::Response & response) = 0;
                               
        virtual void onResponse(class Channel * c,
                                const bp::ipc::Response & response) = 0;

        virtual ~IChannelListener() { }
    };


    class Channel : public IConnectionListener {
      public:
        Channel();
        ~Channel();

        // connect to a "location", either a string on windows or a
        // file path on unix.  Client and server must agree.
        bool connect(const std::string & location,
                     std::string * error = NULL);
        bool setListener(IChannelListener * listener);
        void disconnect();    

        bool sendMessage(const Message & m);
        // are these useful?
        bool sendQuery(const Query & q);        
        bool sendResponse(const Response & r);

      private:
        Channel(Connection * c);
        friend class ChannelServer;
        Connection * m_conn;
        IChannelListener * m_cListener;

        // IConnectionListener
        void gotMessage(const class Connection * c,
                        const unsigned char * msg,
                        unsigned int msg_len);
        
        void connectionEnded(class Connection * c,
                             TerminationReason why,
                             const char * errorString);
        
        bp::thread::Hopper m_hopper;
        static void deliverMessageEvent(void * ctx);
        static void deliverEndedEvent(void * ctx);
    };

} };

#endif
