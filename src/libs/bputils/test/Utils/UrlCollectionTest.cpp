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

#include "UrlCollectionTest.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpurlcollection.h"

using namespace bp::file;


CPPUNIT_TEST_SUITE_REGISTRATION(URLCollectionTest);

void
URLCollectionTest::bogusPath()
{
    bp::URLCollection coll;
    CPPUNIT_ASSERT(coll.init(Path("/this/path/cant/possibly/exist")) == false);
    // after a failed init, other functions should fail too
    CPPUNIT_ASSERT(coll.add("http://www.yahoo.com") == false);    
    CPPUNIT_ASSERT(coll.has("http://www.yahoo.com") == false);    
}

void
URLCollectionTest::malformedJSON()
{
    const char * badJson[] = {
        // just bogus syntax
        "{", 
        // missing required keys
        "{}", 
        "{\"UseDomainForHTTP\": false}", 
        "{\"urls\": []}",         
        // bad types
        "{\"UseDomainForHTTP\": false, \"urls\": true}", 
        "{\"UseDomainForHTTP\": 'abc', \"urls\": []}"
    };

    // ensure we can delete test file
    CPPUNIT_ASSERT(remove(m_path));

    for (unsigned int i = 0;  i < sizeof(badJson) / sizeof(badJson[0]); i++)
    {
        std::string bj(badJson[i]);
        // ensure we can write test file
        CPPUNIT_ASSERT(bp::strutil::storeToFile(m_path, bj));

        // now try to parse that crap
        bp::URLCollection coll;
        CPPUNIT_ASSERT(coll.init(m_path) == false);
        // after a failed init, other functions should fail too
        CPPUNIT_ASSERT(coll.add("http://www.yahoo.com") == false);    
        CPPUNIT_ASSERT(coll.has("http://www.yahoo.com") == false);    

        // ensure we can delete test file
        CPPUNIT_ASSERT(bp::file::remove(m_path));
    }

    // now as a sanity check, let's try some valid json
    {
        std::string gj("{\"UseDomainForHTTP\": false, \"urls\": []}");
        // ensure we can write test file
        CPPUNIT_ASSERT(bp::strutil::storeToFile(m_path, gj));

        // now try to parse that crap
        bp::URLCollection coll;
        CPPUNIT_ASSERT(coll.init(m_path));
        // after a failed init, other functions should fail too
        CPPUNIT_ASSERT(coll.add("http://www.yahoo.com"));    
        CPPUNIT_ASSERT(coll.has("http://www.yahoo.com"));    

        // ensure we can delete test file
        CPPUNIT_ASSERT(bp::file::remove(m_path));
    }
}


void
URLCollectionTest::fullTest()
{
    // urls to load into collection
    const char * urls[] = {
        "http://www.yahoo.com/ooga/booga", 
        "https://www.google.com/yeah/yeah?foo=bar", 
        "file:///a/temp/file.txt", 
        "http://lloydforge.org/projects/ruby"
    };

    const char * positiveTests[] = {    
        "http://www.yahoo.com/ooga/booga", 
        "https://www.google.com/yeah/yeah?foo=bar", 
        "file:///a/temp/file.txt", 
        "http://lloydforge.org/projects/ruby",
        "http://www.yahoo.com:80/ooga/booga", 
        "https://www.google.com/yeah/yeah?baz=bing", 
        "file:///a/temp/file.txt", 
        "http://lloydforge.org/projects/yajl"
    };

    const char * negativeTests[] = {    
        "", 
        "https://google.com/yeah/yeah?foo=bar", 
        "file:///a/temp/file.tx", 
        "http://www.lloydforge.org/projects/ruby"
    };

    unsigned int i;

    // ensure we can delete test file
    CPPUNIT_ASSERT(bp::file::remove(m_path));
    
    {
        bp::URLCollection coll;

        CPPUNIT_ASSERT(coll.init(m_path));

        // load it up!
        for (i = 0;  i < sizeof(urls) / sizeof(urls[0]); i++) {
            CPPUNIT_ASSERT(coll.add(urls[i]));    
        }

        // positive tests
        for (i = 0; i < sizeof(positiveTests) / sizeof(positiveTests[0]); i++)
        {
            CPPUNIT_ASSERT(coll.has(positiveTests[i]));    
        }

        // negative tests
        for (i = 0; i < sizeof(negativeTests) / sizeof(negativeTests[0]); i++)
        {
            CPPUNIT_ASSERT(coll.has(negativeTests[i]) == false);    
        }
    }

    // now test persistence
    {
        bp::URLCollection coll;

        CPPUNIT_ASSERT(coll.init(m_path));

        // positive tests
        for (i = 0; i < sizeof(positiveTests) / sizeof(positiveTests[0]); i++)
        {
            CPPUNIT_ASSERT(coll.has(positiveTests[i]));    
        }

        // negative tests
        for (i = 0; i < sizeof(negativeTests) / sizeof(negativeTests[0]); i++)
        {
            CPPUNIT_ASSERT(coll.has(negativeTests[i]) == false);    
        }
    }

    // ensure we can delete test file
    CPPUNIT_ASSERT(bp::file::remove(m_path));
}


void 
URLCollectionTest::setUp()
{
	m_path = getTempPath(getTempDirectory(), "URLCollectionTest")
             / "URLCollectionTest.json";
    boost::filesystem::create_directories(m_path.parent_path());
}


void 
URLCollectionTest::tearDown()
{
    CPPUNIT_ASSERT(remove(m_path.parent_path()));
}

