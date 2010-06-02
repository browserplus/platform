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

#include "IdnaTest.h"
#include "I18n/idna.h"


CPPUNIT_TEST_SUITE_REGISTRATION(IdnaTest);


// Note: these samples from http://demo.icu-project.org/icu-bin/idnbrowser

// UTF-16:  0077 0077 0077 002E 0062 00FC 0063 0068 0065 0072
//          002E 0064 0065
unsigned char utf8_1[] =
{
    0x77, 0x77, 0x77, 0x2E, 0x62,
    0xC3, 0xBC,
    0x63, 0x68, 0x65, 0x72, 0x2E, 0x64, 0x65, 0x00
};

char ascii_1[] = "www.xn--bcher-kva.de";


// UTF16:   0077 0077 0077 002E 30CF 30F3 30C9 30DC 30FC 30EB
//          30B5 30E0 30BA 002E 0063 006F 006D
unsigned char utf8_2[] =
{
    0x77, 0x77, 0x77, 0x2E,
    0xE3, 0x83, 0x8F,
    0xE3, 0x83, 0xB3,
    0xE3, 0x83, 0x89,
    0xE3, 0x83, 0x9C,
    0xE3, 0x83, 0xBC,
    0xE3, 0x83, 0xAB,
    0xE3, 0x82, 0xB5,
    0xE3, 0x83, 0xA0,
    0xE3, 0x82, 0xBA,
    0x2E, 0x63, 0x6F, 0x6D, 0x00
};

char ascii_2[] = "www.xn--vckk7bxa0eza9ezc9d.com"; 


               
void IdnaTest::testToAscii()
{
    CPPUNIT_ASSERT( bp::i18n::idnaToAscii( (const char*)utf8_1 ) == ascii_1 );
    CPPUNIT_ASSERT( bp::i18n::idnaToAscii( (const char*)utf8_2 ) == ascii_2 );
}


void IdnaTest::testToUnicode()
{
    CPPUNIT_ASSERT( bp::i18n::idnaToUnicode(ascii_1) == (const char*)utf8_1 );
    CPPUNIT_ASSERT( bp::i18n::idnaToUnicode(ascii_2) == (const char*)utf8_2 );

    // toUnicode for something already in unicode should return input string.
    std::string sUtf8_2( (const char*)utf8_2 );
    CPPUNIT_ASSERT( bp::i18n::idnaToUnicode( sUtf8_2 ) == sUtf8_2 );
}

