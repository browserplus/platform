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

/*
 * An abstraction around the capturing standard error and standard output
 * and redirecting them into the BrowserPlus logging system.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/20
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "OutputRedirector.h"
#include <stddef.h>
#include <string>
#include "BPUtils/BPLog.h"
#include "BPUtils/bpthread.h"

static int outPipe[2];
static int errPipe[2];
static int endPipe[2];
static bool running = false;
static bp::thread::Thread redirector;

#ifndef WIN32
static void * redirectStdHandles(void *) 
{
    int maxfd = outPipe[0];
    if (errPipe[0] > maxfd) maxfd = errPipe[0];
    if (endPipe[0] > maxfd) maxfd = endPipe[0];    
    maxfd += 1;
    
    for (;;) 
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(errPipe[0], &readfds);
        FD_SET(outPipe[0], &readfds);
        FD_SET(endPipe[0], &readfds);

        int rv = ::select(maxfd, &readfds, NULL, NULL, NULL);

        if (rv < 0) {
            std::string error = bp::error::lastErrorString("select failed");
            BPLOG_ERROR_STRM("standard handle redirect fails: "
                               << error);
            break;
        }

        // first we'll see if the end channel is hot
        if (FD_ISSET(endPipe[0], &readfds)) {
            break;
        }

        if (FD_ISSET(errPipe[0], &readfds)) {
            char buf[1024];
            int x = read(errPipe[0], (void *) buf, sizeof(buf) - 1);
            if (x > 0) {
                buf[x] = 0;
                BPLOG_ERROR(buf);
            }
        }

        if (FD_ISSET(outPipe[0], &readfds)) {
            char buf[1024];
            int x = read(outPipe[0], (void *) buf, sizeof(buf) - 1);
            if (x > 0) {
                buf[x] = 0;
                BPLOG_INFO(buf);
            }
        }
    }

    BPLOG_DEBUG("standard handle redirector exiting");

    close(outPipe[0]);
    close(outPipe[1]);    
    close(errPipe[0]);
    close(errPipe[1]);    
    close(endPipe[0]);
    close(endPipe[1]);    
    return NULL;
}
#endif

OutputRedirector::OutputRedirector()
{
}

void
OutputRedirector::redirect()
{
#ifndef WIN32
    pipe(outPipe);
    pipe(errPipe);    
    pipe(endPipe);    

    // now let's redirect stdout and stderr
    dup2(outPipe[1], 1);
    dup2(errPipe[1], 2);    

    // and we'll spawn our poller thread
    redirector.run(redirectStdHandles, NULL);
    running = true;
#endif
}

OutputRedirector::~OutputRedirector()
{
#ifndef WIN32
    if (running) {
        BPLOG_DEBUG("stopping standard handle redirector...");
        char byte = 'l';
        write(endPipe[1], (void *) &byte, sizeof(byte));
        redirector.join();
    }
#endif
}
