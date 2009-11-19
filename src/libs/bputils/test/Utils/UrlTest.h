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
 * UrlTest.h
 * Unit tests for bp::url methods
 *
 * Created by David Grigsby on 4/28/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef URLTEST_H_
#define URLTEST_H_

#include "TestingFramework/TestingFramework.h"


class UrlTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(UrlTest);
    CPPUNIT_TEST(testIsFileUrl);
    CPPUNIT_TEST(testPathFromUrl);
    CPPUNIT_TEST(testUrlFromPath);
    CPPUNIT_TEST(testPathRoundtrip);
    CPPUNIT_TEST(testUrlRoundtrip);
#ifdef WIN32
    CPPUNIT_TEST(testNonAscii);
#endif
    CPPUNIT_TEST(testUrlAppendPath);
    CPPUNIT_TEST(testUrlDirname);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual void setUp();
        
protected:
    void testIsFileUrl();
    void testPathFromUrl();
    void testUrlFromPath();
    void testPathRoundtrip();
    void testUrlRoundtrip();
#ifdef WIN32
    void testNonAscii();
#endif
    void testUrlAppendPath();
    void testUrlDirname();
};

#endif
