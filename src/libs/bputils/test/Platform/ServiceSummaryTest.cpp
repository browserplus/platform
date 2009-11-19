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


#include "ServiceSummaryTest.h"
#include <iostream>
#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/ServiceSummary.h"


CPPUNIT_TEST_SUITE_REGISTRATION(ServiceSummaryTest);


void
ServiceSummaryTest::standaloneTest()
{
    static const char * manifestJson =
        "{"
        "    \"type\": \"standalone\","
        "    \"CoreletLibrary\": \"lib.dll\","
        "    \"strings\": { "
        "        \"en\": { "
        "            \"title\": \"Image Manipulation Plugin\", "
        "            \"summary\": \"Allows authorized websites to manipulate images on your computer.  This includes the ability to rotate, crop, and scale images on your computer from within a web browser.\""
        "        },"
        "        \"bg\": { "
        "            \"title\": \"Инструмент За Снимки\","
        "            \"summary\": \"С този инструмент можеш да манипулираш снимки.\""
        "        }"
        "    },"
        "    \"permissions\" : ["
        "        \"foo\","
        "        \"bar\","
        "        \"baz\""
        "    ]"
        "}";

    std::vector<bp::file::Path> filesToTouch;
    filesToTouch.push_back(bp::file::Path(("lib.dll")));

    // now create it
    createService(manifestJson, filesToTouch);

    // now load it
    bp::service::Summary s;
    std::string error;
    CPPUNIT_ASSERT( s.detectCorelet(m_testServiceDir, error) );

    //  --- now check everything is as expected --- 
    CPPUNIT_ASSERT( s.type() == bp::service::Summary::Standalone );
    CPPUNIT_ASSERT( !s.typeAsString().compare("standalone") );

    // now check that shutdownDelaySecs is set properly
    CPPUNIT_ASSERT_EQUAL( s.shutdownDelaySecs(), -1 );

    // instantiated from a non name/version dir.  they should be left
    // empty
    CPPUNIT_ASSERT( s.name().empty() );
    CPPUNIT_ASSERT( s.version().empty() );

    // should NOT be out of date
    CPPUNIT_ASSERT( !s.outOfDate() );
    
    // check localizations
    std::string title, summary;

    // first en should serve up ok
    CPPUNIT_ASSERT( s.localization("en", title, summary) );
    CPPUNIT_ASSERT( !title.compare("Image Manipulation Plugin") );    

    // and bg should serve up ok
    CPPUNIT_ASSERT( s.localization("bg", title, summary) );
    CPPUNIT_ASSERT( !title.compare("Инструмент За Снимки") );

    // and german should default to english
    CPPUNIT_ASSERT( s.localization("de", title, summary) );
    CPPUNIT_ASSERT( !title.compare("Image Manipulation Plugin") );    

    // empty dependent corelet stuff
    CPPUNIT_ASSERT( s.usesCorelet().empty() );

    CPPUNIT_ASSERT( s.isInitialized() );

    // permissions load OK?
    std::set<std::string> wantPerms;
    wantPerms.insert("foo");
    wantPerms.insert("bar");
    wantPerms.insert("baz");

    std::set<std::string> havePerms = s.permissions();
    CPPUNIT_ASSERT( wantPerms == havePerms );    

    // now verify if we update the disk representation things are
    // considered "out of date"
    tearDown();
    setUp();
    createService(manifestJson, filesToTouch);
    CPPUNIT_ASSERT( s.outOfDate() );
}

void
ServiceSummaryTest::shutdownDelayTest()
{
    static const char * manifestJson =
        "{"
        "    \"type\": \"standalone\","
        "    \"CoreletLibrary\": \"lib.dll\","
        "    \"strings\": { "
        "        \"en\": { "
        "            \"title\": \"foo\", "
        "            \"summary\": \"bar\""
        "        }"
        "    },"
        "    \"shutdownDelaySecs\": 42"
        "}";

    std::vector<bp::file::Path> filesToTouch;
    filesToTouch.push_back(bp::file::Path("lib.dll"));

    // now create it
    createService(manifestJson, filesToTouch);

    // now load it
    bp::service::Summary s;
    std::string error;
    bool detectedService = s.detectCorelet(m_testServiceDir, error);
    CPPUNIT_ASSERT_MESSAGE( error, detectedService );

    // now check that shutdownDelaySecs is set properly
    CPPUNIT_ASSERT( s.shutdownDelaySecs() == 42 );
}


void
ServiceSummaryTest::createService(const char * manifestJson,
                                  std::vector<bp::file::Path> filesToTouch)
{
    bp::file::Path mPath = m_testServiceDir / "manifest.json";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(mPath, manifestJson));
    
    for (unsigned int i = 0; i < filesToTouch.size(); i++) {
        bp::file::Path fPath = m_testServiceDir / filesToTouch[i];
        CPPUNIT_ASSERT(bp::strutil::storeToFile(fPath, "foo"));
    }
}

void ServiceSummaryTest::setUp()
{
    // now create our a test directory
    bp::file::Path dir = bp::file::getTempDirectory();
    dir = bp::file::getTempPath(dir, "ServiceSummaryTest");
    CPPUNIT_ASSERT( boost::filesystem::create_directories(dir) );
    m_testServiceDir = dir;
}

void ServiceSummaryTest::tearDown()
{
    CPPUNIT_ASSERT(bp::file::remove(m_testServiceDir));
}
