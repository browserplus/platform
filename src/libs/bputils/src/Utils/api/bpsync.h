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
 *  bpsync.h
 *
 *  Lightweight cross-platform abstractions of synchronization primitives,
 *  in the POSIX style.
 *
 *  Created by Lloyd Hilaiel on 8/1/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPSYNC_H__
#define __BPSYNC_H__

namespace bp { 
namespace sync {
    /** a posix style non-recursive mutual exclusion device */
    class Mutex {
      public:
        Mutex();
        ~Mutex();
        void lock();
        void unlock();
      private:
        void * m_osDep;
        friend class Condition;
    };

    /** standard lock for exception-safe Mutex usage */
    class Lock {
      public:
        Lock(Mutex& mutex) : m_mutex( mutex ) { m_mutex.lock(); }
        ~Lock() { m_mutex.unlock(); }
      private:
        Mutex & m_mutex;
        Lock(const Lock &);             // prevent copy construct
        Lock& operator=(const Lock &);  // prevent copy assign
    };
    
    /** a posix style condition */
    class Condition {
      public:
        Condition();
        ~Condition();
        /** awake all threads waiting on the condition */
        void broadcast();
        /** awake a single thread waiting on the condition */
        void signal();
        /** atomically release a mutex and sleep on a condition.
         *  Warning: under certain conditions a thread may be awoken
         *           spuriously.  Just as in POSIX.  The client must
         *           check state upon waking up to catch this case.
         */
        void wait(Mutex * m);
        /** wait until a signal is recieved or a timeout is hit.
         *  Warning: under certain conditions a thread may be awoken
         *           spuriously.  Just as in POSIX.  The client must
         *           check state upon waking up to catch this case.
         *  \returns true if a signal was recieved, false otherwise */
        bool timeWait(Mutex * m, unsigned int msec);
      private:
        void * m_osDep;
    };
}}

#endif
