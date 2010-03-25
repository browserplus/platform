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
 * On runloops and threadhopping and async event driven systems on linux:
 *
 * In BrowserPlus there are a couple key abstractions upon which the whole
 * darn system is built.  Those are:
 * 1. runloops - an abstraction around a thread of execution that encapsulates
 *               the event delivery.
 * 2. threadhopper - A tool used that allows you to get from any give thread
 *               back to a runloop thread. 
 *
 * #2 is key, as the lowest levels of browserplus will be implemented in a
 * multithreaded fashion for performance reasons (to use multiple cores or
 * to prevent synchronous activies from blocking up other compute tasks.)
 * to make this work, we have a generalized notion of a "threadhopper" which
 * allows the specific threaded subsystem to hop onto the runloop/main thread
 * before calling back into client code.  The fallout of this design is that
 * the large majority of the system is non-blocking event driven async code,
 * and locking is never required.  Some low level modules, conversely, bear
 * all the burden of the complexity of threading to provide great performance.
 * 
 * Implementation of these two abstractions on windows and osx is pretty natural,
 * as there are already constructs available that fit,  window message pumps
 * or "RunLoops"..  That is on both platforms it's possible from a thread to
 * use vendor provided framework calls to post a message cross thread.
 *
 * On linux, no such mechanism exists.  This is why the runloop must have a
 * special relationship with threadhopper.  That special linux specific
 * interface is exposed in bprunloop_Linux.h and used by threadhopper.
 * The basic functionality exposed includes:
 *
 * 1. give me a token which represents the current thread
 * 2. call this function with this argument on the thread represented by this token
 */

#include "api/bprunloop.h"

#include <stdlib.h>
#include <assert.h>

struct BasicRunLoopData 
{
    // queue of events
    std::vector<bp::runloop::Event> m_workQueue;
    bool m_stopped;
    bool m_running;
    bp::sync::Condition m_cond;
};

void
bp::runloop::RunLoop::init()
{
    BasicRunLoopData * rld = new BasicRunLoopData;
    m_osSpecific =  (void *) rld;
    rld->m_stopped = false;
    rld->m_running = false;
}

void
bp::runloop::RunLoop::shutdown()
{
    assert(m_osSpecific != NULL);
    delete ((BasicRunLoopData *) m_osSpecific);
    m_osSpecific = NULL;
}

void
bp::runloop::RunLoop::run()
{
    BasicRunLoopData * rld = (BasicRunLoopData *) m_osSpecific;

    m_lock.lock();
    assert(!rld->m_running);
    rld->m_running = true;
    do {
        // process events
        while (rld->m_workQueue.size() > 0) {
            Event work(*(rld->m_workQueue.begin()));
            rld->m_workQueue.erase(rld->m_workQueue.begin());
            if (m_onEvent) {
                // run work outside of lock
                m_lock.unlock();
                m_onEvent(m_onEventCookie, work);
                m_lock.lock();
            }
        }

        // wait for something to happen
        if (!(rld->m_stopped)) rld->m_cond.wait(&(m_lock));            
    } while (!(rld->m_stopped));
    rld->m_running = false;
    rld->m_stopped = false;
    m_lock.unlock();
}

bool
bp::runloop::RunLoop::sendEvent(Event e)
{
    BasicRunLoopData * rld = (BasicRunLoopData *) m_osSpecific;

    m_lock.lock();
    rld->m_workQueue.push_back(e);
    rld->m_cond.broadcast();
    m_lock.unlock();

    return true;
}

void
bp::runloop::RunLoop::stop()
{
    BasicRunLoopData * rld = (BasicRunLoopData *) m_osSpecific;

    m_lock.lock();
    rld->m_stopped = true;
    rld->m_cond.broadcast();    
    m_lock.unlock();

   // noop
}

void
bp::runloop::RunLoop::setCallBacks(eventCallBack onEvent, void * onEventCookie)
{
    m_onEvent = onEvent;
    m_onEventCookie = onEventCookie;
}
