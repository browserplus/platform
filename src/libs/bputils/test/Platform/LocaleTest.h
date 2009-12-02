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
 * Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * LocaleTest.h
 * Unit tests for locale parsing
 *
 * Created by Gordon Durand on 02/20/09.
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __LOCALETEST_H__
#define __LOCALETEST_H__

#include "TestingFramework/TestingFramework.h"
//#include "BPUtils/BPUtils.h"

#include <vector>
#include <string>

class LocaleTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(LocaleTest);
    CPPUNIT_TEST(parseTest);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    
  protected:
    void parseTest();
};

#endif
