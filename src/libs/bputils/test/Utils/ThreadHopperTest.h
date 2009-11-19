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
 * ThreadHoppertest.h
 * A test of the thread hopper component
 *
 * Created by Lloyd Hilaiel on Fri Jul  6 14:11:57 MDT 2007
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __THREADHOPPERTEST_H__
#define __THREADHOPPERTEST_H__

#include "TestingFramework/TestingFramework.h"
//#include "BPUtils/BPUtils.h"

class ThreadHopperTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(ThreadHopperTest);
    CPPUNIT_TEST(tryHop);
    CPPUNIT_TEST_SUITE_END();
    
  protected:
    void tryHop();
};

#endif
