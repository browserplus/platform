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
 * MessageTest.h
 * Unit tests for the simple IPC Message representation.
 *
 * Created by Lloyd Hilaiel on 7/30/08 (somewhere over the atlantic)
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __MESSAGETEST_H__
#define __MESSAGETEST_H__

#include "TestingFramework/TestingFramework.h"

class MessageTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(MessageTest);
    CPPUNIT_TEST(basicMessageTest);
    CPPUNIT_TEST(basicResponseTest);
    CPPUNIT_TEST_SUITE_END();
    
protected:
    void basicMessageTest();
    void basicResponseTest();
};

#endif
