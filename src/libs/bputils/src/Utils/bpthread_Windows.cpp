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
 *  bpthread.h -- POSIX like abstractions around OS threads.
 *
 *  Adapted from WUF Tehcnologies threading abstraction.
 *
 *  Written by Lloyd Hilaiel, 2005 & 2007
 *  Copywrite Yahoo! Inc. 2007
 */

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "bpthread.h"

using namespace bp::thread;

struct WinThreadData {
  HANDLE thrHndl;
  DWORD id;
};


Thread::Thread()
{
    m_osSpecific = (void *) calloc(1, sizeof(WinThreadData));
}

Thread::~Thread()
{
    free(m_osSpecific);
    m_osSpecific = NULL;
}

bool
Thread::run(StartRoutine startFunc, void * cookie)
{
    WinThreadData * td = (WinThreadData *) m_osSpecific;

    td->thrHndl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) startFunc, cookie, 0,
                               &(td->id));

    if (!td->thrHndl) return false;
    
    return true;
}

void
Thread::detach()
{
    WinThreadData * td = (WinThreadData *) m_osSpecific;
    CloseHandle(td->thrHndl);
}

void
Thread::join()
{
    WinThreadData * td = (WinThreadData *) m_osSpecific;
    WaitForSingleObject(td->thrHndl, INFINITE);
    CloseHandle(td->thrHndl);
}

unsigned int
Thread::ID()
{
    WinThreadData * td = (WinThreadData *) m_osSpecific;
    return td->id;
}

unsigned int
Thread::currentThreadID()
{
    return (unsigned int) GetCurrentThreadId();
}
