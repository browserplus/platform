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

#include "BPUtils/IPCServer.h"
#include "BPUtils/bpthread.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"

//#define _WIN32_WINNT 0x0500

#include <Sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <windows.h>
#include <string>
#include <assert.h>

#ifndef LABEL_SECURITY_INFORMATION
#define LABEL_SECURITY_INFORMATION (0x00000010L)
#endif

using namespace bp::ipc;

static HANDLE
createPipe(std::string pipeName, OVERLAPPED * overlap,
           bool first, std::string * error)
{
    HANDLE pipeHand = 0;

    std::wstring pipeNameW = bp::strutil::utf8ToWide(pipeName);
    pipeHand = CreateNamedPipeW(
        pipeNameW.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | WRITE_DAC |
	(first ? WRITE_OWNER : 0), 
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024 * 32,
        1024 * 32,
        1024,
        NULL);

    if (pipeHand == INVALID_HANDLE_VALUE) {
        if (error) {
            *error = bp::error::lastErrorString("Error creating named pipe");
        }
        return 0;
    }

    // Dumb dumb dumb interface.  
    BOOL connected = ConnectNamedPipe(pipeHand, overlap);
    if (!connected) {
        DWORD lastErr = GetLastError();

        if (lastErr != ERROR_IO_PENDING &&
			lastErr != ERROR_PIPE_CONNECTED)
		{
            *error = bp::error::lastErrorString("Error connecting named pipe");
			CloseHandle(pipeHand);
			pipeHand = 0;
		}
    }

    return pipeHand;
}

struct ipcServerThreadContext
{
    // the pipe and its silly eventing structure
    std::string pipeName;
    HANDLE pipeHand;
    OVERLAPPED * pipeOverlap;

    // an event triggered when it's time to stop  
    HANDLE stopEventHand;
    
    // the guy who cares.
    IServerListener * listener;
};


void *
Server::ipcServerThreadFunc(void * p)
{
    struct ipcServerThreadContext * ctx = (ipcServerThreadContext *) p;

    // english string describing error which caused premature
    // server shutdown, if any
    std::string errBuf;

    // why we ended the server
    IServerListener::TerminationReason whyQuit =
        IServerListener::StopCalled;

    for (;;) {
        // set up the handles we're waiting on
        HANDLE handles[2];
        handles[0] = ctx->stopEventHand;
        handles[1] = ctx->pipeOverlap->hEvent;

        // wait for something to happen
        DWORD rv = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        // what happened?
        if (rv == WAIT_OBJECT_0) {
            // stop was called!
            ResetEvent(ctx->stopEventHand);
            whyQuit = IServerListener::StopCalled;
            break;
        }
        else if (rv == WAIT_OBJECT_0 + 1)
        {
            // incoming connection! win32 is stupid.
            BOOL rv = ConnectNamedPipe(ctx->pipeHand, NULL);

            if (rv) {
                errBuf.append(
                    "unexpected success return from ConnectNamedPipe");
                whyQuit = IServerListener::InternalError;
                break;
            } 

            DWORD lastErr = GetLastError();
            
            if (lastErr == ERROR_IO_PENDING) {
                // noop.  we wait
            } else if (lastErr == ERROR_PIPE_CONNECTED) {
                // this error is not an error. it's success!
                HANDLE connectedPipe = ctx->pipeHand;

                // let's reallocate the listening pipe
                ctx->pipeHand =
                    createPipe(ctx->pipeName, ctx->pipeOverlap, false, &errBuf);
                if (ctx->pipeHand == 0) {
                    whyQuit = IServerListener::InternalError;
                    break;
                }

                // now let's deliver the connected client to the listener
                Connection * c = new Connection;
                if (!c->connect((int) connectedPipe, &errBuf)) {
                    delete c;
                    whyQuit = IServerListener::InternalError;
                    break;
                }
                if (ctx->listener) {
                    ctx->listener->gotConnection((Connection *) c);
                } else {
                    delete c;
                }
                
                
                // finally, check whether we could re-create the
                // pipe
                if (ctx->pipeHand == 0) {
					// errBuf will already be populated (!)
                    whyQuit = IServerListener::InternalError;
                    break;
                }
            } else {
                // oops.  That's a real error
                errBuf = bp::error::lastErrorString(
                    "ConnectNamedPipe() failed");
                whyQuit = IServerListener::InternalError;
                break;
            }
        } else {
            errBuf = bp::error::lastErrorString(
                "unexpected return value from WaitForMultipleObjects");
            whyQuit = IServerListener::InternalError;
            break;
        }
    }

    // instruct our listener that we're going down
    const char * e = (errBuf.empty() ? NULL : errBuf.c_str());
    ctx->listener->serverEnded(whyQuit, e);

    // server is responsible for cleaning up pipeHand
    if (ctx->pipeHand != 0) {
        CloseHandle(ctx->pipeHand);
        ctx->pipeHand = 0;
    }

    delete ctx;
    return NULL;
}

Server::Server()
    : m_listener(NULL), m_running(false)
{
    memset((void *) &m_overlapped, 0, sizeof(m_overlapped));
    m_overlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    m_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}
     
Server::~Server()
{
    stop();
    CloseHandle(m_stopEvent);
    CloseHandle(m_overlapped.hEvent);    
}

bool
Server::setListener(IServerListener * listener)
{
    m_listener = listener;
    return true;
}

// named pipe must be set to low integrity so that vista plugins
// can talk to the daemon.  this will fail on winxp, and that's fine
static bool
SetObjectToLowIntegrity(HANDLE hObject,
                        SE_OBJECT_TYPE type = SE_KERNEL_OBJECT)
{
  // The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity
  static LPCWSTR LOW_INTEGRITY_SDDL_SACL = L"S:(ML;;NW;;;LW)";

  bool bRet = false;
  DWORD dwErr = ERROR_SUCCESS;
  PSECURITY_DESCRIPTOR pSD = NULL;
  PACL pSacl = NULL;
  BOOL fSaclPresent = FALSE;
  BOOL fSaclDefaulted = FALSE;
      
  if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
	    LOW_INTEGRITY_SDDL_SACL, SDDL_REVISION_1, &pSD, NULL )) {
    if (GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl,
				  &fSaclDefaulted)) {
      dwErr = SetSecurityInfo( hObject, type, LABEL_SECURITY_INFORMATION,
			       NULL, NULL, NULL, pSacl );
      bRet = (ERROR_SUCCESS == dwErr);
    }
    LocalFree ( pSD );
  }                     

  return bRet;
}


bool
Server::start(const std::string & location,
              std::string * error)
{
    HANDLE pipeHand = 0;

    if (m_running) return false;
    if (location.empty()) return false;

    m_pipeName.clear();
    m_pipeName.append("\\\\.\\pipe\\");
    m_pipeName.append(location);

    // try first with the first flag set
    pipeHand = createPipe(m_pipeName, &m_overlapped, true, error);

    if (0 == pipeHand) {
        // try again without the first flag set 
        pipeHand = createPipe(m_pipeName, &m_overlapped, false, error);

        if (0 == pipeHand) {        
            if (error != NULL) {
                *error = bp::error::lastErrorString("couldn't create pipe");
            }
            return false;
        }
    }
    
    // This call will fail on XP, which is just fine (the os privilege mechanism
    // it is required by only exists on vista and above in IE).
    (void) SetObjectToLowIntegrity(pipeHand);

    // all ready to go. now let's set up thread context
    ipcServerThreadContext * ctx = new ipcServerThreadContext;
    ctx->pipeHand = pipeHand;
    ctx->pipeOverlap = &m_overlapped;
    ctx->stopEventHand = m_stopEvent;
    ctx->listener = m_listener;
    ctx->pipeName = m_pipeName;

    // and spawn!   
    if (!m_thread.run(ipcServerThreadFunc, (void *) ctx)) {
        delete ctx;
        CloseHandle((HANDLE) pipeHand);
        if (error) *error = "Couldn't spawn thread";
        return false;
    }

    // running indicates that things have been successfully started,
    // and need to be cleaned up
    m_running = true;

    return true;
}

void
Server::stop()
{
    if (m_running) {
        BOOL x = SetEvent(m_stopEvent);
        assert(x);
        x = false;
        m_thread.join();
        m_running = false;
    }
}
