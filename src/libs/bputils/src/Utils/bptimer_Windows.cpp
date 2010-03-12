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
 *  bptimer.h
 *
 *  an abstraction which can asynchronously call back into client code
 *  on the same thread as the timer was set on.
 *
 *  Created by Lloyd Hilaiel on 8/21/08.
 */

#include "api/bptimer.h"
#include "api/bperrorutil.h"
#include "api/bpsync.h"

#include <windows.h>
#include <iostream>
#include <map>

using namespace bp::time;

// static data structures cause windows doesn't give us a way to pass
// a pointer to timerproc
static bp::sync::Mutex s_lock;
static std::map<UINT, std::pair<Timer *, ITimerListener *>> s_timerLookup;

static void
WindowsTimerCallback(HWND, UINT, UINT id, DWORD)
{
    Timer * t = NULL;
    ITimerListener * l = NULL;

    s_lock.lock();
    std::map<UINT, std::pair<Timer *, ITimerListener *>>::iterator it;
    it = s_timerLookup.find(id);
    if (it != s_timerLookup.end()) {
        t = it->second.first;
        l = it->second.second;
        s_timerLookup.erase(it);
    }
    s_lock.unlock();

    if (t) t->cancel();
    if (l) l->timesUp(t);
}

Timer::Timer()
    : m_osSpecific(NULL), m_listener(NULL)
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
    cancel();
    UINT_PTR p = SetTimer(NULL, 0, timeInMilliseconds,
                          (TIMERPROC) WindowsTimerCallback);
    if (!p) BP_THROW_FATAL("couldn't set timer!");
    m_osSpecific = (void *) p;
    // insert into lookup table
    s_lock.lock();
    s_timerLookup[p] = std::pair<Timer *, ITimerListener *>(this, m_listener);
    s_lock.unlock();
}

void
Timer::cancel()
{
    UINT timerId = (UINT) m_osSpecific;
    if (timerId) {
        (void) KillTimer(NULL, timerId);
        m_osSpecific = NULL;
        // remove from lookup table
        s_lock.lock();
        std::map<UINT, std::pair<Timer *, ITimerListener *>>::iterator it;
        it = s_timerLookup.find(timerId);
        if (it != s_timerLookup.end()) s_timerLookup.erase(it);
        s_lock.unlock();


    }
}
