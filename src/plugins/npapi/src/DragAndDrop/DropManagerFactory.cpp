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
#include "BPUtils/BPLog.h"
#include "DropManagerFactory.h"
#include "Html5DropManager.h"
#include "InterceptDropManager.h"
#include "NPAPIPlugin.h"
#include "platform_utils/bpbrowserinfo.h"

using namespace std;
using namespace bp;

IDropManager* 
DropManagerFactory::create(NPP instance,
                           NPWindow* window,
                           IDropListener* listener)
{
    IDropManager* rval = NULL;
    NPAPIPlugin* plugin = (NPAPIPlugin*)instance->pdata;
    BrowserInfo info = plugin->getBrowserInfo();
    string dndType = info.capability(BrowserInfo::kDnDCapability);
    BPLOG_INFO_STRM("platform/browser/version = " << info.asString()
                    << ", dndType = " << dndType);
    if (dndType == BrowserInfo::kDnDIntercept) {
        rval = InterceptDropManager::allocate(instance, window, listener);
    } else if (dndType == BrowserInfo::kDnDHtml5) {
        rval = Html5DropManager::allocate(instance, window,
                                          listener,
                                          info.platform(),
                                          info.browser());
    } else if (dndType == BrowserInfo::kUnsupported) {
        // empty, logging done above
    } else {
        // shouldn't happen
        BPLOG_ERROR_STRM("unknown DnD capability: " << dndType);
    }
    return rval;
}


void
DropManagerFactory::destroy(IDropManager* mgr)
{
    delete mgr;
}

