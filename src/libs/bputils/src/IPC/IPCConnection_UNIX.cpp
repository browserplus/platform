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

#include "api/IPCConnection.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/BPLog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include <sstream>

#define IPCC_QUIT ((int) 0x1234b1e0)

using namespace bp::ipc;

struct ipcConnectionThreadContext
{
    // the file descriptor upon which messages come in 
    int conn_fd;
    // the file descriptor upon which control information comes in
    int control_fd;
    IConnectionListener * listener;
    // only need this to pass to the client
    const Connection * conn;
};

void *
Connection::ipcConnectionThreadFunc(void * p)
{
    struct ipcConnectionThreadContext * ctx = (ipcConnectionThreadContext *) p;
    assert(ctx != NULL);

    // buffer for reading messages.  grows to largest message size,
    // freed at end of connection.  This does mean one large message
    // can increase memory usage for the duration of the connection.
    // it also means that there's minimal memory reallocation during
    // a connection.  twelve of these, a dozen of the other.
    unsigned char * buf = NULL;
    unsigned int    buf_len = 0;    
    // size of current message, and amount read thus far
    unsigned int    cur_msg_len = 0;
    unsigned int    cur_msg_read = 0;    

    // english string describing error which caused premature
    // connection shutdown, if any
    std::string error;
    
    // why we ended the server
    IConnectionListener::TerminationReason whyQuit =
        IConnectionListener::DisconnectCalled;

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

        // TODO: Can a non-terminal signal cause an error return
        //       with EAGAIN?
        if (rv < 0) {
            error = bp::error::lastErrorString("select failed");
            whyQuit = IConnectionListener::InternalError;
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
                error.append("Couldn't read() from control channel");
                whyQuit = IConnectionListener::InternalError;
            }
            else if(cmd != IPCC_QUIT) 
            {
                error = "Unexpected command read from control channel";
                whyQuit = IConnectionListener::InternalError;
            }
            else
            {
                whyQuit = IConnectionListener::DisconnectCalled;
            }
            break;
        }
        
        // now let's see if there's data available
        if (FD_ISSET(ctx->conn_fd, &readfds)) {
            // time to read data, is this hot fd a new message?
            if (cur_msg_len == 0) {
                int x = read(ctx->conn_fd, (void *) &cur_msg_len, sizeof(int));
                // is this EOF?
                if (x == 0) {
                    whyQuit = IConnectionListener::PeerClosed;
                    break;
                } else if (x < 0) {
                    // read error.  curious case.
                    whyQuit = IConnectionListener::InternalError;
                    error = bp::error::lastErrorString("read failed");
                    break;
                } else if (x != sizeof(int)) {
                    // read error.  curious case.
                    whyQuit = IConnectionListener::InternalError;
                    error = "unhandled partial read";
                    break;
                } else if (cur_msg_len == 0) {
                    // dumb peer, wrote 0
                    whyQuit = IConnectionListener::ProtocolError;
                    error = "peer wrote zero length message";
                    break;
                } else if (cur_msg_len > MaxMessageLength) {
                    whyQuit = IConnectionListener::ProtocolError;
                    std::stringstream ss;
                    ss << "IPC message too large: "
                       << cur_msg_len << " bytes is greater than max of "
                       << MaxMessageLength << " bytes";
                    error = ss.str();
                    break;
                }

                // now let's ensure we have enough buffer
                if (buf_len < cur_msg_len) {
                    if (buf) free(buf);
                    buf = (unsigned char *) malloc(cur_msg_len);
                    // memory allocation never fails anymore, right?
                    // 4 gigs of main!  virtual memory all over!
                    // yeah, ok, embedded devices.  runaway programs.
                    assert(buf != NULL);
                    buf_len = cur_msg_len;
                }
            }

            // at this point, cur_msg_len must be populated, and
            // valid, and must be greater than cur_msg_read
            assert(cur_msg_len > cur_msg_read);
            // and our buffer must be allocated
            assert(buf_len >= cur_msg_len);

            // now let's do some message reading!
            {
                int x = read(ctx->conn_fd,
                             (void *) (buf + cur_msg_read),
                             cur_msg_len - cur_msg_read);
                
                if (x == 0) {
                    whyQuit = IConnectionListener::PeerClosed;
                    break;
                } else if (x < 0) {
                    whyQuit = IConnectionListener::InternalError;
                    error = bp::error::lastErrorString("read failed");
                    break;
                }
                
                cur_msg_read += x;
            }
            
            // now if we've completed reading the message,
            // alert the client and re-initialize read state
            if (cur_msg_read >= cur_msg_len) {
                assert(cur_msg_read == cur_msg_len);
                ctx->listener->gotMessage(ctx->conn, buf, cur_msg_read);
                cur_msg_len = cur_msg_read = 0;

                // TODO: It would be trivial to add code here to free
                //   the buffer if it's larger than we're comfortable
                //   with.  idea there would be we grow to a certain
                //   point, then allocate for (hopefully) rare large
                //   messages.  In fact, the code would be fewer lines
                //   than this comment.
            }
        }
    }

    // instruct our listener that we're going down
    // we cast away const here, the client owns it, she may do as she
    // likes.  We're only promising that WE won't do evil.
    ctx->listener->connectionEnded((Connection *) ctx->conn,
                                   whyQuit,
                                   error.length() ? error.c_str() : NULL);

    // clean up
    if (buf) free(buf);
    delete ctx;

    return NULL;
}


Connection::Connection()
    : m_fd(0), m_running(false), m_listener(NULL)
{
    m_control[0] = m_control[1] = 0;
}
        
Connection::~Connection()
{
    disconnect();
}
        
bool
Connection::sendMessage(const std::string & message) const
{
    return sendMessage((const unsigned char *) message.c_str(),
                       message.length());
}

bool
Connection::sendMessage(const unsigned char * msg, unsigned int msg_len) const
{
    // TODO: explicitly deal with invocation from a different thread.

    // woohoo, time to write some data!
    if (msg == NULL || msg_len == 0) return false;
    if (m_fd == 0) return false;

    // we block and write all data.  write buffers should be large,
    // and large messages should be rare.  The alternative would
    // be to copy the client's memory and pass it over to the
    // connection thread.  However in the common case that would
    // be equally obnoxious in terms of memory use.  And a hybrid
    // approach would be premature optimization.  foo.

    int x = write(m_fd, (void *) &msg_len, sizeof(msg_len));    
    if (x != sizeof(msg_len)) return false;

    x = write(m_fd, msg, msg_len);
    return ((unsigned int) x == msg_len);
}

bool
Connection::setListener(IConnectionListener * listener)
{
    if (m_running) return false;
    m_listener = listener;

    // if we're already connected but not running, this is the
    // golden moment for thread spawning
    bool rv = true;
    if (!m_running && m_fd != 0) {
        // TODO: Gracefully deal with thread spawn failure
        rv = startThread();
    }
    return rv;
}
        
bool
Connection::connect(const std::string & location, std::string * error)
{
    const char * path;

    if (location.empty()) return false;

    path = location.c_str();

    /* first let's allocate a stream pipe */
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        if (error) {
            *error = bp::error::lastErrorString("socket allocation failed");
        }
        return false;
    }
    
    struct sockaddr_un unix_addr;
    if (strlen(path) > sizeof(unix_addr.sun_path)) {
        *error = "location too long";
        return false;
    }
    memset((void *) &unix_addr, 0, sizeof(unix_addr));
    unix_addr.sun_family = AF_UNIX;
    ::strcpy(unix_addr.sun_path, path);
    
    int len;
    len =
#ifndef LINUX
        sizeof(unix_addr.sun_len) +
#endif
        sizeof(unix_addr.sun_family) + strlen(unix_addr.sun_path) + 1;

    if (::connect(fd, (struct sockaddr *) &unix_addr, len) < 0)
    {
        ::close(fd);
        if (error) {
            *error = bp::error::lastErrorString("connect failed");
        }
        return false;
    }

    return connect(fd, error);
}

bool
Connection::connect(int fd, std::string * error)
{
    if (fd == 0) return false;
    m_fd = fd;
    // now we've got a connected file descriptor.  we will start up
    // the selecting thread iff setListener has been called.  This
    // is important in the Server case, where bp::ipc::Server accepts
    // an incoming fd, builds up a Connection, and passes it to a client
    // if we were to spawn the thread now, in that case, there may
    // be incoming data and no listener for that data.  We let the
    // data sit in the connection until the client has set a listener.
    // (which should happen immediately, otherwise the client is silly
    // or there's something I haven't thought of.  the latter is
    // certainly possible).
    bool rv = true;
    if (m_listener != NULL) rv = startThread(error);
    return rv;
}
        
void
Connection::disconnect()
{
    // TODO: explicitly deal with invocation from a different thread.

    if (m_running) {
        int cmd = IPCC_QUIT;
        (void) write(m_control[1], (void *)&cmd, sizeof(cmd));
        m_thread.join();
        m_running = false;
    }
    
    if (m_fd) close(m_fd);
    m_fd = 0;

    if (m_control[0]) close(m_control[0]);
    m_control[0] = 0;

    if (m_control[1])  close(m_control[1]);
    m_control[1] = 0;

    // TODO: is there a valid case where this will confuse clients?
    //  they shouldn't be reusing connections, even though everything
    //  is cleaned up by this point

    m_listener = NULL;
}

bool
Connection::startThread(std::string * error)
{
    // ensure the world is sane
    assert(m_control[0] == 0 && m_control[1] == 0);
    assert(m_fd != 0);
    assert(!m_running);
    assert(m_listener != NULL);

    // attain a socket pair for a control channel
    if (0 != ::pipe(m_control)) {
        std::string err = bp::error::lastErrorString(
            "control channel allocation failed [pipe(2)]");
        if (error) *error = err;
        BPLOG_ERROR_STRM("Couldn't allocate pipe: " << err);
        return false;
    }

    // allocate some context, and spawn a thread
    ipcConnectionThreadContext * ctx = new ipcConnectionThreadContext;
    ctx->control_fd = m_control[0];
    ctx->conn_fd = m_fd;
    ctx->listener = m_listener;
    ctx->conn = this;
    
    if (!m_thread.run(ipcConnectionThreadFunc, (void *) ctx)) {
        if (error) *error = "thread spawn failed";
        delete ctx;
        close(m_control[0]);
        close(m_control[1]);        
        m_control[0] = m_control[1] = 0;
        BPLOG_ERROR_STRM("thread spawn failed");
        return false;
    }

    m_running = true;

    return true;
}
