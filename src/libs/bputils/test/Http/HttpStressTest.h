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
 * HttpStressTest.h
 * Stresses the snot out of the HTTP client implementation.
 *
 * Created by Lloyd Hilaiel on 7/18/08.
 */

#ifndef __HTTPSTRESSTEST_H__
#define __HTTPSTRESSTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "TestServer.h"
#include "BPUtils/bpfile.h"

class HttpStressTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(HttpStressTest);
    CPPUNIT_TEST(beatTheSnotOutOfIt);
    CPPUNIT_TEST_SUITE_END();

public:

// Tests    
private:
    // Test a synchronous http text get.
    void beatTheSnotOutOfIt();
};

#endif
