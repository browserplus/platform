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


#include "UuidTest.h"
#include "BPUtils/bpuuid.h"


CPPUNIT_TEST_SUITE_REGISTRATION(UuidTest);

void UuidTest::length()
{
    std::string sUuid;

    CPPUNIT_ASSERT( bp::uuid::generate( sUuid ) );

    // We expect a uuid of the form: bd32e0d6-0356-4fe2-a42e-eea0a88b8b96
    CPPUNIT_ASSERT( sUuid.length() == 36 );
}

void UuidTest::uniqueness()
{
    std::string s1;
    CPPUNIT_ASSERT( bp::uuid::generate( s1 ) );

    std::string s2;
    CPPUNIT_ASSERT( bp::uuid::generate( s2 ) );

    CPPUNIT_ASSERT( s2 != s1 );
}

