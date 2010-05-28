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
 *  DropManagerFactory.cpp
 *  BrowserPlusPlugin
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#include <string>
#include "BPUtils/bpserviceversion.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "NPAPIPlugin.h"
#include "DropManagerFactory.h"
#include "InterceptDropManager.h"
#include "Html5DropManager.h"

using namespace std;

IDropManager* 
DropManagerFactory::create(NPP instance,
                           NPWindow* window,
                           IDropListener* listener)
{
    // Curent Html5 users:
    //  - Firefox 3.6.3 or greater on Windows
    //  - Safari 4.0.3 or greater on OSX
    // Everybody else uses Intercept.
    // Chrome on OSX doesn't support DnD.

    // access user agent
    NPAPIPlugin* plugin = (NPAPIPlugin*)instance->pdata;
    string userAgent = plugin->getUserAgent();
    BPLOG_DEBUG_STRM("DropManagerFactory userAgent: '" << userAgent << "'");

    // A sample Windows Firefox 3.6.3 userAgent
    //    'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3'
    // A sample OSX Safari 4.0.3 userAgent that we're snorting thru
    //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6; en-us) AppleWebKit/531.9 (KHTML, like Gecko) Version/4.0.3 Safari/531.9'
    // A sample OSX Chrome userAgent string
    //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-US) AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.38 Safari/533.4'
    bool unsupported = false;
    bool useHtml5 = false;
    bool isWindows = userAgent.find("Windows") != string::npos;
    bool isOSX = userAgent.find("Mac OS X") != string::npos;
    do {
        string browser;
        bp::ServiceVersion baseVersion;
        if (isOSX && userAgent.find("Chrome")) {
            unsupported = true;
            break;
        }
        if (isOSX && userAgent.find("Safari")) {
            browser = "Safari";
            (void) baseVersion.parse("4.0.3");
        }
        if (isWindows && userAgent.find("Firefox")) {
            (void) baseVersion.parse("3.6.3");
            browser = "Firefox";
            // add a trailing " " so that parsing code below will work
            userAgent += " ";
        }
        std::string vstr = browser == "Safari" ? "Version/" : browser + "/";
        size_t start = userAgent.find(vstr);
        if (start == string::npos) break;
        size_t end = userAgent.find(" ", start);
        if (end == string::npos) break;
        string s = userAgent.substr(start, end - start);
        vector<string> v = bp::strutil::split(s, "/");
        if (v.size() < 2) break;
        bp::ServiceVersion thisVersion;
        if (thisVersion.parse(v[1])) {
            useHtml5 = thisVersion.compare(baseVersion) >= 0;
        }
    } while (false);

    if (unsupported) {
        BPLOG_WARN_STRM("DnD unsupported for " << userAgent);
        return NULL;
    }

    BPLOG_INFO_STRM("using " << string(useHtml5 ? "html5" : "intercept")
                    << " drop manager for " << userAgent);
    return useHtml5 ? Html5DropManager::allocate(instance, window, listener):
                      InterceptDropManager::allocate(instance, window, listener);
}


void
DropManagerFactory::destroy(IDropManager* mgr)
{
    delete mgr;
}

