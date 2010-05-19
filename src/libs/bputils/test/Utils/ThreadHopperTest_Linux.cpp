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

#include "ThreadHopperTest.h"
#include "BPUtils/bpthread.h"
#include "BPUtils/bpthreadhopper.h"
#include "BPUtils/bprunloop.h"

// struct to get data through void *
struct HopperAndTid {
    bp::thread::Hopper * hopper;
    unsigned int * tid;
    bp::runloop::RunLoop * rl;
};
    
static void
mycallback(void * tid)
{
    HopperAndTid * hat = (HopperAndTid *) tid;
    *(hat->tid) = bp::thread::Thread::currentThreadID();
    hat->rl->stop();
}

// a thread who's only purpose in life is to try to kick back over to
// the runloop thread using Thread Hopper
void * threadFunc(void * tid)
{
    HopperAndTid * hat = (HopperAndTid *) tid;
    hat->hopper->invokeOnThread(mycallback, hat);
    return NULL;
}

void
ThreadHopperTest::tryHop()
{
    bp::runloop::RunLoop rl;
    bp::thread::Hopper myhopper;

    rl.init();
    
    bool initd = myhopper.initializeOnCurrentThread();
    CPPUNIT_ASSERT(initd == true);

    // now scedule some work to be done on the main thread.
    // that work is to check the thread id of the current thread
    // matches that upon which the callback is invoked.
    unsigned int originalID = 0;
    unsigned int calledID = 0;    
    originalID = bp::thread::Thread::currentThreadID();

    // spin a thread to call invokeOnThread()
    HopperAndTid hat = { &myhopper, &calledID, &rl };

    bp::thread::Thread thred;
    bool r = thred.run(threadFunc, (void *) &hat);
    CPPUNIT_ASSERT( r );

    // run the runloop.  it should not run for the full duration
    rl.run();

    // verify that the main thread id and the thread id upon which our
    // callback was executed match
    CPPUNIT_ASSERT(originalID == calledID);

    thred.join();

    rl.shutdown();

    return;
}
