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

#ifndef __IPCSERVER_H__
#define __IPCSERVER_H__

#include "BPUtils/IPCConnection.h"
#include "BPUtils/bpthread.h"

#ifdef WIN32
#pragma warning ( push )
#pragma warning ( disable : 4100 )
#endif

namespace bp { namespace ipc {


class IServerListener {
  public:
    /** a callback called by bp::ipc::Server when a new connection
     *  is established.  Connection is a dynamically allocated
     *  instance that the client is responsible for freeing.
     *  this callback will be invoked on a thread spun by
     *  bp::ipc::Server, the client is responsible for
     *  synchronization of shared data structures */
    virtual void gotConnection(Connection * c) = 0;

    enum TerminationReason {
        // there was an unexpected protocol error that caused us
        // to terminate.
        ProtocolError,
        // Server::stop() was called
        StopCalled,
        // there was an unexpected internal error in the server.  See
        // (or log) errorString for more information
        InternalError
    };
    
    virtual void serverEnded(TerminationReason why,
                             const char * errorString) const { };
    virtual ~IServerListener() { }
};

class Server {
  public:
    Server();
    ~Server();

    // set the listener for the server.  must be called before start()
    // has been called, and may not be changed after.
    bool setListener(IServerListener * listener);
    bool start(const std::string & location,
               std::string * error = NULL);
    void stop();    

  private:
    // the listener, set in setListener()
    IServerListener * m_listener;

// platform specific data
#ifdef WIN32
    std::string m_pipeName;
    OVERLAPPED m_overlapped;
    HANDLE m_stopEvent;
#else
    std::string m_path;
    // the file descriptor upon which we're listening
    int m_fd;
    // a control channel for communicating with the running thread
    // (sockpair)
    int m_control[2];
#endif

    // the thread that does the selecting and the function that it runs
    bp::thread::Thread m_thread;
    static void * ipcServerThreadFunc(void *);

    bool m_running;
};

} };

#ifdef WIN32
#pragma warning ( pop )
#endif

#endif
