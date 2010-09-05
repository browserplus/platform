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

#include "LocaleTest.h"
#include <iostream>
#include <sstream>
#include "platform_utils/bplocalization.h"


CPPUNIT_TEST_SUITE_REGISTRATION(LocaleTest);

using namespace std;

void
LocaleTest::parseTest()
{
    // result should be "en-GB.ASCII@Default, en-GB.ASCII, en-GB, en" 
    vector<string> candidates = bp::localization::getLocaleCandidates("en-GB.ASCII@Default");
    CPPUNIT_ASSERT(candidates.size() == 4);
    CPPUNIT_ASSERT(candidates[0].compare("en-GB.ASCII@Default") == 0);
    CPPUNIT_ASSERT(candidates[1].compare("en-GB.ASCII") == 0);
    CPPUNIT_ASSERT(candidates[2].compare("en-GB") == 0);
    CPPUNIT_ASSERT(candidates[3].compare("en") == 0);

    // result should be "zh-Hans-US, zh-Hans, zh, en"
    candidates = bp::localization::getLocaleCandidates("zh-Hans-US");
    CPPUNIT_ASSERT(candidates.size() == 4);
    CPPUNIT_ASSERT(candidates[0].compare("zh-Hans-US") == 0);
    CPPUNIT_ASSERT(candidates[1].compare("zh-Hans") == 0);
    CPPUNIT_ASSERT(candidates[2].compare("zh") == 0);
    CPPUNIT_ASSERT(candidates[3].compare("en") == 0);

    // result should be "zh-Hans-US, zh-Hans, zh, en"
    candidates = bp::localization::getLocaleCandidates("zh_Hans_US");
    CPPUNIT_ASSERT(candidates.size() == 4);
    CPPUNIT_ASSERT(candidates[0].compare("zh-Hans-US") == 0);
    CPPUNIT_ASSERT(candidates[1].compare("zh-Hans") == 0);
    CPPUNIT_ASSERT(candidates[2].compare("zh") == 0);
    CPPUNIT_ASSERT(candidates[3].compare("en") == 0);

    // results should be "en-US.UTF-8, en-US, en"
    candidates = bp::localization::getLocaleCandidates("en_US.UTF-8");
    CPPUNIT_ASSERT(candidates.size() == 3);
    CPPUNIT_ASSERT(candidates[0].compare("en-US.UTF-8") == 0);
    CPPUNIT_ASSERT(candidates[1].compare("en-US") == 0);
    CPPUNIT_ASSERT(candidates[2].compare("en") == 0);
}


void 
LocaleTest::setUp()
{
    // empty
}

void
LocaleTest::tearDown()
{
    // empty
}
