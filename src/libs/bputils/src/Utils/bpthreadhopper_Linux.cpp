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

#include "bpthreadhopper.h"
#include "bpsync.h"
#include "api/bpuuid.h"
#include "api/bpstrutil.h"

#include <stdlib.h>

#warning "threadhopper not implemented on linux, will abort at runtime!"   

bool
bp::thread::Hopper::initializeOnCurrentThread()
{
    abort();
    return false;
}

bool
bp::thread::Hopper::invokeOnThread(InvokeFuncPtr invokeFunc, void * context)
{
    abort();
    return false;
}

bp::thread::Hopper::~Hopper()
{
    abort();
}

void
bp::thread::Hopper::processOutstandingRequests()
{
    abort();
}
