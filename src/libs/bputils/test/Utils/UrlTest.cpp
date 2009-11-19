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

#include "UrlTest.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/bpfile.h"

using namespace std;
using namespace bp::file;

CPPUNIT_TEST_SUITE_REGISTRATION(UrlTest);


/////////////////
// Test Ideas
// * Exercise the bp::url::Url class
// * url-encoded chars
// * embedded spaces
// * character case
// * other schemes besides file, esp. http
// * url<->path conversion attempt when scheme not file
// * max length
// * empty input strings
// * invalid strings (e.g. "\\", "//", etc.



////////////////////
// support methods
//
Path roundtripPath( const Path& path )
{
    return pathFromURL( path.url() );
}


string roundtripUrl( const std::string& sUrl )
{
    return pathFromURL( sUrl ).url();
}


////////////////////
// test data
//
// format is: path, url
vector<pair<Path, string> >  g_data;

////////////////////
// UrlTest methods
//

void UrlTest::setUp()
{
    static bool bCalled = false;

    if (!bCalled)
    {
        bCalled = true;

#ifdef WIN32
        // posix format with host name
        g_data.push_back( make_pair( Path(L"//a/b/c.jpg"), "file://a/b/c.jpg" ));
        g_data.push_back( make_pair( Path(L"//.a/b.ext"),  "file://.a/b.ext" ));
        g_data.push_back( make_pair( Path(L"//a:100/c d/e f.jpg"),
                                     "file://a:100/c%20d/e%20f.jpg" ));
        // and now for a drive letter
        g_data.push_back( make_pair( Path(L"c:/foo.txt"),  "file:///c:/foo.txt" ));
#endif
        // posix format 
        g_data.push_back( make_pair( Path("/some/path"),
                                     "file:///some/path" ));
        g_data.push_back( make_pair( Path("/a/b/c.jpg"),
                                     "file:///a/b/c.jpg" ));
        g_data.push_back( make_pair( Path("/.a/b.ext"),
                                     "file:///.a/b.ext" ));
        g_data.push_back( make_pair( Path("/a b/c d/e f.jpg"),
                                     "file:///a%20b/c%20d/e%20f.jpg" ));
    }
}
    

void UrlTest::testIsFileUrl()
{
    for (size_t i=0; i < g_data.size(); ++i)
    {
        CPPUNIT_ASSERT( bp::url::isFileUrl( g_data[i].second ) );
    }
}


void UrlTest::testPathFromUrl()
{
    for (size_t i=0; i < g_data.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL( g_data[i].first,
                              pathFromURL( g_data[i].second ));
    }

    // posix format path from url with localhost host name
    Path p = pathFromURL("file://localhost/b/c.jpg");
    CPPUNIT_ASSERT(p.utf8().compare("/b/c.jpg") == 0);
    p = pathFromURL("file://localhost/c%20d/e%20f.jpg");
    CPPUNIT_ASSERT(p.utf8().compare("/c d/e f.jpg") == 0);
                             
#ifndef WIN32
    // posix format path from url with host name should fail
    p = pathFromURL("file://a/b/c.jpg");
    CPPUNIT_ASSERT(p.empty());
    p = pathFromURL("file://.a/b.ext");
    CPPUNIT_ASSERT(p.empty());
    p = pathFromURL("file://a:100/c d/e f.jpg");
    CPPUNIT_ASSERT(p.empty());
#endif
}


void UrlTest::testUrlFromPath()
{
    for (size_t i=0; i < g_data.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL( g_data[i].second, g_data[i].first.url() );
    }

#ifndef WIN32
    // url from posix format should ignore windows-style host name
    Path p("//a/b/c.jpg");
    CPPUNIT_ASSERT(p.url().compare("file:///a/b/c.jpg") == 0);
    p = "//.a/b.ext";
    CPPUNIT_ASSERT(p.url().compare("file:///.a/b.ext") == 0);
    p = "//a:100/c d/e f.jpg";
    CPPUNIT_ASSERT(p.url().compare("file:///a%3A100/c%20d/e%20f.jpg") == 0);
#endif
}


void UrlTest::testPathRoundtrip()
{
    for (size_t i=0; i < g_data.size(); ++i)
    {
        Path p = g_data[i].first;
        CPPUNIT_ASSERT_EQUAL( p, roundtripPath( p ) );
    }
}


void UrlTest::testUrlRoundtrip()
{
    for (size_t i=0; i<g_data.size(); ++i)
    {
        string sUrl = g_data[i].second;
        CPPUNIT_ASSERT_EQUAL( sUrl, roundtripUrl( sUrl ) );
    }
}


#ifdef WIN32
void UrlTest::testNonAscii()
{
    unsigned char nonAscii[] = {
        0xd0, 0x9f, 0xd0, 0xb5,
        0xd1, 0x88, 0xd0, 0xbe };
    wstring s(L"C:/Users/");
    for (size_t i = 0; i < sizeof(nonAscii); i++) {
        s.push_back(nonAscii[i]);
    }
    Path p(s);
    CPPUNIT_ASSERT_EQUAL(p, roundtripPath(p));
}
#endif


void UrlTest::testUrlAppendPath()
{
    static struct {
        const char * input;
        const char * append;        
        const char * expected;
    } appendPathTests[] = {
        // relative path creation at root
        {
            "http://foo.com/index.html",
            "biteme.php",
            "http://foo.com/biteme.php"
        },
        // relative path creation at depth 1 in path
        {
            "http://foo.com/bar/index.html",
            "biteme.php",
            "http://foo.com/bar/biteme.php"
        },
        // relative path creation at depth n in path
        {
            "http://foo.com/bar/baz/bing/boom/index.html",
            "biteme.php",
            "http://foo.com/bar/baz/bing/boom/biteme.php"
        },
        // absolute path overwriting
        {
            "http://foo.com/index.html",
            "/biteme.php",
            "http://foo.com/biteme.php"
        }

    };

    for (unsigned int i = 0;
         i < sizeof(appendPathTests)/sizeof(appendPathTests[0]);
         i++)
    {
        std::string expected(appendPathTests[i].expected);
        std::string err;
        bp::url::Url u;
        CPPUNIT_ASSERT_MESSAGE(err, u.parse(appendPathTests[i].input, err));
        CPPUNIT_ASSERT(u.appendPath(appendPathTests[i].append));
        CPPUNIT_ASSERT_EQUAL(expected, u.toString());
    }

}


void UrlTest::testUrlDirname()
{
    typedef vector<pair<string,string> > tStrPairVec;
    tStrPairVec data;

//  data.push_back( make_pair( "http://", "http://" ));
//	data.push_back( make_pair( "http://a", "http://a/" ));
    data.push_back( make_pair( "http://a/", "http://a/" ));
    data.push_back( make_pair( "http://a/b", "http://a/" ));
    data.push_back( make_pair( "http://a/b/", "http://a/" ));
    data.push_back( make_pair( "http://a/b/c", "http://a/b/" ));
    data.push_back( make_pair( "http://a/b/c/", "http://a/b/" ));
    data.push_back( make_pair( "http://a.b.c/d/e", "http://a.b.c/d/" ));
    data.push_back( make_pair( "http://a.b.c/d/e/", "http://a.b.c/d/" ));
    data.push_back( make_pair( "http://a.b.c/d/e?f=g", "http://a.b.c/d/" ));
    
    for (tStrPairVec::iterator it = data.begin(); it != data.end(); ++it)
    {
        string err;
        bp::url::Url url;
        CPPUNIT_ASSERT_MESSAGE(err, url.parse(it->first, err));
        CPPUNIT_ASSERT(url.dirname());
        CPPUNIT_ASSERT_EQUAL(it->second, url.toString());
    }
}

