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

/*
 *  bprunloop.h
 *
 *  Created by Lloyd Hilaiel on 6/20/08.
 */

#include "api/bprunloop.h"

#include <stdlib.h>

bp::runloop::Event::Event(void * payload)
    : m_payload(payload)
{
}

bp::runloop::Event::Event(const Event & e)
    : m_payload(e.m_payload)
{
}

void *
bp::runloop::Event::payload()
{
    return m_payload;
}

bp::runloop::Event::~Event()
{
}

bp::runloop::RunLoop::RunLoop()
    : m_osSpecific(NULL),
      m_onEvent(NULL),
      m_onEventCookie(NULL)
{
}

bp::runloop::RunLoop::~RunLoop()
{
}

