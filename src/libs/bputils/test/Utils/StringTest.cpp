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
 * StringTest.cpp
 * Unit tests for the bp::strutil facility
 *
 * Created by David Grigsby on 8/21/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */
#include "StringTest.h"
#include "BPUtils/bpstrutil.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(StringTest);


void StringTest::testSplit()
{
    // If you give an empty string you get it back.
    string s0;
    vector<string> vs0 = bp::strutil::split( s0, "," );
    CPPUNIT_ASSERT( vs0.size() == 1 );
    CPPUNIT_ASSERT( vs0[0] == "" );

    string s1 = "Lago Maggiore";
    vector<string> vs1 = bp::strutil::split( s1, "," );
    CPPUNIT_ASSERT( vs1.size() == 1 );
    CPPUNIT_ASSERT( vs1[0] == "Lago Maggiore" );
    
    string s2 = "a,b,c";
    vector<string> vs2 = bp::strutil::split( s2, "," );
    CPPUNIT_ASSERT( vs2.size() == 3 );
    CPPUNIT_ASSERT( vs2[0] == "a" );
    CPPUNIT_ASSERT( vs2[1] == "b" );
    CPPUNIT_ASSERT( vs2[2] == "c" );

    // Note if string starts with delimiter, first member of result vec
    // will be empty.  This is boost behavior and agrees with JS
    // String.split() as described in Rhino.
    string s3 = ",d , e, fg ";
    vector<string> vs3 = bp::strutil::split( s3, "," );
    CPPUNIT_ASSERT( vs3.size() == 4 );
    CPPUNIT_ASSERT( vs3[0] == "" );
    CPPUNIT_ASSERT( vs3[1] == "d " );
    CPPUNIT_ASSERT( vs3[2] == " e" );
    CPPUNIT_ASSERT( vs3[3] == " fg " );

    // Similarly, if delimiter appears at end of input, the final result
    // member will be an empty string.
    string s4 = "h,";
    vector<string> vs4 = bp::strutil::split( s4, "," );
    CPPUNIT_ASSERT( vs4.size() == 2 );
    CPPUNIT_ASSERT( vs4[0] == "h" );
    CPPUNIT_ASSERT( vs4[1] == "" );
    
    string s5 = "color:red\r\nelement:iridium\r\ngame:volleyball";
    vector<string> vs5 = bp::strutil::split( s5, "\r\n" );
    CPPUNIT_ASSERT( vs5.size() == 3 );
    CPPUNIT_ASSERT( vs5[0] == "color:red" );
    CPPUNIT_ASSERT( vs5[1] == "element:iridium" );
    CPPUNIT_ASSERT( vs5[2] == "game:volleyball" );

    string s6 = "Date: Mon, 25 Aug 2008 22:37:49 GMT";
    vector<string> vs6 = bp::strutil::split( s6, ": " );
    CPPUNIT_ASSERT( vs6.size() == 2 );
    CPPUNIT_ASSERT( vs6[1] == s6.substr( 6 ) );
}


void StringTest::testWideToUtf8()
{
    wstring ws;
    ws.push_back( 0x0080 );

    string s = bp::strutil::wideToUtf8( ws );
    CPPUNIT_ASSERT_EQUAL( (unsigned char)0xC2, (unsigned char)s[0] );
    CPPUNIT_ASSERT_EQUAL( (unsigned char)0x80, (unsigned char)s[1] );

    wstring ws2( L"howdy" );
    string s2 = bp::strutil::wideToUtf8( ws2 );
    CPPUNIT_ASSERT_EQUAL( s2 , string("howdy") );
}


void StringTest::testUtf8ToWide()
{
    // TODO: fails on mac. 
#ifdef WIN32
    unsigned char utf8_1[] =
    {
        0xD0, 0x9F, 0xD0, 0xB5, 0xD1, 0x88, 0xD0, 0xBE, 0x00
    };

    wstring ws = bp::strutil::utf8ToWide( (const char*) utf8_1 );
    CPPUNIT_ASSERT( ws.length() == 4 );
    CPPUNIT_ASSERT( ws[0] == 0x041F );
    CPPUNIT_ASSERT( ws[1] == 0x0435 );
    CPPUNIT_ASSERT( ws[2] == 0x0448 );
    CPPUNIT_ASSERT( ws[3] == 0x043E );
#endif
}

