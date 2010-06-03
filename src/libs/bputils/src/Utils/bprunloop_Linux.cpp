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
 * The basic functionality required is a function which allows threadhopper
 * to call cause a function to be invoked with an argument on a given thread.
 */

#include "api/bprunloop.h"
#include "api/bpsync.h"
#include "api/bperrorutil.h"
#include "bprunloop_Linux.h"

#include <stdlib.h>
#include <stdio.h>

#include <map>

/* linux events are one of two types, either application events,
 * which come into existence via the RunLoop::sendEvent call,
 * or Hop events, which come in via the bprll_invokeOnThread hook
 * which exists exclusively for threadhopper */
struct LinuxEvent 
{
    LinuxEvent() : e(NULL) { }
            
         
    enum { T_Hop, T_App } type;
    // in the case of T_App
    bp::runloop::Event e;
    // in the case of T_Hop
    bprll_invokableFunc func;
    void * argument;
};

struct BasicRunLoopData 
{
    // queue of events
    std::vector<LinuxEvent> m_workQueue;

    bool m_stopped;
    bool m_running;
    bp::sync::Condition m_cond;
};


namespace bp { namespace runloop {
     
class GlobalRunLoopCollection
{
public:
    GlobalRunLoopCollection() 
    {
    }

    ~GlobalRunLoopCollection() 
    {
    }
    
    void addRunLoop(bp::runloop::RunLoop * rl) 
    {
        bp::sync::Lock l(m_lock);
        unsigned int tid = bp::thread::Thread::currentThreadID();
        if (m_runloops.find(tid) != m_runloops.end()) {
            BP_THROW_FATAL("multiple runloops allocated on the same thread");
        }
        m_runloops[tid] = rl;
    }

    void removeRunLoop() 
    {
        bp::sync::Lock l(m_lock);
        unsigned int tid = bp::thread::Thread::currentThreadID();
        std::map<unsigned int, bp::runloop::RunLoop *>::iterator rlit;        
        rlit = m_runloops.find(tid);

        if (rlit == m_runloops.end()) {
            BP_THROW_FATAL("no runloop to remove from thread");
        }
        m_runloops.erase(rlit);
    }

    void invoke(unsigned int threadID,
                bprll_invokableFunc func,
                void * argument)   
    {
        // here, we must get a LinuxEvent of type hop onto
        // the runloop's queue
        bp::sync::Lock l(m_lock);
        std::map<unsigned int, bp::runloop::RunLoop *>::iterator rlit;        
        rlit = m_runloops.find(threadID);
        if (rlit == m_runloops.end()) {
            BP_THROW_FATAL("no runloop on target thread");
        }
        // an altered implementation of RunLoop::sendEvent -
        bp::runloop::RunLoop * rl = rlit->second;
        
        BasicRunLoopData * rld = (BasicRunLoopData *) (rl->m_osSpecific);

        bp::sync::Lock l2(rl->m_lock);
        LinuxEvent le;
        le.type = LinuxEvent::T_Hop;
        le.func = func;
        le.argument = argument;
        rld->m_workQueue.push_back(le);
        rld->m_cond.broadcast();
    }

private:
    std::map<unsigned int, bp::runloop::RunLoop *> m_runloops;
    bp::sync::Mutex m_lock;
};
} }

static bp::runloop::GlobalRunLoopCollection s_runloopColl;

void bprll_invokeOnThread(unsigned int threadID,
                          bprll_invokableFunc func,
                          void * argument)
{
    s_runloopColl.invoke(threadID, func, argument);
}

void
bp::runloop::RunLoop::init()
{
    BasicRunLoopData * rld = new BasicRunLoopData;
    m_osSpecific =  (void *) rld;
    rld->m_stopped = false;
    rld->m_running = false;
    s_runloopColl.addRunLoop(this);
}

void
bp::runloop::RunLoop::shutdown()
{
    s_runloopColl.removeRunLoop();
    BPASSERT(m_osSpecific != NULL);
    delete ((BasicRunLoopData *) m_osSpecific);
    m_osSpecific = NULL;
}

void
bp::runloop::RunLoop::run()
{
    BasicRunLoopData * rld = (BasicRunLoopData *) m_osSpecific;
    // a boolean to cause us to run through the queue once after we're
    // stopped to process outstanding events, allowing events to 
    // be handled properly when client calls sendEvent() stop() in rapid
    // succession.
    bool finalProcess = true;

    m_lock.lock();
    BPASSERT(!rld->m_running);
    rld->m_running = true;
    do {
        if (rld->m_stopped) finalProcess = false;
        // process events
        while (rld->m_workQueue.size() > 0) {
            LinuxEvent le = *(rld->m_workQueue.begin());
            rld->m_workQueue.erase(rld->m_workQueue.begin());

            if (le.type == LinuxEvent::T_App) {
                if (m_onEvent) {
                    // run work outside of lock
                    m_lock.unlock();
                    m_onEvent(m_onEventCookie, le.e);
                    m_lock.lock();
                }
            } else {
                BPASSERT(le.type == LinuxEvent::T_Hop);
                // run hop outside of lock
                m_lock.unlock();
                le.func(le.argument);
                m_lock.lock();
            }
        }
        
        // wait for something to happen
        if (!(rld->m_stopped)) rld->m_cond.wait(&(m_lock));            
    } while (!(rld->m_stopped) || finalProcess);
    rld->m_running = false;
    rld->m_stopped = false;
    m_lock.unlock();
}

bool
bp::runloop::RunLoop::sendEvent(Event e)
{
    BasicRunLoopData * rld = (BasicRunLoopData *) m_osSpecific;

    bp::sync::Lock l(m_lock);
    LinuxEvent le;
    le.type = LinuxEvent::T_App;
    le.e = e;
    rld->m_workQueue.push_back(le);
    rld->m_cond.broadcast();

    return true;
}

void
bp::runloop::RunLoop::stop()
{
    BasicRunLoopData * rld = (BasicRunLoopData *) m_osSpecific;

    bp::sync::Lock l(m_lock);
    rld->m_stopped = true;
    rld->m_cond.broadcast();    

    // noop
}

void
bp::runloop::RunLoop::setCallBacks(eventCallBack onEvent, void * onEventCookie)
{
    m_onEvent = onEvent;
    m_onEventCookie = onEventCookie;
}
