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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

//
//  second_tryPref.h
//  second_try
//
//  Created by Lloyd Hilaiel on 7/14/08.
//  Copyright (c) 2008 __MyCompanyName__. All rights reserved.
//

#import <PreferencePanes/PreferencePanes.h>

#include "ScriptableConfigObject.h"
#import <WebKit/WebKit.h>

@interface BPPrefPane : NSPreferencePane 
{
    ScriptableConfigObject * m_so;
    WebView * m_webView;
}
- (void)                webView: (WebView *) sender
    windowScriptObjectAvailable: (WebScriptObject *)windowScriptObject;
- (void)  mainViewDidLoad;

- (void) webView: (WebView *) sender
didFailProvisionalLoadWithError: (NSError *) error
        forFrame: (WebFrame *) frame;

- (void)                    webView: (WebView *) sender
               didFailLoadWithError: (NSError *) error 
                           forFrame: (WebFrame *) frame;
- (void)webView: (WebView *) sender didFinishLoadForFrame:(WebFrame *)frame;
- (void)webView:(WebView *)sender setStatusText:(NSString *)text;

// yuck!  undocumented:
// http://lists.apple.com/archives/webkitsdk-dev/2006/Apr/msg00018.html
- (void)webView:(WebView *)webView addMessageToConsole:(NSDictionary *)d;

- (NSView *) loadMainView;
- (void) dealloc;

@end
