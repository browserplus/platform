/** * ***** BEGIN LICENSE BLOCK *****
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
 * SemanticVersionTest.h
 * Unit tests for bp::SemanticVersion class.
 *
 * Created by Gordon Durand on 8/27/10.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __BPSEMANTICVERSIONTEST_H__
#define __BPSEMANTICVERSIONTEST_H__

#include "TestingFramework/TestingFramework.h"

class SemanticVersionTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(SemanticVersionTest);
    CPPUNIT_TEST(matchTest);
    CPPUNIT_TEST(compareTest);
	CPPUNIT_TEST(rangeTest);
    CPPUNIT_TEST(wildNanoTest);
	CPPUNIT_TEST(wildMicroTest);
	CPPUNIT_TEST(wildMinorTest);
	CPPUNIT_TEST(wildMajorTest);
	CPPUNIT_TEST_SUITE_END();

protected:
    void matchTest();
	void compareTest();
	void rangeTest();
    void wildNanoTest();
	void wildMicroTest();
	void wildMinorTest();
	void wildMajorTest();
};

#endif

