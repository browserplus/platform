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

#include "bpthreadhopper.h"
#include "bpsync.h"
#include "api/bpuuid.h"
#include "api/bpstrutil.h"
#include "api/bpthread.h"
#include "bprunloop_Linux.h"

#include <stdlib.h>

bool
bp::thread::Hopper::initializeOnCurrentThread()
{
    m_osSpecific = (void *) bp::thread::Thread::currentThreadID();
    return true;
}

bool
bp::thread::Hopper::invokeOnThread(InvokeFuncPtr invokeFunc, void * context)
{
    unsigned int tid = (unsigned int) ((size_t) m_osSpecific);
    bprll_invokeOnThread(tid, invokeFunc, context);
    return true;
}

bp::thread::Hopper::~Hopper()
{
}

void
bp::thread::Hopper::processOutstandingRequests()
{
    // XXX
}
