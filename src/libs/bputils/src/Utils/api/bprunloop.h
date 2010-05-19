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
 *  bprunloop.h
 *
 *  an abstraction around running and stopping a native operating system
 *  runloop.
 *
 *  CAVEATS & TODOS:
 *   + required function call ordering is annoying (init, run, shutdown),
 *     and results from the need to allocate runloops on one thread and
 *     run them on another.  this annoyance should be mitigated. 
 *   + needs more runtime checking of correct api usage.
 * 
 *  Created by Lloyd Hilaiel on 6/20/08.
 */

#ifndef __BPRUNLOOP_H__
#define __BPRUNLOOP_H__

#include "bpthread.h"
#include "bpsync.h"

#include <vector>

namespace bp { namespace runloop {

  class Event {
    public:
      Event(void * payload);
      Event(const Event & e);
      virtual ~Event();
      void * payload();
    protected:
      void * m_payload;
  };

  typedef void (*eventCallBack)(void *, Event);

  class RunLoop
  {
    public:
      RunLoop();
      ~RunLoop();

      // set the callback that should be invoked when events are
      // received
      void setCallBacks(eventCallBack onEvent, void * onEventCookie);

      // Initialize the runloop. This must be called before run on the
      // thread that will run the runloop.  
      void init();
      // run the runloop
      void run();
      // stop the runloop.  This may be called from any thread and will
      // cause run to return after all pending events are processed.
      void stop();

      // send an event to the runloop  This may be called from any thread.
      bool sendEvent(Event e);

      // Shutdown the runloop.  This should be called after run returns
      // on the same thread.  Failure to call shutdown will result in
      // resource leakage.  Calling shutdown while the runloop is running
      // results in undefined behavior (i.e. crash, divorce, etc)
      void shutdown();

    private:
      void * m_osSpecific;
      eventCallBack m_onEvent;
      void * m_onEventCookie;

      std::vector<bp::runloop::Event> m_workQueue;
      bp::sync::Mutex m_lock;

      // linux has a global runloop collection with deep access to runloops
      // to allow us to implement "thread hopping" at the application level,
      // as os frameworks provide no such notion.
      friend class GlobalRunLoopCollection;
  };
}}

#endif
