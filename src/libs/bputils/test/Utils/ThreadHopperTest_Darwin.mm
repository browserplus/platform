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
#include "Cocoa/Cocoa.h"
#include "BPUtils/bpthread.h"
#include "BPUtils/bpthreadhopper.h"

static void
mycallback(void * tid)
{
    GetCurrentThread((ThreadID *) tid);
    CFRunLoopStop(CFRunLoopGetCurrent());
}

// struct to get data through void *
struct HopperAndTid {
    bp::thread::Hopper * hopper;
    ThreadID * tid;
};
    
// a thread who's only purpose in life is to try to kick back over to
// the runloop thread using Thread Hopper
void * threadFunc(void * tid)
{
    HopperAndTid * hat = (HopperAndTid *) tid;
    hat->hopper->invokeOnThread(mycallback, hat->tid);
    return NULL;
}

void
ThreadHopperTest::tryHop()
{
    bp::thread::Hopper myhopper;
    bool initd = myhopper.initializeOnCurrentThread();
    CPPUNIT_ASSERT(initd == true);

    // now scedule some work to be done on the main thread.
    // that work is to check the thread id of the current thread
    // matches that upon which the callback is invoked.
    ThreadID originalID = 0;
    ThreadID calledID = 0;    
    GetCurrentThread(&originalID);

    // spin a thread to call invokeOnThread()
    HopperAndTid hat = { &myhopper, &calledID };

    bp::thread::Thread thred;
    bool r = thred.run(threadFunc, (void *) &hat);
    CPPUNIT_ASSERT( r );

    // run the runloop.  it should not run for the full duration
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 10, false);

    // verify that the main thread id and the thread id upon which our
    // callback was executed match
    CPPUNIT_ASSERT(originalID == calledID);

    thred.join();

    return;
}
