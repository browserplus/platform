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

#include "FormatTimeTest.h"
#include "BPUtils/bptime.h"
#include "I18n/FormatTime.h"


CPPUNIT_TEST_SUITE_REGISTRATION(RelativeTimeTest);

              
void RelativeTimeTest::testYesterday()
{
    const int knSecondsPerDay = 60*60*24;
    BPTime tm;
    tm.set( tm.get() - knSecondsPerDay );
    
    CPPUNIT_ASSERT_EQUAL( std::string("Yesterday"),
                          bp::i18n::formatRelativeTime( tm, "en-US" ) );
    
    CPPUNIT_ASSERT_EQUAL( std::string("ayer"),
                          bp::i18n::formatRelativeTime( tm, "es-ES" ) );
    
    CPPUNIT_ASSERT_EQUAL( std::string("hier"),
                          bp::i18n::formatRelativeTime( tm, "fr-FR" ) );
}


