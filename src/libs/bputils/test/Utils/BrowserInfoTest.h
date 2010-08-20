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
 * BrowserInfoTest.h
 * A test of the bp::BrowserInfo class
 *
 * Created by Gordon Durand on 08/19/10
 * Copyright (c) 2010 Yahoo!, Inc. All rights reserved.
 */

#ifndef __BROWSERINFOTEST_H__
#define __BROWSERINFOTEST_H__

#include "TestingFramework/TestingFramework.h"

class BrowserInfoTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(BrowserInfoTest);
    CPPUNIT_TEST(osxSafari);
    CPPUNIT_TEST(osxFirefox);
    CPPUNIT_TEST(osxChrome);
    CPPUNIT_TEST(winIE6);
    CPPUNIT_TEST(winIE7);
    CPPUNIT_TEST(winIE8);
    CPPUNIT_TEST(winFirefox);
    CPPUNIT_TEST(winSafari);
    CPPUNIT_TEST(winChrome);
    CPPUNIT_TEST_SUITE_END();
    
  protected:
    void osxSafari();
    void osxFirefox();
    void osxChrome();
    void winIE6();
    void winIE7();
    void winIE8();
    void winFirefox();
    void winSafari();
    void winChrome();
};

#endif
