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

#include "api/bptimer.h"
#include "api/bperrorutil.h"
#include "api/bpsync.h"

#include <stdlib.h>

using namespace bp::time;

Timer::Timer() : m_osSpecific(NULL)
{
}

Timer::~Timer()
{
    cancel();
}

void
Timer::setListener(ITimerListener * listener)
{
    m_listener = listener;
}

void
Timer::setMsec(unsigned int timeInMilliseconds)
{
    // TODO: write me
#warning "implementation not complete on linux, will abort at runtime"
    abort();
}

void
Timer::cancel()
{
    // TODO: write me
#warning "implementation not complete on linux, will abort at runtime"
    abort();
}
