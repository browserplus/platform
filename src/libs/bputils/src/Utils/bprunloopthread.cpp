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
 *  bprunloopthread.h
 *
 *  Created by Lloyd Hilaiel on 6/20/08.
 */

#include "api/bprunloopthread.h"
#include "api/bpthread.h"
#include "api/bperrorutil.h"

#include <stdlib.h>

void *
bp::runloop::RunLoopThread::threadFunction(void * c)
{
    RunLoopThread * rl = (RunLoopThread *) c;

    BPASSERT(!rl->m_running);

    rl->m_rlThreadID = bp::thread::Thread::currentThreadID();

    rl->m_rl.init();

    // call atstart callback
    if (rl->m_atStart) rl->m_atStart(rl->m_atStartCookie);
    rl->m_lock.lock();
    rl->m_running = true;
    rl->m_cond.broadcast();    

    // run the runloop
    rl->m_lock.unlock();
    rl->m_rl.run();
    rl->m_lock.lock();
    // call atend callback
    if (rl->m_atEnd) rl->m_atEnd(rl->m_atEndCookie);
    rl->m_running = false;    

    rl->m_rl.shutdown();

    rl->m_cond.broadcast();    
    rl->m_lock.unlock();

    return NULL;
}

bp::runloop::RunLoopThread::RunLoopThread()
    : m_atStart(NULL), m_atStartCookie(NULL), m_atEnd(NULL),
      m_atEndCookie(NULL), m_running(false), m_stopped(false),
      m_state(S_Allocated), m_rlThreadID(0)
{
}

bp::runloop::RunLoopThread::~RunLoopThread()
{
}

bool
bp::runloop::RunLoopThread::running()
{
    bool running;
    m_lock.lock();
    running = m_running && !m_stopped;
    m_lock.unlock();    
    return running;
}

void
bp::runloop::RunLoopThread::setCallBacks(
    runLoopCallBack atStart, void * atStartCookie,
    runLoopCallBack atEnd, void * atEndCookie,
    eventCallBack onEvent, void * onEventCookie)
{
    m_atStart = atStart;
    m_atStartCookie = atStartCookie;
    m_atEnd = atEnd;
    m_atEndCookie = atEndCookie;
    m_rl.setCallBacks(onEvent, onEventCookie);
}

bool
bp::runloop::RunLoopThread::run()
{
    validateStateChange(S_Running);

    bool ran = false;

    BPASSERT(!m_running);
    m_lock.lock();
    ran = m_thr.run(threadFunction, (void *) this);
    while (ran && !m_running) {
        m_cond.wait(&m_lock);
    }
    m_lock.unlock();

    return ran;
}

bool
bp::runloop::RunLoopThread::stop()
{
    if (m_rlThreadID == bp::thread::Thread::currentThreadID()) {
        m_rl.stop();
    } else {
        if (m_state > S_Running) return true;

        validateStateChange(S_Stopping);

        m_lock.lock();
        m_stopped = true;
        m_rl.stop();
        m_cond.broadcast();
        while (m_running) m_cond.wait(&m_lock);
        m_lock.unlock();
        
        validateStateChange(S_Stopped);
    }
    return true;
}

bool
bp::runloop::RunLoopThread::join()
{
    BPASSERT(m_rlThreadID != bp::thread::Thread::currentThreadID());
    if (m_rlThreadID == bp::thread::Thread::currentThreadID()) {
        return false;
    }
    // thread has already ended
    if (m_state == S_Allocated) return false;
    if (m_state > S_Running) return true;
    m_thr.join();
    validateStateChange(S_Stopped);
    return true;
}

bool
bp::runloop::RunLoopThread::sendEvent(Event e)
{
    BPASSERT(m_running);
    return m_rl.sendEvent(e);
}

void
bp::runloop::RunLoopThread::validateStateChange(State newState)
{
    switch (m_state) {
        case S_Allocated:
            if (newState == S_Stopped || newState == S_Stopping) {
                BP_THROW_FATAL(
                    "stopping runloopthread that hasn't been started");
            }
            if (newState != S_Running) {
                BP_THROW_FATAL(
                    "internal error: runloopthread expecting run() call");
            }
            break;
        case S_Running:
            if (newState == S_Running) {
                BP_THROW_FATAL("attempted double run of runloopthread");
            }
            if (newState != S_Stopped && newState != S_Stopping) {
                BP_THROW_FATAL(
                    "internal error: running runloopthread expected stop()");
            }
            break;
        case S_Stopping:
            if (newState == S_Running) {
                BP_THROW_FATAL("attempted re-run of runloopthread");
            }
            if (newState == S_Stopping) {
                BP_THROW_FATAL("double stop() of runloopthread");
            }
            break;
        case S_Stopped:
            if (newState == S_Running) {
                BP_THROW_FATAL("attempted re-run of runloopthread");
            }
            BP_THROW_FATAL("unexecpected use of stopped runloopthread");
            break;
    }
    m_state = newState;
}
