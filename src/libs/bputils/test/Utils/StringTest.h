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
 * StringTest.h
 * Unit tests for the bp::string facility
 *
 * Created by David Grigsby on 8/21/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */
#ifndef __STRINGTEST_H__
#define __STRINGTEST_H__

#include "TestingFramework/TestingFramework.h"
//#include "BPUtils/BPUtils.h"



class StringTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(StringTest);
    CPPUNIT_TEST(testSplit);
    CPPUNIT_TEST(testUtf8ToWide);
    CPPUNIT_TEST(testWideToUtf8);
    CPPUNIT_TEST_SUITE_END();
    
protected:
    void testSplit();
    void testUtf8ToWide();
    void testWideToUtf8();
};

#endif
