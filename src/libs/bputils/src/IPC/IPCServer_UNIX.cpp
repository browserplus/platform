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

#include "api/IPCServer.h"
#include "BPUtils/bperrorutil.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sstream>

#define SOCK_PERMS S_IRWXU

// IPC commands
#define IPCS_QUIT ((int) 0xabcd0b1e)

using namespace bp::ipc;

struct ipcServerThreadContext
{
    // the file descriptor upon which new connections come in 
    int conn_fd;
    // the file descriptor upon which control messages come in
    int control_fd;
    IServerListener * listener;
};

void *
Server::ipcServerThreadFunc(void * p)
{
    struct ipcServerThreadContext * ctx = (ipcServerThreadContext *) p;
    BPASSERT(ctx != NULL);

    // english string describing error which caused premature
    // server shutdown, if any
    const char * error = NULL;
    std::string errBuf;
    
    // why we ended the server
    IServerListener::TerminationReason whyQuit =
        IServerListener::StopCalled;

    // ye ol' select loop
    for (;;) {
        // set up select input
        int maxfd = ((ctx->conn_fd > ctx->control_fd )
                     ? ctx->conn_fd
                     : ctx->control_fd) + 1;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(ctx->conn_fd, &readfds);
        FD_SET(ctx->control_fd, &readfds);

        // block in select
        int rv = ::select(maxfd, &readfds, NULL, NULL, NULL);

        // verify no select error, if there was an error, we'll quit
        // with "InternalError"

        // TODO: Will EAGAIN be returned if non-terminal interrupts
        //       are recieved?  (i.e. SIGCONT)?
        if (rv < 0) {
            errBuf = bp::error::lastErrorString("select failed");
            whyQuit = IServerListener::InternalError;
            break;
        }
        
        // check the control pipe first, the only expected message
        // thus far is "shutdown".  Any time the control fd is hot,
        // it means we shutdown.
        if (FD_ISSET(ctx->control_fd, &readfds)) {
            int cmd = 0;
            if (sizeof(int) != read(ctx->control_fd,
                                    (void *)&cmd, sizeof(cmd)))
            {
                errBuf = "Couldn't read() from control channel";
                whyQuit = IServerListener::InternalError;
            }
            else if(cmd != IPCS_QUIT) 
            {
                error = "Unexpected command read from control channel";
                whyQuit = IServerListener::InternalError;
            }
            else
            {
                whyQuit = IServerListener::StopCalled;
            }
            break;
        }
        
        // now let's see if there's a new connection available
        if (FD_ISSET(ctx->conn_fd, &readfds)) {
            // accept the connection                                    
            int clifd;
            unsigned len;
            struct sockaddr_un cli_addr;
            len = sizeof(cli_addr);
            clifd = accept(ctx->conn_fd, (struct sockaddr *) &cli_addr,
                           &len);
            
            if (clifd < 0) {
                errBuf = bp::error::lastErrorString("accept fails on STREAM pipe");
                whyQuit = IServerListener::InternalError;
                break;
            }

            // now we have a new client fd.  let's allocate and return
            // a new connection object for it
            Connection * c = new Connection;
            if (!c->connect(clifd)) {
                delete c;
                error = "bp::ipc::Connection::connect(int) failed";
                whyQuit = IServerListener::InternalError;
                break;
            }
            // instruct our listener, she now owns the Connection
            // memory
            if (ctx->listener) {
                ctx->listener->gotConnection(c);
            } else {
                delete c;
            }
        }
    }

    // instruct our listener that we're going down
    // as a convenience, above code may populate either error or
    // errBuf.
    if (error == NULL && !errBuf.empty()) error = errBuf.c_str();
    if (ctx->listener) {
        ctx->listener->serverEnded(whyQuit, error);
    }
    
    // clean up
    delete ctx;

    return NULL;
}

Server::Server()
    : m_listener(NULL), m_fd(0), m_running(false)
{
    m_control[0] = m_control[1] = 0;
}

Server::~Server()
{
    stop();
}

bool
Server::setListener(IServerListener * listener)
{
    BPASSERT(!m_running);
    if (!m_running) m_listener = listener;
    return !m_running;
}

bool
Server::start(const std::string & location,
              std::string * error)
{
    if (m_running) return false;
    
    if (location.empty()) return false;

    const char * path = location.c_str();
    m_path = location;
    
    // may fail if path DNE.  we don't care.
    ::unlink(path);

    // allocate a socket
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        if (error) {
            *error = bp::error::lastErrorString("socket() failed");
        }
        return false;
    }

    // bind the socket to the specified path
    struct sockaddr_un unix_addr;
    if (strlen(path) > sizeof(unix_addr.sun_path)) {
        *error = "location too long";
        return false;
    }
    memset((void *) &unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    ::strcpy(unix_addr.sun_path, path);

    
    int len =
#ifndef LINUX
        sizeof(unix_addr.sun_len) +
#endif
        sizeof(unix_addr.sun_family) + strlen(unix_addr.sun_path) + 1;

    if (::bind(fd, (struct sockaddr *) &unix_addr, len) < 0) {
        close(fd);
        if (error) {
            *error = bp::error::lastErrorString("bind() failed");
        }
        return false;
    }
    
    ::chmod(unix_addr.sun_path, SOCK_PERMS);

    if (::listen(fd, 10) < 0) {
        if (error) {
            *error = bp::error::lastErrorString("listen() failed");
        }
        close(fd);
        return false;
    }

    // now, let's attain a socket pair for a control channel
    if (0 != ::pipe(m_control)) {
        if (error) {
            *error = bp::error::lastErrorString("pipe() failed");
        }
        close(fd);
        return false;
    }

    // great, and finally, allocate some context, and spawn a thread
    ipcServerThreadContext * ctx = new ipcServerThreadContext;
    ctx->control_fd = m_control[0];
    ctx->conn_fd = fd;
    ctx->listener = m_listener;
    if (!m_thread.run(ipcServerThreadFunc, (void *) ctx)) {
        delete ctx;
        close(m_control[0]);
        close(m_control[1]);        
        close(fd);
        m_control[0] = m_control[1] = 0;
        if (error) {
            *error = bp::error::lastErrorString("pipe() failed");
        }
        return false;
    }

    // running indicates that things have been successfully started,
    // and need to be cleaned up
    m_running = true;

    // keep track of listening file descriptor to clean up in stop()
    m_fd = fd;
    
    return true;
}

void
Server::stop()
{
    if (m_running) {
        int cmd = IPCS_QUIT;
        (void) write(m_control[1], (void *)&cmd, sizeof(cmd));
        m_thread.join();
        close(m_fd); close(m_control[0]); close(m_control[1]);
        m_fd = m_control[0] = m_control[1] = 0;
        m_running = false;
        ::unlink(m_path.c_str());
    }
}
