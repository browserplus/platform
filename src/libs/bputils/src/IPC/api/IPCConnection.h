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

/**
 *  IPCConnection.h - low level abstractions to move buffers between
 *                    processes running on the same machine. 
 */ 


#ifndef __IPCCONNECTION_H__
#define __IPCCONNECTION_H__

#include <string>

#include "BPUtils/bpthread.h"

#ifdef WIN32
// for definition of HANDLE, private data.
#include <wtypes.h>
#endif

namespace bp { namespace ipc {

class IConnectionListener {
  public:
    enum TerminationReason {
        // there was an unexpected protocol error that caused us
        // to terminate.
        ProtocolError,
        // Connection::stop() was called
        DisconnectCalled,
        // peer closed the connection
        PeerClosed,
        // there was an unexpected internal error in the connection.  See
        // (or log) errorString for more information.
        InternalError
    };

    static std::string terminationReasonToString(TerminationReason tr);

    virtual void gotMessage(const class Connection * c,
                            const unsigned char * msg,
                            unsigned int msg_len) = 0;

    virtual void connectionEnded(class Connection * c,
                                 TerminationReason why,
                                 const char * errorString) = 0;

    virtual ~IConnectionListener() { }
};

class Connection {
  public:
    // maximum allowable message length (4mb)
    static const unsigned int MaxMessageLength;

    Connection();
    virtual ~Connection();

    // the two sendMessage overloads are equally efficient, however the
    // latter doesn't require the construction of a std::string if
    // you have raw data allocated already.
    bool sendMessage(const std::string & message) const;
    bool sendMessage(const unsigned char * msg, unsigned int msg_len) const;

    virtual bool setListener(IConnectionListener * listener);
    virtual bool connect(const std::string & location, std::string * error = NULL);
    void disconnect();    
  private:
    // the Server uses this hook to wrap connections around incoming
    // file descriptors/HANDLEs
    bool connect(int fd, std::string * error = NULL);    
    friend class Server;

#ifdef WIN32
    HANDLE m_stopEvent;
    HANDLE m_pipeHand;
#else
    // a control channel for communicating with the running thread
    // (sockpair)
    int m_control[2];
    // the file descriptor for the connection
    int m_fd;
#endif

    // whether the thread is running or not 
    bool m_running;

    // the thread that does the selecting and the function that it runs,
    // and a function to start everything up
    bp::thread::Thread m_thread;
    static void * ipcConnectionThreadFunc(void *);
    bool startThread(std::string * error = NULL);

    IConnectionListener * m_listener;
};

} };

#endif
