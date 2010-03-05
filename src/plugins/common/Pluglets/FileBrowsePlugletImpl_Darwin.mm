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

#import "PluginCommonLib/bppluginutil.h"
#import "PluginCommonLib/CommonErrors.h"
#import "PluginCommonLib/FileBrowsePluglet.h"
#import "BPUtils/bpfile.h"
#import "BPUtils/bplocalization.h"
#import "BPUtils/bpurl.h"
#import "BPUtils/BPLog.h"
#import <sstream>
#import <Cocoa/Cocoa.h>

using namespace bp::localization;
using namespace std;

static string
getBrowseTitle(const char* key,
               const string &sUrl,
               const string& locale)
{
    string title;
    (void) getLocalizedString(key, locale, title);
    
    bp::url::Url url;
    if (url.parse(sUrl)) {
        title += "(";
        title += url.friendlyHostPortString();
        title += ")";
    }
    return title;
}

// When run from an out-of-process Safari plugin, NSOpenPanel's runModal
// always returns NSFileHandlingPanelCancelButton, even if OK was selected.
// Works fine in-process (Firefox and 32-bit Safari).  Thus, we subclass
// NSOpenPanel and override the ok/cancel methods.  Jeez.

@interface MyOpenPanel : NSOpenPanel {
    bool m_ok;
}
- (id) init;
- (IBAction) ok: (id)sender;
- (IBAction) cancel: (id)sender;
- (bool) okSelected;
@end

@implementation MyOpenPanel
- (id) init
{
    if ((self = [super init])) {
        m_ok = false;
    }
    return self;
}

- (IBAction) ok: (id)sender
{
    m_ok = true;
    return [super ok:sender];
}

- (IBAction) cancel: (id)sender 
{
    m_ok = false;
    return [super cancel:sender];
}

- (bool) okSelected 
{
    return m_ok;
}
@end


// A delegate to apply mimetype filtering for the MyOpenPanel used below
@interface MyDelegate : NSObject {
    set<string>* m_mimetypes;
}
- (id) init;
- (void) setMimetypes: (set<string>*) mimetypes;
- (BOOL) panel: (id)sender shouldShowFilename: (NSString*)filename;
@end

@implementation MyDelegate
- (id) init
{
    if ((self = [super init])) {
        m_mimetypes = 0;
    }
    return self;
}

- (void) setMimetypes: (set<string>*) mimetypes
{
    m_mimetypes = mimetypes;
}

- (BOOL) panel: (id)sender shouldShowFilename: (NSString*)filename
{
    string path([filename UTF8String]);
    if (boost::filesystem::is_directory(path)) {
        return YES;
    }
    return bp::file::isMimeType(path, *m_mimetypes) ? YES : NO;
}
@end


void
FileBrowsePluglet::execute(unsigned int tid,
                           const char * function,
                           const bp::Object * arguments,
                           bool /* syncInvocation */, 
                           plugletExecutionSuccessCB successCB,
                           plugletExecutionFailureCB failureCB,
                           plugletInvokeCallbackCB   callbackCB,
                           void * callbackArgument)
{
    if (!function || !arguments) {
        BPLOG_WARN_STRM("execute called will NULL function or arguments");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
        return;
    }

    if (strcmp(function, "OpenBrowseDialog")) {
        std::string s("unknown FileBrowse function " 
                      + std::string(function) + " called");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters,
                  s.c_str());
        return;
    }

    bool recurse = true;
    std::set<std::string> mimetypes;
    bool includeGestureInfo = false;
    unsigned int limit = 10000;
    if (m_desc.majorVersion() == 1) {
        // Dig out args
        if (arguments->has("recurse", BPTBoolean)) {
          recurse = ((bp::Bool*) arguments->get("recurse"))->value();
        }
    
        if (arguments->has("mimeTypes", BPTList)) {
            const bp::List* l = (const bp::List*) arguments->get("mimeTypes");
            for (unsigned int i = 0; i < l->size(); i++) {
                const bp::String* s = dynamic_cast<const bp::String*>(l->value(i));
                if (s) {
                    mimetypes.insert(s->value());
                }
            }
        } 
    
        if (arguments->has("includeGestureInfo", BPTBoolean)) {
            includeGestureInfo = ((bp::Bool*) arguments->get("includeGestureInfo"))->value();
        }
    
        if (arguments->has("limit", BPTInteger)) {
            limit = ((bp::Integer*) arguments->get("limit"))->value();
        }
    }

    // extract current Url for window title
    std::string currentUrl;
    if (m_plugin) (void) m_plugin->getCurrentURL(currentUrl);

    // Create our panel
    MyDelegate* delegate = [[MyDelegate alloc] init];
    [delegate setMimetypes: &mimetypes];
    MyOpenPanel* panel = [[MyOpenPanel alloc] init];
    [panel setDelegate:delegate];
    [panel setAllowsMultipleSelection:YES];
    [panel setCanChooseFiles:YES];
    string title = getBrowseTitle(FileBrowsePluglet::kSelectFilesFoldersKey,                            
                                  currentUrl, m_locale);
    [panel setTitle: [NSString stringWithUTF8String: title.c_str()]];
    if (m_desc.majorVersion() > 1) {
        [panel setResolvesAliases: NO];
    }
    
    // Can folders be selected?
    if (recurse) {
        [panel setCanChooseDirectories:YES];
    } else {
        if (mimetypes.count(bp::file::kFolderMimeType) == 1) {
            [panel setCanChooseDirectories:YES];
            if (mimetypes.size() == 1) {
                [panel setCanChooseFiles:NO];
                title = getBrowseTitle(FileBrowsePluglet::kSelectFolderKey,                            
                                       currentUrl, m_locale);
            }
        }
    }

    // Run the panel and get the results
    std::vector<bp::file::Path> selection;
    (void) [panel runModal];
    if ([panel okSelected]) {
        NSArray* urls = [panel URLs];
        int count = [urls count];
        for (int i = 0; i < count; i++) {
            NSURL* url = [urls objectAtIndex:i];
            if (![url isFileURL]) {
                BPLOG_WARN_STRM("ignoring non-file url " << [url absoluteString]);
                continue;
            }
            selection.push_back([[url path] UTF8String]);
        }
    }
    [panel setDelegate:nil];
    [delegate release];
    [panel release];

    bp::Object* obj = NULL;
    if (m_desc.majorVersion() == 1) {
        // version 1 api applies filtering, recursion, etc...
        unsigned int flags = 0;
        if (recurse) flags |= bp::pluginutil::kRecurse;
        if (includeGestureInfo) flags |= bp::pluginutil::kIncludeGestureInfo;
        obj = bp::pluginutil::applyFilters(selection, mimetypes, flags, limit);
    } else {
        // version 2 and above api just gives you what was selected
        bp::Map* m = new bp::Map;
        bp::List* l = new bp::List;
        vector<bp::file::Path>::const_iterator it;
        for (it = selection.begin(); it != selection.end(); ++it) {
            bp::file::Path path(*it);
            l->append(new bp::Path(path));
        }
        m->add("files", l);
        obj = m;
    }
        
    // return results
    successCB(callbackArgument, tid, obj);
    delete obj;
}
