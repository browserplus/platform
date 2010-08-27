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

#include "BrowserInfoTest.h"
#include "BPUtils/bpbrowserinfo.h"
#include "BPUtils/bperrorutil.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(BrowserInfoTest);

void
BrowserInfoTest::osxSafari()
{
    string ua = "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_4; en-us) "
                "AppleWebKit/533.17.8 (KHTML, like Gecko) Version/5.0.1 "
                "Safari/533.17.8";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "OSX");
    CPPUNIT_ASSERT(info.browser() == "Safari");
    CPPUNIT_ASSERT(info.version().majorVer() == 5);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == 1);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);
}


void
BrowserInfoTest::osxFirefox()
{
    string ua = "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; en-US; "
                "rv:1.9.2.8) Gecko/20100722 Firefox/3.6.8";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "OSX");
    CPPUNIT_ASSERT(info.browser() == "Firefox");
    CPPUNIT_ASSERT(info.version().majorVer() == 3);
    CPPUNIT_ASSERT(info.version().minorVer() == 6);
    CPPUNIT_ASSERT(info.version().microVer() == 8);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);
}


void
BrowserInfoTest::osxChrome()
{
    string ua = "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_4; en-US) "
                "AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.126 "
                "Safari/533.4";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "OSX");
    CPPUNIT_ASSERT(info.browser() == "Chrome");
    CPPUNIT_ASSERT(info.version().majorVer() == 5);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == 375);
    CPPUNIT_ASSERT(info.version().nanoVer() == 126);
}


void
BrowserInfoTest::winIE6()
{
    string ua = "Mozilla/4.0 (compatible; MSIE 6.0; Windows 98; "
                "Rogers Hi·Speed Internet; (R1 1.3))";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "IE");
    CPPUNIT_ASSERT(info.version().majorVer() == 6);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == -1);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);
}


void
BrowserInfoTest::winIE7()
{
    string ua = "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; "
                "GTB6.4; .NET CLR 1.1.4322; FDM; .NET CLR 2.0.50727; "
                ".NET CLR 3.0.04506.30; .NET CLR 3.0.4506.2152; "
                ".NET CLR 3.5.30729)";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "IE");
    CPPUNIT_ASSERT(info.version().majorVer() == 7);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == -1);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);
}


void
BrowserInfoTest::winIE8()
{
    string ua = "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; "
                "Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; "
                ".NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C)";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "IE");
    CPPUNIT_ASSERT(info.version().majorVer() == 8);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == -1);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);
}


void
BrowserInfoTest::winFirefox()
{
    string ua = "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.3) "
                "Gecko/20100401 Firefox/3.6.6";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "Firefox");
    CPPUNIT_ASSERT(info.version().majorVer() == 3);
    CPPUNIT_ASSERT(info.version().minorVer() == 6);
    CPPUNIT_ASSERT(info.version().microVer() == 6);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);   

    // vista/ffx can have different userAgents (of course)
    ua = "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.8) "
         "Gecko/20100722 Firefox/3.6.8 (.NET CLR 3.5.30729; .NET4.0C)";
    info = bp::BrowserInfo(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "Firefox");
    CPPUNIT_ASSERT(info.version().majorVer() == 3);
    CPPUNIT_ASSERT(info.version().minorVer() == 6);
    CPPUNIT_ASSERT(info.version().microVer() == 8);
    CPPUNIT_ASSERT(info.version().nanoVer() == -1);   
}


void
BrowserInfoTest::winSafari()
{
    string ua = "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) "
                "AppleWebKit/533.17.8 (KHTML, like Gecko) Version/5.0.1 "
                "Safari/533.17.8";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "Safari");
    CPPUNIT_ASSERT(info.version().majorVer() == 5);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == 1);
}


void
BrowserInfoTest::winChrome()
{
    string ua = "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) "
                "AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.126 "
                "Safari/533.4";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform() == "Windows");
    CPPUNIT_ASSERT(info.browser() == "Chrome");
    CPPUNIT_ASSERT(info.version().majorVer() == 5);
    CPPUNIT_ASSERT(info.version().minorVer() == 0);
    CPPUNIT_ASSERT(info.version().microVer() == 375);
    CPPUNIT_ASSERT(info.version().nanoVer() == 126);
}


void
BrowserInfoTest::badUserAgent()
{
    string ua = "Mozilla/5.0 (Macintosh; U; Intel Mac I-AM-BAD  10_6_4; en-US) "
                "AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.126 "
                "Safari/533.4";
    bp::BrowserInfo info(ua);
    CPPUNIT_ASSERT(info.platform().empty());
    CPPUNIT_ASSERT(info.browser().empty());
    CPPUNIT_ASSERT(info.version().asString().empty());
    CPPUNIT_ASSERT(info.supported() == false);
    CPPUNIT_ASSERT(info.capabilities().empty());
}

