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
#import "PluginCommonLib/FileBrowsePluglet.h"
#import "BPUtils/bpfile.h"
#import "BPUtils/bplocalization.h"
#import "BPUtils/bpurl.h"
#import "BPUtils/BPLog.h"
#import "BPUtils/OS.h"
#import <sstream>
#import <Cocoa/Cocoa.h>

using namespace bp::localization;
using namespace std;

// Ugh, due to a modality bug in 64 bit safari 5 on snow leopard,
// we must play some ugly games to get modality to work.  Hence,
// we need two little subclasses just to keep track of whether
// the "OK" button was pressed on a browse/save dialog.
//
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

@interface MySavePanel : NSSavePanel {
    bool m_ok;
}
- (id) init;
- (IBAction) ok: (id)sender;
- (IBAction) cancel: (id)sender;
- (bool) okSelected;
@end

@implementation MySavePanel
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
    if (bp::file::isDirectory(path)) {
        return YES;
    }
    return bp::file::isMimeType(path, *m_mimetypes) ? YES : NO;
}
@end


static bool 
isSafari5OnSnowLeopard(const string& userAgent)
{
    bp::ServiceVersion osVersion, leastOSVersion;
    leastOSVersion.parse("10.6.0");
    osVersion.parse(bp::os::PlatformVersion());
    
    if (osVersion.compare(leastOSVersion) >= 0) {
        if (userAgent.find("Safari") != string::npos)
        {
            bp::ServiceVersion baseVersion;
            baseVersion.parse("5.0.0");

            std::string vstr = "Version/";
            size_t start = userAgent.find(vstr);
            if (start == string::npos) return false;
            size_t end = userAgent.find(" ", start);
            if (end == string::npos) return false;
            string s = userAgent.substr(start, end - start);
            vector<string> v = bp::strutil::split(s, "/");
            if (v.size() < 2) return false;
        
            bp::ServiceVersion thisVersion;
            // get around a bug in the serviceversion class where 5.0 would be less than
            // 5.0.0
            thisVersion.setMajor(0);
            thisVersion.setMinor(0);
            thisVersion.setMicro(0);
            
            if (thisVersion.parse(v[1])) {
                if (thisVersion.compare(baseVersion) >= 0) {
                    return true;
                }
            }
        
        }
    }

    return false;
}


static struct ProcessSerialNumber 
getPSN()
{
    struct ProcessSerialNumber psn;
    NSDictionary * dict = [[NSWorkspace sharedWorkspace] activeApplication];
    psn.highLongOfPSN = [[dict objectForKey: @"NSApplicationProcessSerialNumberHigh"] unsignedLongValue];
    psn.lowLongOfPSN = [[dict objectForKey: @"NSApplicationProcessSerialNumberLow"] unsignedLongValue];
    return psn;
}


static vector<bp::file::Path>
runPanel(NSSavePanel* panel,
         NSString* file,       // non-nil implies save panel
         const string& userAgent)
{
    MyOpenPanel* openPanel = file ? nil : (MyOpenPanel*)panel;
    MySavePanel* savePanel = file ? (MySavePanel*)panel : nil;
    std::vector<bp::file::Path> selection;
    if (isSafari5OnSnowLeopard(userAgent)) {
        BPLOG_DEBUG("Using 10.6+ Safari 5+ modality workaround for file browse");
        
        // let's get the PSN of the current active application for later re-activation
        struct ProcessSerialNumber psn = getPSN();

        [panel _loadPreviousModeAndLayout];
        [panel setDirectoryURL: nil];
        if (file) [panel setNameFieldStringValue: file];

        [NSApp activateIgnoringOtherApps: YES];
        NSModalSession session = [NSApp beginModalSessionForWindow:panel];
        for (;;) {
            if ([NSApp runModalSession:session] != NSRunContinuesResponse) break;
        }
        [NSApp endModalSession:session];

        // now re-activate whoever was active before us
        SetFrontProcess(&psn);
    } else {
        BPLOG_DEBUG("Using runModal[ForDirectory] for panel display");
        if (openPanel) {
            [openPanel runModal];
        } else {
            [savePanel runModalForDirectory: nil file: file];
        }
    }

    bool ok = openPanel ? [openPanel okSelected] : [savePanel okSelected];
    if (ok) {
        if (savePanel) {
            NSURL* url = [savePanel URL];
            if ([url isFileURL]) {
                selection.push_back([[url path] UTF8String]);
            } else {
                BPLOG_WARN_STRM("ignoring non-file url " << [url absoluteString]);
            }
        } else {
            NSArray* urls = [openPanel URLs];
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
    }
    return selection;
}


static string
getBrowseTitle(const char* key,
               const string &sUrl,
               const string& locale)
{
    string title;
    (void) getLocalizedString(key, locale, title);
    
    bp::url::Url url;
    if (url.parse(sUrl)) {
        title += " (";
        title += url.friendlyHostPortString();
        title += ")";
    }
    return title;
}


void
FileBrowsePluglet::v1Browse(unsigned int tid,
                            const bp::Object* arguments,
                            plugletExecutionSuccessCB successCB,
                            plugletExecutionFailureCB failureCB,
                            void* callbackArgument)
{
    // Dig out args
    bool recurse = true;
    std::set<std::string> mimetypes;
    bool includeGestureInfo = false;
    unsigned int limit = 10000;
    if (!arguments) {
        BPLOG_WARN_STRM("execute called will NULL arguments");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
        return;
    }
    if (arguments->has("recurse", BPTBoolean)) {
        recurse = ((bp::Bool*) arguments->get("recurse"))->value();
    }
    
    if (arguments->has("mimeTypes", BPTList)) {
        const bp::List* l = (const bp::List*) arguments->get("mimeTypes");
        for (unsigned int i = 0; l && (i < l->size()); i++) {
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
    [panel setTitle: [NSString stringWithUTF8String: title.c_str()]];

    // Run the panel and get the results
    vector<bp::file::Path> selection = runPanel(panel, NULL, m_plugin->getUserAgent());
    [panel setDelegate:nil];
    [panel orderOut: nil];
    [panel release];
    [delegate release];

    bp::Object* obj = NULL;
    unsigned int flags = 0;
    if (recurse) flags |= bp::pluginutil::kRecurse;
    if (includeGestureInfo) flags |= bp::pluginutil::kIncludeGestureInfo;
    obj = bp::pluginutil::applyFilters(selection, mimetypes, flags, limit);
        
    // return results
    successCB(callbackArgument, tid, obj);
    delete obj;
}


void
FileBrowsePluglet::browse(unsigned int tid,
                          plugletExecutionSuccessCB successCB,
                          plugletExecutionFailureCB failureCB,
                          void* callbackArgument)
{
    // extract current Url for window title
    std::string currentUrl;
    if (m_plugin) (void) m_plugin->getCurrentURL(currentUrl);

    // Create our panel
    MyOpenPanel* panel = [[MyOpenPanel alloc] init];
    [panel setAllowsMultipleSelection:YES];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:YES];
    string title = getBrowseTitle(FileBrowsePluglet::kSelectFilesFoldersKey,
                                  currentUrl, m_locale);
    [panel setTitle: [NSString stringWithUTF8String: title.c_str()]];

    // Run the panel and get the results
    vector<bp::file::Path> selection = runPanel(panel, NULL, m_plugin->getUserAgent());
    [panel orderOut: nil];
    [panel release];

    // return results
    bp::Map* m = new bp::Map;
    bp::List* l = new bp::List;
    vector<bp::file::Path>::const_iterator it;
    for (it = selection.begin(); it != selection.end(); ++it) {
        bp::file::Path path(*it);
        l->append(new bp::Path(path));
    }
    m->add("files", l);
    successCB(callbackArgument, tid, m);
    delete m;
}


void
FileBrowsePluglet::save(unsigned int tid,
                        const bp::Object* arguments,
                        plugletExecutionSuccessCB successCB,
                        plugletExecutionFailureCB failureCB,
                        void* callbackArgument)
{
    // dig out arguments
    string fileName;
    if (arguments && arguments->has("name", BPTString)) {
        fileName = ((bp::String*) arguments->get("name"))->value();
    }

    // Create our panel
    MySavePanel* panel = [[MySavePanel alloc] init];
    [panel setCanCreateDirectories: YES];
    string title([[panel title] UTF8String]);
    std::string currentUrl;
    if (m_plugin) (void) m_plugin->getCurrentURL(currentUrl);
    bp::url::Url url;
    if (url.parse(currentUrl)) {
        title += " (";
        title += url.friendlyHostPortString();
        title += ")";
    }
    [panel setTitle: [NSString stringWithUTF8String: title.c_str()]];

    // Run the panel and get the results
    vector<bp::file::Path> selection = runPanel(panel,
                                                [NSString stringWithUTF8String: fileName.c_str()],
                                                m_plugin->getUserAgent());
    [panel orderOut: nil];
    [panel release];
        
    // return results
    if (selection.size() > 0) {
        bp::Object* obj = new bp::WritablePath(selection[0]);
        successCB(callbackArgument, tid, obj);
        delete obj;
    } else {
        failureCB(callbackArgument, tid, "FileBrowse.userCanceled",
                  "user canceled browse");
    }
}
