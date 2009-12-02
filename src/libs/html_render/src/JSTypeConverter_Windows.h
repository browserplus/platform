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
 *  JSTypeConverter_Windows.h
 *
 *  Declares a class to process and convert JS objects into C++ objects.
 *  
 *  Created by David Grigsby on 10/04/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _JSTYPECONVERTER_WINDOWS_H_
#define _JSTYPECONVERTER_WINDOWS_H_

#include <atlbase.h>
#include <atlcomcli.h>
#include <ExDisp.h>


// Forward decls
namespace bp { class Object; }
namespace bp { namespace html { class BPHTMLTransactionContext; } }


namespace bp {
namespace ie {


class JsTypeConverter
{
// Construction/Destruction
public:
    JsTypeConverter( const CComPtr<IWebBrowser2>& browser,
                     bp::html::BPHTMLTransactionContext& tc );

    
// Services
public:
    // TODO: convert to use exceptions.
    
    // Sets bIsArray per whether the variant refers to a JS array.
    // Returns whether check succeeded.
    bool isJsArray( CComPtr<IDispatch> dispIn, bool& bIsArray );

    // Sets bIsFunction per whether the variant refers to a JS function.
    // Returns whether check succeeded.
    bool isJsFunction( CComPtr<IDispatch> dispIn, bool& bIsFunction );
    
    // Returns a pointer to bp obj corresponding to vtIn, or 0 on failure.
    bp::Object* toBPObject( const CComVariant& vtIn );
    
    // Returns a pointer to bp obj corresponding to dispIn, or 0 on failure.
    bp::Object* toBPObject( CComPtr<IDispatch> dispIn );

    // Returns a JS object corresponding to the provided bp object.
    // In case of error, the returned variant will have type VT_ERROR.
    CComVariant toJSObject( const bp::Object* poIn );
    
    
// State
private:
    CComPtr<IWebBrowser2>               m_browser;
    bp::html::BPHTMLTransactionContext& m_tc;
    CComPtr<IDispatch>                  m_jsfnIsArray;
    CComPtr<IDispatch>                  m_jsfnIsFunction;


// Prevent auto-generation
private:
    JsTypeConverter& operator=( const JsTypeConverter& rhs );
};


} // namespace ie
} // namespace js


#endif

