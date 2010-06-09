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
 *  FileBrowsePlugletImpl_Windows.cpp
 *
 *  Created by David Grigsby on 11/20/2007.
 *  Modified to support multiple selects
 *  and mimetype filtering by Gordon Durand on 07/10/08.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */
#include "BPUtils/BPLog.h"
#include "PluginCommonLib/bppluginutil.h"
#include "PluginCommonLib/CommonErrors.h"

#include "FileSavePluglet.h"

#include <vector>
#include <string>

using namespace std;

void
FileSavePluglet::execute(unsigned int tid,
                         const char* function,
                         const bp::Object* arguments,
                         bool /* syncInvocation */, 
                         plugletExecutionSuccessCB successCB,
                         plugletExecutionFailureCB failureCB,
                         plugletInvokeCallbackCB   /*callbackCB*/,
                         void* callbackArgument )
{
    if (!function) {
        BPLOG_WARN_STRM("execute called will NULL function");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
        return;
    }

    if (strcmp(function, "OpenSaveDialog")) {
        std::string s("unknown FileSave function " 
                      + std::string(function) + " called");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters,
                  s.c_str());
        return;
    }

    // dig out arguments
    string fileName;
    if (arguments->has("name", BPTString)) {
        fileName = ((bp::String*) arguments->get("name"))->value();
    }

    // XXX unimplemented
    failureCB(callbackArgument, tid, "Unimplemented", NULL);
    return;
}

