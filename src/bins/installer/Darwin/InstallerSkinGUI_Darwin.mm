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

#include "../InstallerSkinGUI.h"

#include "BPUtils/BPUtils.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include <string>
#include <sstream>
#include <iostream>

/*
@interface MyWindow : NSWindow {
};
- (void) sendEvent: (NSEvent *) e;
@end;

@implementation MyWindow
- (void) sendEvent: (NSEvent *) e
{
    std::cout << "event intercept" << std::endl;
    [super sendEvent: e];
}
@end;
*/

@interface MyWebViewListener : NSObject
{
@public
    ScriptableInstallerObject * m_so;
}
- (void)                webView: (WebView *) sender
    windowScriptObjectAvailable: (WebScriptObject *)windowScriptObject;
- (void)  mainViewDidLoad;

- (void) webView: (WebView *)sender
         decidePolicyForNavigationAction:(NSDictionary *)actionInformation
         request:(NSURLRequest *)request
         frame:(WebFrame *)frame
         decisionListener:(id < WebPolicyDecisionListener >) listener;

// yuck!  undocumented!  used to capture javascript errors.
// http://lists.apple.com/archives/webkitsdk-dev/2006/Apr/msg00018.html
- (void)webView:(WebView *)webView addMessageToConsole:(NSDictionary *)d;

- (void) dealloc;
@end

@implementation MyWebViewListener

- (void)                webView: (WebView *) sender
    windowScriptObjectAvailable: (WebScriptObject *) wso
{
    BPLOG_INFO("windowScriptObjectAvailable");
    WebScriptObject * scriptable = (WebScriptObject *)
        m_so->getScriptableObject()->scriptableObject((void *) wso);
    [wso setValue:scriptable forKey: @"BPInstaller"];
}

- (void) mainViewDidLoad
{
    BPLOG_INFO("Installer UI rendered");
}

- (void) dealloc
{
    [super dealloc];
}

- (void)webView:(WebView *)webView addMessageToConsole:(NSDictionary *)d
{
    std::stringstream ss;

    ss << "Error from JavaScript Rendering engine -- " << std::endl;    

	id key;

    NSEnumerator *enumerator = [d keyEnumerator];
    
    while ((key = [enumerator nextObject])) {
        ss << "    " << [key UTF8String] << ": "
           << [[[d objectForKey:key] description] UTF8String];
    }

    BPLOG_ERROR(ss.str());
}

- (void) webView: (WebView *)sender
         decidePolicyForNavigationAction:(NSDictionary *)actionInformation
         request:(NSURLRequest *)request
         frame:(WebFrame *)frame
         decisionListener:(id < WebPolicyDecisionListener >) listener
{
    [[NSWorkspace sharedWorkspace] openURL: [request URL]];
    [listener ignore];
}

@end

InstallerSkinGUI::InstallerSkinGUI(const bp::file::Path & uiDirectory)
		: m_uiDirectory(uiDirectory), m_sio()
{
}

InstallerSkinGUI::~InstallerSkinGUI()
{
}

void
InstallerSkinGUI::startUp(unsigned int width, unsigned int height,
                          std::string title)
{
    [NSApplication sharedApplication];


    NSRect nr = { {0,0}, {width, height} };
    NSWindow * window = [[NSWindow alloc]
                            initWithContentRect: nr
                            styleMask: NSTitledWindowMask | NSClosableWindowMask
                            backing: NSBackingStoreBuffered
                            defer: NO];
    if (title.length()) {
      NSString * nsWindowTitle =
          [[NSString alloc] initWithUTF8String: title.c_str()];
      [window setTitle: nsWindowTitle];
    }

    [window setHasShadow: YES];

    // alloc a webview
    WebView * webView =
        [[WebView alloc] initWithFrame: [[window contentView] frame]
                         frameName: nil
                         groupName: nil];

    // load up index.html from the ui directory
    bp::file::Path pathToIndex = m_uiDirectory / "index.html";
    // now create an NSString
    NSString * nsPath =
        [[NSString alloc] initWithUTF8String: pathToIndex.external_file_string().c_str()];
    NSURL * url = [[NSURL alloc] initFileURLWithPath: nsPath];
    [[webView mainFrame] loadRequest: [NSURLRequest requestWithURL: url]];

    // disable scrolling
    [[[webView mainFrame] frameView]  setAllowsScrolling: NO];

    // save some resources
    [webView setMaintainsBackForwardList: NO];
    [[webView preferences] setPrivateBrowsingEnabled: YES];

    // add webview to window's main content view
    [[window contentView] addSubview: webView];
    [webView setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];


    // update the listener of our scriptable object so it can
    // callback into the skin listener
    m_sio.setListener(m_listener);

    // allocate and set the webview delegate
    MyWebViewListener * listener = [[MyWebViewListener alloc] init];
    listener->m_so = &m_sio;
    [webView setFrameLoadDelegate: listener];
    [webView setUIDelegate: listener];

    // spawn subsequent navigation actions in default browser
    [webView setPolicyDelegate: listener];

    [window display];
    [window orderFront: window];    
    [window makeMainWindow];    
    [window makeKeyWindow];    
    [window center];

    [[NSApplication sharedApplication] activateIgnoringOtherApps: YES];
    [[NSApplication sharedApplication] requestUserAttention:
                                           NSInformationalRequest];

    // TODO: this seems to be the only way to get everything working correctly
    // :/  but it's a little strange.
    [NSApp run];
}

void
InstallerSkinGUI::statusMessage(const std::string & s)
{
    m_sio.setStatus(s);
}

void
InstallerSkinGUI::errorMessage(const std::string & s)
{
    m_sio.setError(s, std::string());
}

void
InstallerSkinGUI::debugMessage(const std::string & s)
{
//    std::cout << "(TODO - get this in ui) debug: " << s << std::endl;
}

void
InstallerSkinGUI::allDone()
{
    // javascript will call into scriptable object which will invoke
    // shutdown(), ending the program
	// m_listener->shutdown();
}

void
InstallerSkinGUI::ended()
{
    // not all that effected for nested event processing :/
    // we rely on higher level code to shut the application down.
    [[NSApplication sharedApplication] stop: nil];
}
