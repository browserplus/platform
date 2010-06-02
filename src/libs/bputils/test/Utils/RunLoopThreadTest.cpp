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

/**
 * RunLoopTest.cpp
 * Unit tests for the bp::runloop component
 *
 * Created by Lloyd Hilaiel on 6/20/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "RunLoopThreadTest.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/bprunloopthread.h"

#include <stdlib.h>

CPPUNIT_TEST_SUITE_REGISTRATION(RunLoopThreadTest);

static void setToTrue(void * boolPtr) 
{
    *((bool *) boolPtr) = true;
}

static void processEventSumming(void * intPtr, bp::runloop::Event e) 
{
    *((int *) intPtr) += (long) e.payload();
}


void RunLoopThreadTest::basicTest()
{
    bool called[2];
    called[0] = false;
    called[1] = false;    
    int sum = 0;

    bp::runloop::RunLoopThread rl;
    rl.setCallBacks(setToTrue, (void *) called,
                    setToTrue, (void *) (called + 1),
                    processEventSumming, &sum);

    CPPUNIT_ASSERT( !called[0] );    
    CPPUNIT_ASSERT( !called[1] );    

    CPPUNIT_ASSERT( rl.run() );

    CPPUNIT_ASSERT( called[0] );    
    CPPUNIT_ASSERT( !called[1] );    

    rl.sendEvent(bp::runloop::Event((void *) 1));
    rl.sendEvent(bp::runloop::Event((void *) 2));
    rl.sendEvent(bp::runloop::Event((void *) 3));
    rl.sendEvent(bp::runloop::Event((void *) 4));
    
    CPPUNIT_ASSERT( rl.stop() );    
    CPPUNIT_ASSERT( sum == 10 );    

    CPPUNIT_ASSERT( called[0] );    
    CPPUNIT_ASSERT( called[1] );    
}

struct TTEvent
{
    enum { Stop, Sum} action;
    union {
        int val;
        bp::runloop::RunLoopThread * rl;    
    } u;
};

static void processEventStopAndSumming(void * intPtr, bp::runloop::Event e) 
{
    TTEvent * ev = (TTEvent *) e.payload();

    if (ev->action == TTEvent::Stop)  {
        ev->u.rl->stop();
    } else if (ev->action == TTEvent::Sum) {
        *((int *) intPtr) += ev->u.val;
    } else {
        abort();
    }
    
    delete ev;
}

void RunLoopThreadTest::joinTest()
{
    bool called[2];
    called[0] = false;
    called[1] = false;    
    int sum = 0;

    bp::runloop::RunLoopThread rl;
    rl.setCallBacks(setToTrue, (void *) called,
                    setToTrue, (void *) (called + 1),
                    processEventStopAndSumming, &sum);

    CPPUNIT_ASSERT( !called[0] );    
    CPPUNIT_ASSERT( !called[1] );    

    CPPUNIT_ASSERT( rl.run() );

    CPPUNIT_ASSERT( called[0] );    
    CPPUNIT_ASSERT( !called[1] );    

    TTEvent * e = new TTEvent;
    e->action = TTEvent::Sum;
    e->u.val = 1;
    rl.sendEvent(bp::runloop::Event((void *) e));
    e = new TTEvent;
    e->action = TTEvent::Sum;
    e->u.val = 2;
    rl.sendEvent(bp::runloop::Event((void *) e));
    e = new TTEvent;
    e->action = TTEvent::Sum;
    e->u.val = 3;
    rl.sendEvent(bp::runloop::Event((void *) e));
    e = new TTEvent;
    e->action = TTEvent::Sum;
    e->u.val = 4;
    rl.sendEvent(bp::runloop::Event((void *) e));
    e = new TTEvent;
    e->action = TTEvent::Stop;
    e->u.rl = &rl;
    rl.sendEvent(bp::runloop::Event((void *) e));
    rl.join();

    CPPUNIT_ASSERT_EQUAL( (int) 10, sum );    
    CPPUNIT_ASSERT( called[0] );    
    CPPUNIT_ASSERT( called[1] );    
}

void RunLoopThreadTest::stopBeforeRunTest()
{
    bp::runloop::RunLoopThread rl;
    CPPUNIT_ASSERT_THROW(rl.stop(), bp::error::FatalException);
}

void RunLoopThreadTest::joinBeforeRunTest()
{
    bp::runloop::RunLoopThread rl;
    CPPUNIT_ASSERT(!rl.join());
}

void RunLoopThreadTest::doubleStopTest()
{
    bp::runloop::RunLoopThread rl;
    CPPUNIT_ASSERT( rl.run() );
    CPPUNIT_ASSERT( rl.stop() );    
    CPPUNIT_ASSERT( rl.stop() );
}

void RunLoopThreadTest::stopAndJoinTest()
{
    bp::runloop::RunLoopThread rl;
    CPPUNIT_ASSERT( rl.run() );
    CPPUNIT_ASSERT( rl.stop() );    
    CPPUNIT_ASSERT( rl.join() );
}
