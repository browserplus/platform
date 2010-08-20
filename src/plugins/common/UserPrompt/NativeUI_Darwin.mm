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

#include "NativeUI.h"
#include "BPUtils/BPUtils.h"
#include "HTMLRender/HTMLRender.h"

#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>
#include <WebKit/HIWebView.h>


@interface HTMLDiagWebViewListener : NSObject {
@public
    // we want to display the dialog AFTER the frame loads for a better
    // visual effect.  This bit of state lets us display the dialog
    // in either show() or didFinishLoadForFrame() whichever comes
    // last
    bool showCalledOrFrameLoaded;

    bool modalComplete;

    // If parentWindow is non-nil, we'll use a modal sheet attached to it.
    // If nil, we just use an app modal window
    NSWindow * parentWindow;

    NSWindow * cocoaWRef;
    WebView * wv;
    NSModalSession session;
    
    // the scriptable interface
    class DialogScriptableObject * scriptObj;

    bp::Object * response;
};

- (void) webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame;
- (void) webView:(WebView *)sender
         windowScriptObjectAvailable: (WebScriptObject *)windowScriptObject;
- (void) webView: (WebView *)sender
         decidePolicyForNavigationAction:(NSDictionary *)actionInformation
         request:(NSURLRequest *)request
         frame:(WebFrame *)frame
         decisionListener:(id < WebPolicyDecisionListener >) listener;
- (void) dealloc;
- (void) webView:(WebView *)webView addMessageToConsole:(NSDictionary *)d;
- (void) startDialog;

// functions called by scriptObj in response to javascript calls
- (void) complete: (const bp::Object *) obj;
- (void) show: (short) w height: (short) h;

@end;

//////////////////////////////////////////////////////////////////////
// begin DialogScriptableObject
// A class which manages exposing a scriptable interface into javascript
class DialogScriptableObject : public bp::html::ScriptableFunctionHost
{
  public:
    DialogScriptableObject(HTMLDiagWebViewListener * listener,
                           const bp::Object * args);
    ~DialogScriptableObject();    
    bp::html::ScriptableObject * getScriptableObject();
  private:
    // dispatch function for when javascript calls into us
    virtual bp::Object * invoke(const std::string & functionName,
                                unsigned int id,
                                std::vector<const bp::Object *> args); 

    bp::html::ScriptableObject m_so;
    HTMLDiagWebViewListener * m_listener;
    bp::Object * m_args;
};


DialogScriptableObject::DialogScriptableObject(
    HTMLDiagWebViewListener * listener, const bp::Object * args)
{
    // let's expose some functions
    m_so.mountFunction(this, "args");
    m_so.mountFunction(this, "dpiHack");
    m_so.mountFunction(this, "complete");
    m_so.mountFunction(this, "log");    
    m_so.mountFunction(this, "show"); 

    m_listener = listener;
    m_args = args ? args->clone() : new bp::Null;
}


DialogScriptableObject::~DialogScriptableObject()
{
    if (m_args) delete m_args;
}


bp::html::ScriptableObject *
DialogScriptableObject::getScriptableObject()
{
    return &m_so;
}


// dispatch function for when javascript calls into us
bp::Object *
DialogScriptableObject::invoke(const std::string & functionName,
                               unsigned int id,
                               std::vector<const bp::Object *> args)
{
    if (!functionName.compare("log"))
    {
        for (unsigned int i = 0; i < args.size(); i++) {
            std::string s;
            if (args[i]->type() == BPTString) {
                s = std::string(*(args[i]));
            } else {
                s = args[i]->toPlainJsonString(true);
            }

            BPLOG_INFO_STRM("JavaScript Logging: " << s);
        }
    }
    else if (!functionName.compare("complete"))    
    {
        if (args.size() != 1) {
            throw std::string("complete function takes exactly one arguments");
        }
        if (m_listener) [m_listener complete: args[0]];
    }
    else if (!functionName.compare("args"))    
    {
        if (args.size() != 0) {
            throw std::string("args function takes no arguments");
        }
        return m_args->clone();
    }
    else if (functionName == "dpiHack")
    {
        return new bp::Bool(false);
    }
    else if (!functionName.compare("show"))        
    {
        if (args.size() != 2 ||
            args[0]->type() != BPTInteger ||
            args[1]->type() != BPTInteger)
        {
            throw std::string("invalid arguments");
        }
        long long w = (long long) *(args[0]);
        long long h = (long long) *(args[1]);

        if (m_listener) [m_listener show: w height: h];
    }

    return NULL;
}


// end DialogScriptableObject
//////////////////////////////////////////////////////////////////////

@implementation HTMLDiagWebViewListener
- (void)dealloc
{
    BPLOG_INFO("dealloc HTMLDiagWebViewListener");

    // no no  it's released on close
    // [cocoaWRef release];
    
    if (scriptObj) delete scriptObj;

    [super dealloc];
}


- (void)webView:(WebView *)webView addMessageToConsole:(NSDictionary *)d;
{
    std::stringstream ss;
    ss << "Warning from JavaScript Rendering engine -- " << std::endl;    

    id key;
    NSEnumerator *enumerator = [d keyEnumerator];
    while ((key = [enumerator nextObject])) {
        ss << "    " << [key UTF8String] << ": "
           << [[[d objectForKey:key] description] UTF8String];
    }

    BPLOG_ERROR(ss.str());   
}


- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    if (showCalledOrFrameLoaded)
    {
        BPLOG_DEBUG("call startDialog");
        [self startDialog];
    }
    else 
    {
        showCalledOrFrameLoaded = true;
    }
}


- (void)webView:(WebView *)sender
        windowScriptObjectAvailable: (WebScriptObject *)wso
{
    WebScriptObject * scriptable = (WebScriptObject *)
        scriptObj->getScriptableObject()->scriptableObject((void *) wso);

    [scriptable retain];
    [wso setValue:scriptable forKey: @"BPDialog"];
}


// invoked by DialogScriptableObject in response to javascript call
- (void) complete: (const bp::Object *) obj
{
    // now pass return value
    if (obj) response = obj->clone();

    [self->wv setPolicyDelegate: nil];
    [self->wv setFrameLoadDelegate: nil];
    [self->wv setUIDelegate: nil];
    [self->wv setHostWindow: nil];

    if (parentWindow) 
    {
        BPLOG_DEBUG("endModalSession, endSheet");
        [NSApp endModalSession: session];
        [NSApp endSheet: cocoaWRef];
        [cocoaWRef close];
    }
    else 
    {
        BPLOG_DEBUG("stopModal");
        [NSApp stopModal];
    }

    modalComplete = true;

    BPLOG_DEBUG_STRM("BPDialog.complete() called by javascript");
}


- (void) startDialog
{
    // run the window modally
    if (parentWindow) 
    {
        BPLOG_DEBUG_STRM("begin sheet for Window " << cocoaWRef);
        [NSApp beginSheet: cocoaWRef modalForWindow: parentWindow
            modalDelegate: nil didEndSelector: nil contextInfo: nil];
        session = [NSApp beginModalSessionForWindow: cocoaWRef];
    } 
    else
    {
        BPLOG_DEBUG_STRM("run app modal for Window " << cocoaWRef);
        [NSApp runModalForWindow:cocoaWRef];
    }
}


- (void) show: (short) w height: (short) h
{
    NSSize size = {w,h};
    [self->cocoaWRef setContentSize: size];

    if (showCalledOrFrameLoaded)
    {
        BPLOG_DEBUG("call startDialog");
        [self startDialog];
    } 
    else
    {
        showCalledOrFrameLoaded = true;
    }
}


- (void) webView: (WebView *)sender
         decidePolicyForNavigationAction:(NSDictionary *)actionInformation
         request:(NSURLRequest *)request
         frame:(WebFrame *)frame
         decisionListener:(id < WebPolicyDecisionListener >) listener
{
    [[NSWorkspace sharedWorkspace] openURL: [request URL]];
    [listener ignore];
    [self complete: NULL];
}
@end


bool
bp::ui::HTMLPrompt(void * parentWindow,
                   const std::string & pathToHTMLDialog,
                   const std::string & userAgent,
                   const bp::Object * arguments,
                   bp::Object ** oResponse)
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    // allocate our listener class
    HTMLDiagWebViewListener * mso = [[HTMLDiagWebViewListener alloc] init];
//    [mso autorelease]; don't try this!  certain environments
//    release the pool prematurely.
    mso->modalComplete = false;
    mso->showCalledOrFrameLoaded = false;
    mso->cocoaWRef = nil;
    mso->wv = nil;
    mso->session = nil;
    mso->response = NULL;
    // Don't use sheets on Safari, we don't have a useful
    // window in the out-of-proc case
    unsigned int style = 0;
    bp::BrowserInfo info(userAgent);
    if (info.browser() == "Safari")
    {
        mso->parentWindow = nil;
        style = NSTitledWindowMask;
    }
    else
    {
        mso->parentWindow = [[NSWindow alloc] initWithWindowRef: parentWindow];
        style = NSBorderlessWindowMask;
    }
    BPLOG_DEBUG_STRM("userAgent = '" << userAgent 
                     << "', parentWindow = " << mso->parentWindow);
    
    NSRect frame = { {0,0}, {300, 600} };
    mso->cocoaWRef = [[NSWindow alloc] 
                        initWithContentRect: frame
                                  styleMask: style
                                    backing: NSBackingStoreBuffered
                                      defer: YES ];
    [mso->cocoaWRef setHasShadow: YES];
    [mso->cocoaWRef setOneShot: YES];
    [mso->cocoaWRef setReleasedWhenClosed: YES];
    
    // allocate a webview
    mso->wv = [[WebView alloc] initWithFrame: frame
                                   frameName: nil
                                   groupName: nil];
    [mso->wv setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];

    NSView * windowRoot = [mso->cocoaWRef contentView];
    [windowRoot addSubview: mso->wv];
    [mso->wv release]; // ownership transfered
    [windowRoot setFrame: frame];
    
    // disable scrolling
    [[[mso->wv mainFrame] frameView]  setAllowsScrolling: NO];

    // figure out when the load is complete
    [mso->wv setFrameLoadDelegate: mso];

    // save some resources by not maintaining a back/forward list
    [mso->wv setMaintainsBackForwardList: NO];
    [[mso->wv preferences] setPrivateBrowsingEnabled: YES];

    // let's load up the HTML
    NSString * nsPath = 
		[NSString stringWithUTF8String: pathToHTMLDialog.c_str()];

    [[mso->wv mainFrame] loadRequest:[NSURLRequest
                    requestWithURL:[NSURL fileURLWithPath: nsPath]]];

    // spawn subsequent navigation actions in default browser
    [mso->wv setPolicyDelegate: mso];

    // hear about javascript errors
    [mso->wv setUIDelegate: mso];

    // allocate a scriptable object
    mso->scriptObj = new DialogScriptableObject(mso, arguments);

    BPLOG_INFO("Running User Prompt");

    if (mso->parentWindow)
    {
        while (((mso->session == nil) ||
                ([NSApp runModalSession: mso->session] == NSRunContinuesResponse))
               && !mso->modalComplete)
        {
            [[NSRunLoop currentRunLoop]
             runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.01]];
        }
        [mso->parentWindow release];
        BPLOG_DEBUG_STRM("Finished Cocoa Outer runloop / "
                         << "mso->session == " << mso->session << " / "
                         << "mso->modalComplete == " << mso->modalComplete);
    } 
    else
    {
        while (!mso->modalComplete) 
        {
            [[NSRunLoop currentRunLoop]
             runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.01]];
        }
        BPLOG_DEBUG_STRM("Finished Cocoa Outer runloop / "
                         << "mso->modalComplete == " << mso->modalComplete);
        [mso->cocoaWRef close];
    } 

    *oResponse = mso->response;
    if (*oResponse == NULL) *oResponse = new bp::Null;

    [mso release];
    [pool release];

    BPLOG_INFO_STRM("Ran User Prompt, got: " <<
                    (*oResponse)->toPlainJsonString(true));
    
    return true;
}
