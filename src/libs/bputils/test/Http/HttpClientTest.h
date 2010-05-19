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
 * HttpClientTest.h
 * Tests of the basic functionality of the BPUtils HTTP client
 *
 * Created by Lloyd Hilaiel on 7/18/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __HTTPCLIENTTEST_H__
#define __HTTPCLIENTTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "TestServer.h"
#include "BPUtils/bpfile.h"

class HttpClientTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(HttpClientTest);
    CPPUNIT_TEST(testTextGet);
    CPPUNIT_TEST(testTextGetAsync);
    CPPUNIT_TEST(testChunkedResponseProgress);
    CPPUNIT_TEST(testSlowGetAsync);
    CPPUNIT_TEST(testNotFound);
    CPPUNIT_TEST(testBinaryGet);
    CPPUNIT_TEST(testBinaryGetAsync);
    CPPUNIT_TEST(testRedirect);
    CPPUNIT_TEST(testPost);
    CPPUNIT_TEST(testPostAsync);
    CPPUNIT_TEST(testPostCRLF);
    CPPUNIT_TEST(testServerDelay);
    CPPUNIT_TEST(testTimeout);
    CPPUNIT_TEST(testTimeoutAsync);
    CPPUNIT_TEST(testCancelAsync);
    CPPUNIT_TEST(testCookies);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual void setUp();
    virtual void tearDown();

// Tests    
private:
    // Test a synchronous http text get.
    void testTextGet();
    
    // Test an asynchronous http text get.
    void testTextGetAsync();

    // Test our progress reporting when response uses chunked encoding.
    void testChunkedResponseProgress();
    
    // Test an asynchronous http get where the response packets are delayed.
    void testSlowGetAsync();

    // Test a get with "not found" returned from the server.
    void testNotFound();

    // Test a get of binary content.
    void testBinaryGet();

    // Test an async get of binary content.
    void testBinaryGetAsync();

    // Test we can http POST and get the same body back in response.
    void testPost();
    
    // Test we can async http POST and get the same body back in response.
    void testPostAsync();

    // Test posting text with \r\n
    void testPostCRLF();

    // Verifies that our test server will delay its response by the amount
    // we request.
    void testServerDelay();

    // Test request timeout functionality.
    void testTimeout();
    void testTimeoutAsync();
    
    // Test cancel.
    void testCancelAsync();

    // Test cookie behavior.
    void testCookies();

    // Test HTTP redirect handling.
    void testRedirect();
    
// Support
private:    
    // Save specified http body to binary file.
    void saveBodyToBinaryFile( const bp::file::Path& path,
                               const bp::http::Body& body );
    
private:    
    TestServer m_testServer;
};

#endif
