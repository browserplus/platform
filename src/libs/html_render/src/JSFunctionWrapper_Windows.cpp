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

/*
 *  JSTypeConverter_Windows.cpp
 *
 *  Created by David Grigsby on 10/06/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "JSFunctionWrapper.h"

#include <atlbase.h>
#include <atlcomcli.h>
#include <comutil.h>
#include <ExDisp.h>

#include "BPUtils/BPLog.h"
#include "HTMLTransactionContext.h"
#include "IEUtils_Windows.h"
#include "JSTypeConverter_Windows.h"


bp::Object* JSFunctionWrapper::invoke( std::vector<const bp::Object*> vBPArgs )
{
    if (!m_osContext)
    {
        BPLOG_ERROR( "no callback context." );
        return 0;
    }
    
    if (!m_osCallback)
    {
        BPLOG_ERROR( "no callback method." );
        return 0;
    }

    // Get the browser.
    IWebBrowser2* pBrsr = (IWebBrowser2*) m_osContext;
    CComPtr<IWebBrowser2> browser( pBrsr );
    
    // Get the method to call.
    IDispatch* pDisp = (IDispatch*) m_osCallback;
    CComPtr<IDispatch> dispMethod( pDisp );
    
    // Setup a type converter.
    bp::html::BPHTMLTransactionContext tc; // dummy
    bp::ie::JsTypeConverter conv( browser, tc );
    
    // Setup the js args vector.
    std::vector<CComVariant> vJSArgs;
    for (std::vector<const bp::Object*>::iterator it = vBPArgs.begin();
         it != vBPArgs.end(); ++it)
    {
        vJSArgs.push_back( conv.toJSObject( *it ) );
    }
    
    // Invoke the js method.
    CComVariant vtRet;
    if (!bp::ie::invokeJsFunction( dispMethod, vJSArgs, vtRet ))
    {
        BPLOG_ERROR( "invokeJSFunction() failed." );
        return 0;
    }

    // Return converted method results
    return conv.toBPObject( vtRet );
}


void JSFunctionWrapper::retain( void* o )
{
    // We can get called with null in certain situations.
    IDispatch* pDisp = ((IDispatch*) o);
    if (!pDisp)
        return;
    
    pDisp->AddRef();
}


void JSFunctionWrapper::release( void* o )
{
    // We can get called with null in certain situations.
    IDispatch* pDisp = ((IDispatch*) o);
    if (!pDisp)
        return;
    
    pDisp->Release();
}

