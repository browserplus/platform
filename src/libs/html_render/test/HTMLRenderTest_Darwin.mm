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

#include "HTMLRenderTest.h"
#include "HTMLRender/HTMLRender.h"
#include <iostream>

#include <Cocoa/Cocoa.h>
#include <WebKit/WebKit.h>

@interface LoadWatcher : NSObject {
@public
    bp::html::ScriptableObject * so;
    NSString* soName;
}
- (void)                webView: (WebView *) sender
    windowScriptObjectAvailable: (WebScriptObject *)windowScriptObject;
@end;

@implementation LoadWatcher
- (void) dealloc
{
    [soName release];
    [super dealloc];
}

- (void)                webView: (WebView *) sender
    windowScriptObjectAvailable: (WebScriptObject *)windowScriptObject
{
    WebScriptObject * scriptable =
        (WebScriptObject *) so->scriptableObject((void *) windowScriptObject);
    [windowScriptObject setValue:scriptable forKey: soName];
}
@end

HTMLRenderTest::JavascriptRunner::JavascriptRunner() : m_osSpecific(NULL)
{
}
HTMLRenderTest::JavascriptRunner::~JavascriptRunner()
{
}

void 
HTMLRenderTest::JavascriptRunner::run(const boost::filesystem::path & path,
                                      bp::html::ScriptableObject & so,
                                      const std::string & soName)
{
    LoadWatcher * watcher = [[LoadWatcher alloc] init];
    watcher->so = &so;
    watcher->soName = [[[NSString alloc] initWithUTF8String: soName.c_str()] retain];    
    // allocate a webview, size don't matter
    NSRect frame = { {0,0}, {100,100} };

    WebView * webView = [[WebView alloc] initWithFrame: frame
                                         frameName: nil
                                         groupName: nil];

    // figure out when the load is complete
    [webView setFrameLoadDelegate: watcher];

    NSString * nsPath = [[NSString alloc] initWithUTF8String: path.c_str()];
    NSURL * url = [[NSURL alloc] initFileURLWithPath: nsPath];
    [[webView mainFrame] loadRequest: [NSURLRequest requestWithURL: url]];
}

