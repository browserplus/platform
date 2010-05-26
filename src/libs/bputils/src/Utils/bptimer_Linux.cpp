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
#include "api/bpthreadhopper.h"
#include "api/bpsync.h"
#include "api/bpthread.h"
#include <stdlib.h>

using namespace bp::time;


class LinuxTimer : public bp::thread::HoppingClass
{
public:
    LinuxTimer(Timer * timerPtr) : m_timerPtr(timerPtr)
    {
    }
        
    ~LinuxTimer()
    {
        cancel();
    }
    
    void setListener(ITimerListener * listener)
    {
        m_listener = listener;
    }

private:
    static void * threadfunc(void * context) 
    {
        class LinuxTimer * self = (class LinuxTimer *) context;
        self->m_mutex.lock();
        if (self->m_time) {
            self->m_cond.timeWait(&(self->m_mutex), self->m_time);
            if (self->m_time) self->hop(NULL);
        }
        self->m_mutex.unlock();
        return NULL;
    }

public:    
    
    void setMsec(unsigned int timeInMilliseconds)
    {
        cancel();
        m_mutex.lock();
        m_time = timeInMilliseconds;
        m_thread = new bp::thread::Thread;
        m_thread->run(threadfunc, (void *) this);
        m_mutex.unlock();
    }
    
    void cancel()
    {
        if (m_thread) {
            m_mutex.lock();            
            m_time = 0;
            m_cond.broadcast();
            m_mutex.unlock();
            m_thread->join();
            delete m_thread;
            m_thread = NULL;
        }
    }

  private:
    void onHop(void * context) 
    {
        if (m_time) {
            cancel();
            m_listener->timesUp(m_timerPtr);
        }
    }
    
    ITimerListener * m_listener;
    bp::thread::Thread * m_thread;
    bp::sync::Mutex m_mutex;
    bp::sync::Condition m_cond;
    unsigned int m_time;
    Timer * m_timerPtr;
};


Timer::Timer() 
{
    m_osSpecific = new LinuxTimer(this);
}

Timer::~Timer()
{
    delete ((LinuxTimer *) m_osSpecific);
}

void
Timer::setListener(ITimerListener * listener)
{
    ((LinuxTimer *) m_osSpecific)->setListener(listener);
}

void
Timer::setMsec(unsigned int timeInMilliseconds)
{
    ((LinuxTimer *) m_osSpecific)->setMsec(timeInMilliseconds);
}

void
Timer::cancel()
{
    ((LinuxTimer *) m_osSpecific)->cancel();
}
