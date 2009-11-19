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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

#include "bpsync.h"

#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>

bp::sync::Mutex::Mutex()
{
    pthread_mutex_t * mtx =
        (pthread_mutex_t *) calloc(sizeof(pthread_mutex_t), 1);
    pthread_mutex_init(mtx, NULL);
    m_osDep = (void *) mtx;
}

bp::sync::Mutex::~Mutex()
{
    pthread_mutex_destroy((pthread_mutex_t *) m_osDep);
    free(m_osDep); 
}

void
bp::sync::Mutex::lock()
{
    pthread_mutex_lock((pthread_mutex_t *) m_osDep);  
}

void
bp::sync::Mutex::unlock()
{
    pthread_mutex_unlock((pthread_mutex_t *) m_osDep);  
}

bp::sync::Condition::Condition()
{
    pthread_cond_t * c =
        (pthread_cond_t *) calloc(sizeof(pthread_cond_t), 1);
    pthread_cond_init(c, NULL);
    m_osDep = (void *) c;
}

bp::sync::Condition::~Condition()
{
    pthread_cond_destroy((pthread_cond_t *) m_osDep);
    free(m_osDep);
}

void
bp::sync::Condition::broadcast()
{
  pthread_cond_broadcast((pthread_cond_t *) m_osDep);
}

void
bp::sync::Condition::signal()
{
    pthread_cond_signal((pthread_cond_t *) m_osDep);
}

void
bp::sync::Condition::wait(bp::sync::Mutex * m)
{
    pthread_cond_wait((pthread_cond_t *) m_osDep,
                      (pthread_mutex_t *) m->m_osDep);
}

bool
bp::sync::Condition::timeWait(Mutex * m, unsigned int msec)
{
    int ret;

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  
      struct timespec 
      {
        __time_t tv_sec;             Seconds.  
        long int tv_nsec;            Nanoseconds.  
      };
  */ 
    {
        struct timespec ts;
        struct timeval  tv;

        gettimeofday(&tv,NULL);
        msec += (tv.tv_usec/1000);
        ts.tv_sec = tv.tv_sec + msec/1000;
        ts.tv_nsec = (msec % 1000) * 1000000;
        ret = pthread_cond_timedwait((pthread_cond_t *) m_osDep,
                                     (pthread_mutex_t *) m->m_osDep, &ts);
    }

    // return of zero means success, a signal was received, nonzero means
    // error, we assume timeout. 
    return (ret == 0);
}
