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
#include <windows.h>
#include "BPUtils/bpthread.h"
#include "BPUtils/bpthreadhopper.h"



static void
mycallback(void * tid)
{
    *((long long *) tid) = ::GetCurrentThreadId();
    // stop the runloop
    PostQuitMessage(0);
}

// struct to get data through void *
struct HopperAndTid {
    bp::thread::Hopper * hopper;
    long long * tid;
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
    long long originalID = ::GetCurrentThreadId();
    long long calledID = 0;    

    // spin a thread to call invokeOnThread()
    HopperAndTid hat = { &myhopper, &calledID };

    bp::thread::Thread thred;
    bool r = thred.run(threadFunc, (void *) &hat);
    CPPUNIT_ASSERT( r );

    // run the windows message pump!
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);        
    }
    
    // verify that the main thread id and the thread id upon which our
    // callback was executed match
    CPPUNIT_ASSERT(originalID == calledID);

    thred.join();

    return;
}
