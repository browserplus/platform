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

/**
 * TimerTest.cpp
 *
 * Created by Lloyd Hilaiel 8/21/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "TimerTest.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bptimer.h"


CPPUNIT_TEST_SUITE_REGISTRATION(TimerTest);

class RunLoopStopper : public bp::time::ITimerListener
{
public:
    RunLoopStopper(bp::runloop::RunLoop * rl) : m_rl(rl) { }
private:
    void timesUp(bp::time::Timer *)
    {
        m_rl->stop();
    }
    bp::runloop::RunLoop * m_rl;
};

void
TimerTest::simpleTest()
{
    bp::runloop::RunLoop rl;
    rl.init();

    RunLoopStopper rls(&rl);

    bp::time::Stopwatch sw;
    sw.start();

    bp::time::Timer t;
    t.setListener(&rls);
    t.setMsec(200);

    rl.run();

    sw.stop();
    
    // allow 20ms slop
    CPPUNIT_ASSERT( 0.180 < sw.elapsedSec() && sw.elapsedSec() < 0.220 );

    rl.shutdown();
}
