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

#include "BPUtils/IPCConnection.h"
#include "BPUtils/bpthread.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"

#include <assert.h>
#include <string>
#include <sstream>
#include <windows.h>

using namespace bp::ipc;

struct ipcConnectionThreadContext
{
    // the handle upon which messages come in 
    HANDLE pipeHand;
    // the handle which signifies our run is complete
    HANDLE stopEventHand;

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
    // Overlapped reads are a headache.  This state manages the
    // correct flow.
    enum {
        State_None,     // no overlapped read event has been started
        State_Header,   // we've started the read event for the header
        State_Body      // we've started the read event for the body
    } state = State_None;

    // english string describing error which caused premature
    // connection shutdown, if any
    std::string error;
    
    // why we ended the server
    IConnectionListener::TerminationReason whyQuit =
        IConnectionListener::DisconnectCalled;

    // an OVERLAPPED object that we'll used for async reading
    OVERLAPPED olapReadObj;
    memset((void *) &olapReadObj, 0, sizeof(olapReadObj));
    olapReadObj.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    // ye ol' select loop
    for (;;) {
        // set up WaitForMultipleObjects input
        HANDLE handles[2];
        handles[0] = ctx->stopEventHand;

        // now manage "starting" the overlapped read event
        if (state == State_None)
        {
            // we must start a read for the header
            BOOL rv = ReadFile(ctx->pipeHand, (LPVOID) &cur_msg_len,
                               sizeof(cur_msg_len), NULL, &olapReadObj);
            if (!rv) {
                DWORD err = GetLastError();
                if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE) {
                    whyQuit = IConnectionListener::PeerClosed;
                    break;
                } else if (err != ERROR_IO_PENDING) {
                    whyQuit = IConnectionListener::InternalError;
                    error = bp::error::lastErrorString("ReadFile failed");
                    break;
                }
                // guten.  now windows knows we're waiting for a message
                // header.
            }
            state = State_Header;
        }
        else if (state == State_Header || state == State_Body)
        {
            // noop!
        }
        handles[1] = olapReadObj.hEvent;

        // block in WaitForMultipleObjects
        DWORD rv = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (rv == WAIT_OBJECT_0) {
            // someone called stop
            ResetEvent(ctx->stopEventHand);
            CancelIo(ctx->pipeHand);
            whyQuit = IConnectionListener::DisconnectCalled;
            break;
        } else if (rv == WAIT_OBJECT_0 + 1) {
            // our read handle is hot
            if (state == State_Header) {
                // read the header and start the body read
                DWORD rd = 0;
                BOOL rv = GetOverlappedResult(
                    ctx->pipeHand, &olapReadObj, &rd, FALSE);

                // this should always succeed!
                if (!rv) {
                    DWORD err = GetLastError();
                    if (err == ERROR_IO_INCOMPLETE) {
                        // spurious wakeups may result from writes during
                        // reads in a multi core environment
                        // (YIB-2216320)
                        continue;
                    } else if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE) {
                        whyQuit = IConnectionListener::PeerClosed;
                        break;
                    } else {
                        whyQuit = IConnectionListener::InternalError;
                        error = bp::error::lastErrorString(
                            "GetOverlappedResult fails");
                        break;
                    }
                } else if (rd != sizeof(cur_msg_len)) {
                    whyQuit = IConnectionListener::InternalError;
                    error = bp::error::lastErrorString(
                        "GetOverlappedResult fails, partial header read");
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

                // now cur_msg_len is populated, grow our buffer if
                // it needs it.
                if (buf_len < cur_msg_len) {
                    if (buf) free(buf);
                    buf = (unsigned char *) malloc(cur_msg_len);
                    // memory allocation never fails anymore, right?
                    // 4 gigs of main!  virtual memory all over!
                    // yeah, ok, embedded devices.  runaway programs.
                    assert(buf != NULL);
                    buf_len = cur_msg_len;
                }
                // now it's time to start reading the body
                state = State_Body;
            } else if (state == State_Body) {
                // the handle is hot while reading the body.  
                assert(buf_len >= cur_msg_len);
                assert(cur_msg_read < cur_msg_len);

                // get the results of the read
                DWORD rd = 0;
                BOOL rv = GetOverlappedResult(
                    ctx->pipeHand, &olapReadObj, &rd, FALSE);
                if (!rv) {
                    DWORD err = GetLastError();
                    if (err == ERROR_IO_INCOMPLETE) {
                        // spurious wakeups may result from writes during
                        // reads in a multi core environment
                        // (YIB-2216320)
                        continue;
                    }
                    whyQuit = IConnectionListener::InternalError;
                    error = bp::error::lastErrorString(
                        "GetOverlappedResult fails (reading body)");
                    break;
                }

                // now update cur_msg_read
                cur_msg_read += rd;

                assert(cur_msg_read <= cur_msg_len);
                
                if (cur_msg_read >= cur_msg_len) {
                    // all done reading this message!
                    ctx->listener->gotMessage(ctx->conn, buf, cur_msg_read);
                    cur_msg_len = cur_msg_read = 0;
                    state = State_None;
                }
            } else {
                // internal state error
                whyQuit = IConnectionListener::InternalError;
                error = bp::error::lastErrorString(
                    "state machine is broken, get a wrench");
                break;
            }

            // if, at this point, we're in State_Body, it means we've
            // just read headers or a portion of the body and it's time
            // to start a new overlapped read for the remainder of the
            // body (which may be the whole thing)
            if (state == State_Body) {
                BOOL rv = ReadFile(ctx->pipeHand, 
                                   (LPVOID) (buf + cur_msg_read),
                                   cur_msg_len - cur_msg_read,
                                   NULL, &olapReadObj);
                if (!rv) {
                    DWORD err = GetLastError();
                    if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
                    {
                        whyQuit = IConnectionListener::PeerClosed;
                        break;
                    }
                    else if (err != ERROR_IO_PENDING)
                    {
                        whyQuit = IConnectionListener::InternalError;
                        error = bp::error::lastErrorString("ReadFile failed");
                        break;
                    }
                }
            }
        } else {
            whyQuit = IConnectionListener::InternalError;
            error = bp::error::lastErrorString(
                "WaitForMultipleObjects failed");
            break;
        }
    }

    // instruct our listener that we're going down
    // we cast away const here, the client owns it, she may do as she
    // likes.  We're only promising that WE won't do evil.
    ctx->listener->connectionEnded((Connection *) ctx->conn,
                                   whyQuit,
                                   error.length() ? error.c_str() : NULL);

    // clean up
    CloseHandle(olapReadObj.hEvent);
    if (buf) free(buf);
    delete ctx;

    return NULL;
}

Connection::Connection()
    : m_stopEvent(0), m_pipeHand(0), m_running(false), m_listener(NULL)
{
    m_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}
     
Connection::~Connection()
{
    disconnect();
    CloseHandle(m_stopEvent);
}

bool
Connection::sendMessage(const std::string & message) const
{
	return sendMessage((const unsigned char *) message.c_str(), message.length());
    
}

bool
Connection::sendMessage(const unsigned char * msg, unsigned int msg_len) const
{
    // time to write. We're going to block until all data is written.
    // see ICPConnection_UNIX comments for motivation of this,
    // perhaps innefficient, blocking design.
    if (msg == NULL || msg_len == 0) return false;
    if (m_pipeHand == 0) return false;

    OVERLAPPED olapWrite;
    memset((void *) &olapWrite, 0, sizeof(olapWrite));
    olapWrite.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    bool status = true;

    // first write the "message header"
    BOOL rv = WriteFile(m_pipeHand, (LPVOID) &msg_len, sizeof(msg_len),
                        NULL, &olapWrite);
    if (!rv) {
        DWORD err = GetLastError();
        if (err != ERROR_IO_PENDING) {
            status = false;
        } else {
            DWORD wt = 0;
            rv = GetOverlappedResult(m_pipeHand, &olapWrite, &wt, TRUE);
            if (!rv || wt != sizeof(msg_len)) {
                status = false;
            }
        }
    }

    // if we got through the header, it's time for the body
    if (status) {
        rv = WriteFile(m_pipeHand, (LPVOID) msg, msg_len,
                       NULL, &olapWrite);
        if (!rv) {
			DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                status = false;
            } else {
                DWORD wt = 0;
                rv = GetOverlappedResult(m_pipeHand, &olapWrite, &wt, TRUE);
                if (!rv || wt != msg_len) {
                    status = false;
                }
            }
        }
    }
    
	return status;
}


bool
Connection::setListener(IConnectionListener * listener)
{
    // may not change a listener while running
    if (m_running) return false;
    m_listener = listener;

    // if we're already connected but not running, this is the
    // golden moment for thread spawning
    bool rv = true;
    if (m_listener != NULL && !m_running && m_pipeHand != 0)
    {
        // TODO: gracefully handle thread spawn failure
        rv = startThread();
    }
    return rv;
}

bool
Connection::connect(const std::string & location, std::string * errorStr)
{
    HANDLE pipeHand = 0;

    std::string pipeName;
    pipeName.append("\\\\.\\pipe\\");
    pipeName.append(location);
    std::wstring pipeNameW = bp::strutil::utf8ToWide(pipeName);

    pipeHand = CreateFileW( 
        pipeNameW.c_str(),            // pipe name 
        GENERIC_READ | GENERIC_WRITE, // read and write access 
        0,                            // no sharing 
        NULL,                         // default security attributes
        OPEN_EXISTING,                // opens existing pipe 
        FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
        NULL); // no template file 
 
    // Break if the pipe handle is valid. 
    if (pipeHand == INVALID_HANDLE_VALUE)
    {
        if (errorStr) {
            *errorStr = bp::error::lastErrorString("CreateFileW() fails");
        }
        return false;
    }

    return connect((int) pipeHand, errorStr);
}

bool
Connection::connect(int fd, std::string * error)
{
    if (fd == 0) return false;
    m_pipeHand = (HANDLE) fd;

    // now we've got a connected file descriptor.  we will start up
    // the selecting thread iff setListener has been called.  This
    // is important in the Server case, where bp::ipc::Server accepts
    // an incoming fd, builds up a Connection, and passes it to a client
    // if we were to spawn the thread now, in that case, there may
    // be incoming data and no listener for that data.  We let the
    // data sIit in the connection until the client has set a listener.
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
    if (m_running) {
        BOOL x = SetEvent(m_stopEvent);
        assert(x);
        x = false;
        m_thread.join();
        CloseHandle(m_pipeHand);
        m_pipeHand = 0;
        m_running = false;
    }
}

bool
Connection::startThread(std::string * error)
{
    // ensure the world is sane
    assert(m_stopEvent != 0);
    assert(m_pipeHand != 0);
    assert(!m_running);
    assert(m_listener != NULL);

    // allocate some context, and spawn a thread
    ipcConnectionThreadContext * ctx = new ipcConnectionThreadContext;
    ctx->stopEventHand = m_stopEvent;
    ctx->pipeHand = m_pipeHand;
    ctx->listener = m_listener;
    ctx->conn = this;
    
    if (!m_thread.run(ipcConnectionThreadFunc, (void *) ctx)) {
        *error = "thread spawn failed";
        delete ctx;
        CloseHandle(m_pipeHand);
        m_pipeHand = 0;
        return false;
    }

    m_running = true;

    return true;
}

