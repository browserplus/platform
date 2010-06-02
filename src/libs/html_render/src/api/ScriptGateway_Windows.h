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
 *  ScriptGateway_Windows.h
 *
 *  Declares ScriptGateway, which enables communication in an html
 *  dialog between JavaScript and C++.
 *
 *  Created by David Grigsby on 9/25/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _SCRIPTGATEWAY_H_
#define _SCRIPTGATEWAY_H_

#include "ComUtils_Windows.h"
#include "IEUtils_Windows.h"
#include "HTMLDialog_Windows.h"
#include "HTMLScriptObject.h"


namespace bp {
namespace html {


class ScriptGateway : public HtmlDialog::EventListener
{
// Construction/Destruction    
public:
    ScriptGateway( const ScriptableObject& so,
                   const std::string& sScriptObjName );

    
// EventListener overrides
public:
    // src:     source dialog
    // sUrl:    proposed url, listener may change it
    // return:  true to allow the navigation
    //          false to cancel the navigation
    virtual bool onBeforeNavigate( HtmlDialog& src, std::string& sUrl );
    
    virtual void onDocumentComplete( HtmlDialog& src );

    
// Internal methods
private:
    // Adds a js object to the js window object.
    // "Invoker" methods will be added to this object to call into our
    // scriptable com object.
    bool addScriptableJsObj();

    // Adds a com object that exposes IDispatch to the js window object.
    // "Invoker" functions will call the invoke method on this object.
    // From there, c++ actions may be performed.
    bool addScriptableComObj();

    // Adds an "invoker" method of the specified name to our js
    // scriptable object.
    // The invoker method is called by client Js.  It packages its
    // arguments into a js array and then calls the invoke method of
    // our scriptable com object.
    bool addInvoker( const std::string& sMethodName );


// State
private:
    const ScriptableObject& m_so;
    std::string             m_sJsObjName;
    std::string             m_sComObjName;
    CComPtr<IWebBrowser2>   m_browser;
    CComPtr<IDispatch>      m_jsoBPDialog;
    bool                    m_bHaveNavigated;
};


} // namespace html
} // namespace bp


#endif
