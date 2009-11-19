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
 * HeadersTest.cpp
 * Unit tests for the bp::http::Headers class.
 *
 * Created by David Grigsby on 8/25/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */
#include "HeadersTest.h"
#include "BPUtils/HttpHeaders.h"


CPPUNIT_TEST_SUITE_REGISTRATION(HeadersTest);


void HeadersTest::testAdd()
{
    bp::http::Headers headers;

    std::string s1 = "Date: Mon, 25 Aug 2008 22:37:49 GMT";
    headers.add( s1 );
    
    CPPUNIT_ASSERT( headers.size() == 1 );
    std::string sDate = headers.get( "Date" );
    CPPUNIT_ASSERT( sDate == s1.substr( 6 ) );
}


