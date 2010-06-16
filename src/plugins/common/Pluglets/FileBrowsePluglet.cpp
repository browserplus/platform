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

#include "FileBrowsePluglet.h"
#include "BPUtils/BPLog.h"
#include "PluginCommonLib/CommonErrors.h"


const char* FileBrowsePluglet::kSelectKey = "FileBrowsePluglet::kSelectKey";
const char* FileBrowsePluglet::kFileFolderNameKey = "FileBrowsePluglet::kFileFolderNameKey";
const char* FileBrowsePluglet::kFileNameKey = "FileBrowsePluglet::kFileNameKey";
const char* FileBrowsePluglet::kAllFilesFoldersKey = "FileBrowsePluglet::kAllFilesFoldersKey";
const char* FileBrowsePluglet::kSelectFilesFoldersKey = "FileBrowsePluglet::kSelectFilesFoldersKey";
const char* FileBrowsePluglet::kSelectFilesKey = "FileBrowsePluglet::kSelectFilesKey";
const char* FileBrowsePluglet::kSelectFolderKey = "FileBrowsePluglet::kSelectFolderKey";

FileBrowsePluglet::FileBrowsePluglet(BPPlugin * plugin,
                                     const bp::service::Description & desc)
    : Pluglet(plugin, desc)
{
}


FileBrowsePluglet::~FileBrowsePluglet()
{
}


void
FileBrowsePluglet::execute(unsigned int tid,
                           const char* function,
                           const bp::Object* arguments,
                           bool /* syncInvocation */, 
                           plugletExecutionSuccessCB successCB,
                           plugletExecutionFailureCB failureCB,
                           plugletInvokeCallbackCB   callbackCB,
                           void* callbackArgument)
{
    // validate function
    if (!function) {
        BPLOG_WARN_STRM("execute called will NULL function");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
        return;
    }
    if (m_desc.majorVersion() == 1) {
        if (strcmp(function, "OpenBrowseDialog")) {
            std::string s("unknown FileBrowse function " 
                          + std::string(function) + " called");
            failureCB(callbackArgument, tid, pluginerrors::InvalidParameters,
                      s.c_str());
            return;
        }
        v1Browse(tid, arguments, successCB, failureCB, callbackArgument);
    } else if (m_desc.majorVersion() == 2) {
        if (strcmp(function, "OpenBrowseDialog")) {
            std::string s("unknown FileBrowse function " 
                          + std::string(function) + " called");
            failureCB(callbackArgument, tid, pluginerrors::InvalidParameters,
                      s.c_str());
            return;
        }
        browse(tid, successCB, failureCB, callbackArgument);
    } else if (m_desc.majorVersion() >= 3) {
        if (!strcmp(function, "selectFiles")) {
            browse(tid, successCB, failureCB, callbackArgument);
        } else if (!strcmp(function, "saveAs")) {
            save(tid, arguments, successCB, failureCB, callbackArgument);
        } else {
            std::string s("unknown FileBrowse function " 
                          + std::string(function) + " called");
            failureCB(callbackArgument, tid, pluginerrors::InvalidParameters,
                      s.c_str());
            return;
        }
    }
}

