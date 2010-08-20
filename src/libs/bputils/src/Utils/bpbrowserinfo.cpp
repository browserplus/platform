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

/*
 *  bpbrowserinfo.cpp
 *
 *  Parses a user agent string and returns browser info from it
 *
 *  Created by Gordon Durand on 08/19/10
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpbrowserinfo.h"
#include "bperrorutil.h"

using namespace std;

namespace bp { 

BrowserInfo::BrowserInfo(const std::string& userAgent)
{
    // A sample Windows IE8 userAgent
    //    'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C)'
    // A sample Windows Firefox 3.6.3 userAgent
    //    'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3'
    // A sample OSX Safari 4.0.3 userAgent that we're snorting thru
    //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6; en-us) AppleWebKit/531.9 (KHTML, like Gecko) Version/4.0.3 Safari/531.9'
    // A sample OSX Chrome userAgent string
    //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-US) AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.38 Safari/533.4'

    // first grab platform
    if (userAgent.find("Windows") != string::npos) {
        m_platform = "Windows";
    } else if (userAgent.find("Mac OS X") != string::npos) {
        m_platform = "OSX";
    } else {
        BP_THROW("unknown platform in '" + userAgent + "'");
    }

    // now figure out the browser
    string browser;
    if (userAgent.find("MSIE") != string::npos) {
        m_browser = "IE";
    } else if (userAgent.find("Firefox") != string::npos) {
        m_browser = "Firefox";
    } else if (userAgent.find("Chrome") != string::npos) {
        m_browser = "Chrome";
    } else if (userAgent.find("Safari") != string::npos) {
        m_browser = "Safari";
    } else {
        BP_THROW("unknown browser in '" + userAgent + "'");
    }
    
    // now figure out the version
    size_t start = 0;
    size_t len = string::npos;
    if (m_browser == "IE") {
        // 'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C)'
        start = userAgent.find("MSIE ") + strlen("MSIE ");
        len = userAgent.find(";", start) - start;
    } else if (m_browser == "Firefox") {
        // 'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3'
        start = userAgent.find("Firefox/") + strlen("Firefox/");
        len = string::npos;
    } else if (m_browser == "Safari") {
        // 'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6; en-us) AppleWebKit/531.9 (KHTML, like Gecko) Version/4.0.3 Safari/531.9'
        start = userAgent.find("Version/") + strlen("Version/");
        len = userAgent.find(" ", start) - start;
    } else if (m_browser == "Chrome") {
        // 'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-US) AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.38 Safari/533.4'
        start = userAgent.find("Chrome/") + strlen("Chrome/");
        size_t end = userAgent.find(" ", start);
        len = userAgent.rfind(".", end) - start;
    }
    
    string vstr = userAgent.substr(start, len);
    if (!m_version.parse(vstr)) {
        BP_THROW("unable to parse version'" + vstr
                 + "', userAgent = '" + userAgent + "'");
    }
}

}

