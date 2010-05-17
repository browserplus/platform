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
    // Currently, Safari 4.0.3 or greater on OSX uses Html5, everybody
    // else uses Intercept.  Chrome on OSX doesn't support DnD.

    // access user agent
    NPAPIPlugin* plugin = (NPAPIPlugin*)instance->pdata;
    std::string userAgent = plugin->getUserAgent();
    BPLOG_DEBUG_STRM("DropManagerFactory userAgent: '" << userAgent << "'");

    // A sample OSX Safari 4.0.3 userAgent that we're snorting thru
    // 'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6; en-us) AppleWebKit/531.9 (KHTML, like Gecko) Version/4.0.3 Safari/531.9'
    // A sample OSX Chrome userAgent string
    // 'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-US) AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.38 Safari/533.4'
    bool useIntercept = true;
    bool unsupported = false;
    do {
        if (userAgent.find("Mac OS X") == string::npos) break;
        if (userAgent.find("Chrome") != string::npos) {
            unsupported = true;
            break;
        }
        if (userAgent.find("Safari") == string::npos) break;
        size_t start = userAgent.find("Version/");
        if (start == string::npos) break;
        size_t end = userAgent.find(" ", start);
        if (end == string::npos) break;
        string s = userAgent.substr(start, end - start);
        vector<string> v = bp::strutil::split(s, "/");
        if (v.size() < 2) break;
        string version = v[1];
        bp::ServiceVersion baseVersion;
        baseVersion.parse("4.0.3");
        bp::ServiceVersion thisVersion;
        if (!thisVersion.parse(version)) break;
        useIntercept = thisVersion.compare(baseVersion) < 0;
    } while (false);

    if (unsupported) {
        BPLOG_WARN("DnD unsupported on platform/browser");
        return NULL;
    }
    if (useIntercept) {
        return InterceptDropManager::allocate(instance, window, listener);
    }
    return Html5DropManager::allocate(instance, window, listener);
}


void
DropManagerFactory::destroy(IDropManager* mgr)
{
    delete mgr;
}

