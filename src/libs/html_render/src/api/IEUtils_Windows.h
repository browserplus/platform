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

/*
 *  IEUtils_Windows.h
 *
 *  Internet Explorer utilities.
 *
 *  Created by David Grigsby on 10/10/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _IEUTILS_WINDOWS_H_
#define _IEUTILS_WINDOWS_H_

#include <atlbase.h>
#include <dispex.h>
#include <ExDisp.h>
#include <MsHTML.h>
#include <string>
#include <vector>


namespace bp {
namespace ie {


// Create an empty js Object.
bool createJsObject( IWebBrowser2* pBrowser, CComPtr<IDispatchEx>& dispObj );

// Get the names of the object's properties.
bool enumerateProperties( CComPtr<IDispatch> dispObj,
                          std::vector<std::string>& vsPropNames,
                          bool bOwnPropsOnly=true );

// Evaluate a json string to get a jscript object.
bool evaluateJson( IWebBrowser2* pBrowser, const std::string& sJson,
                   CComVariant& vtRet );

// Evaluate script using the js eval() method.
bool evaluateScript( IWebBrowser2* pBrowser, const std::string& sScript,
                     CComVariant& vtRet );

// Execute script using ms execScript().  Note no return value available.
bool executeScript( IWebBrowser2* pBrowser, const std::string& sScript );

// Get the specified array element of the object.
bool getArrayElement( CComPtr<IDispatch> dispObj,
                      int nIdx,
                      CComVariant& vtVal );

// Get the length of the specified js array.
bool getArrayLength( CComPtr<IDispatch> dispObj, int& nSize );

// Get body size in pixels.
bool getContentSize( IWebBrowser2* pBrowser, int& nWidth, int& nHeight );


// Get the document element with the specified name.
bool getDocElementById( IWebBrowser2* pBrowser, const std::string& sId,
                        CComPtr<IHTMLElement>& elem );

// Get the window for the current doc.
bool getDocWindow( IWebBrowser2* pBrowser, CComPtr<IHTMLWindow2>& wnd );

// Get the IHTMLDocument2 interface to current doc.
bool getHtmlDoc2( IWebBrowser2* pBrowser, CComPtr<IHTMLDocument2>& doc2 );

// Get the global scripts collection.
bool getJsScripts( IWebBrowser2* pBrowser, CComPtr<IDispatch>& dispScripts );

// Get the specified property of the object.
bool getObjectProperty( CComPtr<IDispatch> dispObj,
                        const std::string& sPropName,
                        CComVariant& vtVal );

// Get the id of event source element.
bool getSourceId( IHTMLEventObj* pEvtObj, std::string& sSourceId );

// Get the user agent of the specified browser.
bool getUserAgent( IWebBrowser2* pBrowser, std::string& sUserAgent );

// Get whether the object has an own property of the specified name.
bool hasOwnProperty( CComPtr<IDispatch> dispObj, const std::string& sPropName,
                     bool& bHasProp );

// Invoke the specified global js method that takes one arg.
bool invokeGlobalJsFunction( IWebBrowser2* pBrowser,
                             const std::string& sFuncName, CComVariant vArg,
                             CComVariant& vtRet );

// Invoke the provided function with one argument.
bool invokeJsFunction( CComPtr<IDispatch> func, CComVariant vtArg,
                       CComVariant& vtRet );

// Invoke the provided function passing a vector of args.
bool invokeJsFunction( CComPtr<IDispatch> func, std::vector<CComVariant>& vArgs,
                       CComVariant& vtRet );

// Creates a js function that tests if an object is a js array. 
bool makeJsFuncIsArray( IWebBrowser2* pBrowser,
                        CComPtr<IDispatch>& jsfnIsArray );

// Creates a js function that tests if an object is a js function. 
bool makeJsFuncIsFunction( IWebBrowser2* pBrowser,
                           CComPtr<IDispatch>& jsfnIsFunction );


} // bp
} // ie


#endif // _IEUTILS_WINDOWS_H_
