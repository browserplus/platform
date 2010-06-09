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

#import "PluginCommonLib/bppluginutil.h"
#import "PluginCommonLib/CommonErrors.h"
#import "PluginCommonLib/FileSavePluglet.h"
#import "BPUtils/bpfile.h"
#import "BPUtils/bplocalization.h"
#import "BPUtils/bpurl.h"
#import "BPUtils/BPLog.h"
#import <sstream>
#import <Cocoa/Cocoa.h>

using namespace bp::localization;
using namespace std;

void
FileSavePluglet::execute(unsigned int tid,
                         const char* function,
                         const bp::Object* arguments,
                         bool /* syncInvocation */, 
                         plugletExecutionSuccessCB successCB,
                         plugletExecutionFailureCB failureCB,
                         plugletInvokeCallbackCB   callbackCB,
                         void* callbackArgument)
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

    // Create our panel
    NSSavePanel* panel = [NSSavePanel savePanel];
    [panel setCanCreateDirectories: YES];

    // Run the panel and get the results
    bp::file::Path selection;
    NSString* fileStr = [NSString stringWithUTF8String: fileName.c_str()];
    if ([panel runModalForDirectory: nil
                               file: fileStr] == NSFileHandlingPanelOKButton) {
        NSURL* url = [panel URL];
        if ([url isFileURL]) {
            selection = [[url path] UTF8String];
        } else {
            BPLOG_WARN_STRM("ignoring non-file url " << [url absoluteString]);
            failureCB(callbackArgument, tid, "non-file url", NULL);
            return;
        }
    }
        
    // return results
    bp::Map* m = new bp::Map;
    m->add("file", new bp::Path(selection));
    successCB(callbackArgument, tid, m);
    delete m;
}
