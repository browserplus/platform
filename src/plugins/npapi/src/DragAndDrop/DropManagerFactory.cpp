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
#include "BPUtils/bpbrowserinfo.h"
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
    // access user agent and get browserinfo from it
    NPAPIPlugin* plugin = (NPAPIPlugin*)instance->pdata;
    string userAgent = plugin->getUserAgent();
    BPLOG_DEBUG_STRM("DropManagerFactory userAgent: '" << userAgent << "'");
    bp::BrowserInfo info(userAgent);
    BPLOG_INFO_STRM("platform/browser/version = " << info.asString());

    // Curent Html5 users:
    //  - Firefox 3.6.3 or greater on Windows
    //  - Safari 4.0.3 or greater on OSX
    // Everybody else uses Intercept.
    // Chrome on OSX doesn't support DnD.
    bool unsupported = false;
    bool useHtml5 = false;
    if (info.platform() == "Windows") {
        if (info.browser() == "Firefox") {
            bp::ServiceVersion baseVersion;
            (void) baseVersion.parse("3.6.3");
            useHtml5 = info.version().compare(baseVersion) >= 0;
        }
    } else if (info.platform() == "OSX") {
        if (info.browser() == "Safari") {
            bp::ServiceVersion baseVersion;
            (void) baseVersion.parse("4.0.3");
            useHtml5 = info.version().compare(baseVersion) >= 0;
        } else if (info.browser() == "Chrome") {
            unsupported = true;
        }
    }

    if (unsupported) {
        BPLOG_WARN_STRM("DnD unsupported for " << userAgent);
        return NULL;
    }

    BPLOG_INFO_STRM("using " << string(useHtml5 ? "html5" : "intercept")
                    << " drop manager for " << userAgent);
    return useHtml5 ? Html5DropManager::allocate(instance, window, listener,
                                                 info.platform(), info.browser())
                      : InterceptDropManager::allocate(instance, window, listener);
}


void
DropManagerFactory::destroy(IDropManager* mgr)
{
    delete mgr;
}

