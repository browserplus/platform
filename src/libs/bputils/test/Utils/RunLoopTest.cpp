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

/**
 * RunLoopTest.cpp
 * Unit tests for the bp::runloop component
 *
 * Created by Lloyd Hilaiel on 6/20/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "RunLoopTest.h"
#include "BPUtils/bprunloop.h"


CPPUNIT_TEST_SUITE_REGISTRATION(RunLoopTest);

struct SumAndLoop 
{
    int sum;
    bp::runloop::RunLoop * rl;
};
    
static void processEventSumming(void * ptr, bp::runloop::Event e) 
{
    long amt = (long) e.payload();
    SumAndLoop * snl = (SumAndLoop *) ptr;
    snl->sum += amt;
    if (amt == 4) snl->rl->stop();
}

void RunLoopTest::basicRunLoopTest()
{
    // run on a single thread, post some events before we run the runloop

    bp::runloop::RunLoop rl;
    SumAndLoop snl = { 0, &rl };

    rl.setCallBacks(processEventSumming, &snl);

    rl.init();
    
    rl.sendEvent(bp::runloop::Event((void *) 1));
    rl.sendEvent(bp::runloop::Event((void *) 2));
    rl.sendEvent(bp::runloop::Event((void *) 3));
    rl.sendEvent(bp::runloop::Event((void *) 4));
    
    rl.run();

    CPPUNIT_ASSERT( snl.sum == 10 );    

    rl.shutdown();
}


static void mtSummingCallback(void * ptr, bp::runloop::Event e) 
{
    *((long long *) ptr) += (long) e.payload();
}

static void * mtSummingThreadFunc(void * ptr) 
{
    static int s_numFinished = 0;
    static bp::sync::Mutex s_m;

    bp::runloop::RunLoop * rl = (bp::runloop::RunLoop *) ptr;
    unsigned int i = 0;
    for (i = 0; i <= 999; i++) {
        rl->sendEvent(bp::runloop::Event((void *) i));
    }

    s_m.lock();
    if (++s_numFinished == 3) rl->stop();
    s_m.unlock();

    return NULL;
}

void RunLoopTest::mtSafetyTest()
{
    // run runloop, spawn three threads to pound it with events 
    bp::runloop::RunLoop rl;
    long long int sum = 0;
    bp::thread::Thread t1, t2, t3;

    rl.setCallBacks(mtSummingCallback, (void *) &sum);

    rl.init();
    
    t1.run(mtSummingThreadFunc, (void *) &rl);
    t2.run(mtSummingThreadFunc, (void *) &rl);    
    t3.run(mtSummingThreadFunc, (void *) &rl);

    rl.run();

    t1.join();
    t2.join();
    t3.join();
    
    CPPUNIT_ASSERT(sum == 1498500);

    rl.shutdown();
}
