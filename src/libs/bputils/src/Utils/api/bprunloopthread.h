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

/*
 *  bprunloopthread.h
 *
 *  An abstraction around managing a thread that runs a native OS runloop,
 *  built on top of bp::runloop::RunLoop.
 *
 *  Created by Lloyd Hilaiel on 6/20/08.
 */

#ifndef __BPRUNLOOPTHREAD_H__
#define __BPRUNLOOPTHREAD_H__

#include "bpthread.h"
#include "bpsync.h"
#include "bprunloop.h"

#include <vector>

namespace bp { namespace runloop {

  typedef void (*runLoopCallBack)(void *);

  class RunLoopThread
  {
    public:
      // allocate a new runloop thread
      RunLoopThread();
      ~RunLoopThread();

      void setCallBacks(runLoopCallBack atStart, void * atStartCookie,
                        runLoopCallBack atEnd, void * atEndCookie,
                        eventCallBack onEvent, void * onEventCookie);

      // Run the runloop.  the atStart callback will have been invoked on
      // the spun thread by the time this function returns.
      // this call WILL NOT BLOCK.
      bool run();

      // Stop the runloop, this function may be called from any thread.  
      //
      // This function behaves differently depending on the thread
      // it's called from:
      // * When called from the spun runloop thread: the call is non-blocking
      // * When called from any other thread, atEnd callback will be have
      //   invoked by the time this function returns.
      bool stop();

      // is the runloop thread running?
      bool running();

      // join the runloop thread.
      // this function may not be called from the runloop thread itself.
      // \warning Join will not STOP the runloop, by calling this you're
      //          blocking until someone else (perhaps the thread itself)
      //          calls stop on the RunLoopThread
      bool join();
      
      // get the numeric thread ID of the thread upon which the runloop
      // is running
      unsigned int ID();

      bool sendEvent(Event e);
    private:
      bp::thread::Thread m_thr;
      bp::sync::Mutex m_lock;
      bp::sync::Condition m_cond;

      runLoopCallBack m_atStart;
      void * m_atStartCookie;
      runLoopCallBack m_atEnd;
      void * m_atEndCookie;
      
      // a flag which is only set by spun thread
      bool m_running;

      // a flag which is set when the runloop is to be stopped
      bool m_stopped;

      // a state machine to ensure correct API usage
      // state is not manipulated by the runloop thread, only
      // the api entry points, presumably invoked from the
      // controlling thread.
      enum State {
          S_Allocated,
          S_Running,
          S_Stopping,
          S_Stopped
      } m_state;
      // validate correct api usage, throwing fatal errors if
      // used incorrectly
      void validateStateChange(State newState);

      static void * threadFunction(void * c);

      // os-specific runloop abstraction
      RunLoop m_rl;
      
      unsigned int m_rlThreadID;
  };

}}

#endif
