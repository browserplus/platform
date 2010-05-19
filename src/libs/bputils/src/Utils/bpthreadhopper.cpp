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

/************************************************************************
 * ThreadHopper - A utility to grab the attention of the thread that
 *        it's instantiated on.  In order to function the thread must
 *        be using operating system provided runloops/event pumps.
 *
 * Written by Lloyd Hilaiel, Fri Jul  6 13:58:30 MDT 2007
 * (c) 2007, Yahoo! Inc, all rights reserved.
 */

#include "bpthreadhopper.h"

#include <iostream>
#include <stdlib.h>

bp::thread::Hopper::Hopper()
    : m_osSpecific(NULL)
{
}

bp::thread::HoppingClass::HoppingClass() : m_hopper()
{
    m_hopper.initializeOnCurrentThread();
}

bp::thread::HoppingClass::~HoppingClass()
{
}

void
bp::thread::HoppingClass::hoppingClassRelayFunc(void * ctx)
{
    HoppingClass * t = (HoppingClass *) (((void **)ctx)[0]);
    void * arg = ((void **)ctx)[1];
    t->onHop(arg);
    free(ctx);
}

void
bp::thread::HoppingClass::hop(void * context)
{
    void ** args = (void **) calloc(2, sizeof(void *));
    args[0] = this;
    args[1] = context;
    m_hopper.invokeOnThread(hoppingClassRelayFunc, (void *) args);
}
