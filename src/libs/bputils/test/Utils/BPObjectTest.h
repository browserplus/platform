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
 * BPObjectTest.h
 * Unit tests for the in memory object representation (stored in
 * format appropriate for transmission over boundaries that use C types
 * defined in ServiceAPI)
 *
 * Created by Lloyd Hilaiel on 7/24/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __BPOBJECTTEST_H__
#define __BPOBJECTTEST_H__

#include "TestingFramework/TestingFramework.h"
//#include "BPUtils/BPUtils.h"

class BPObjectTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(BPObjectTest);
    CPPUNIT_TEST(boolTest);
    CPPUNIT_TEST(nullTest);
    CPPUNIT_TEST(doubleTest);
    CPPUNIT_TEST(stringTest);
    CPPUNIT_TEST(pathTest);
    CPPUNIT_TEST(integerTest);
    CPPUNIT_TEST(callbackTest);
    CPPUNIT_TEST(listTest);
    CPPUNIT_TEST(mapTest);
    CPPUNIT_TEST(exceptionTest);
    CPPUNIT_TEST(parsingTest);
    CPPUNIT_TEST_SUITE_END();
    
protected:
    void boolTest();
    void nullTest();
    void doubleTest();
    void stringTest();
    void pathTest();
    void integerTest();
    void callbackTest();
    void listTest();
    void mapTest();
    void exceptionTest();
    void parsingTest();
};

#endif
