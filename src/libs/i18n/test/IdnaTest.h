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
 * IdnaTest.h
 * 
 * Test IDNA facilities.
 *
 * Created by David Grigsby on 1/28/09.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef _IDNATEST_H_
#define _IDNATEST_H_

#include "TestingFramework/TestingFramework.h"

class IdnaTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(IdnaTest);
    CPPUNIT_TEST(testToAscii);
    CPPUNIT_TEST(testToUnicode);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testToAscii();
    void testToUnicode();
};

#endif
