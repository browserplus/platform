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
 *  HTMLScriptableComObject_Windows.cpp
 *
 *  Implementation for a special bp com object suitable for interaction with
 *  script.
 *
 *  Created by David Grigsby on 9/12/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HTMLScriptableComObject_Windows.h"

#include <comutil.h>
#include <sstream>
#include <string>
#include <vector>

#include "BPUtils/BPLog.h"
#include "HTMLScriptObject.h"
#include "HTMLTransactionContext.h"
#include "IEUtils_Windows.h"
#include "JSTypeConverter_Windows.h"


using namespace std;


// This is a class that allows us to access ScriptableFunctionHost internals.
namespace bp { namespace html {
class ScriptFunctionHostProxyClass 
{
public:
    static void release(ScriptableFunctionHost * host, unsigned int id)
    {
        host->release(id);
    }
    
    static unsigned int addTransaction(ScriptableFunctionHost * host,
                                       bp::html::BPHTMLTransactionContext * t)
    {
        return host->addTransaction(t);
    }
};
}}


void
BpScriptableComObject::setScriptableObject( const bp::html::ScriptableObject* pSO )
{
    m_pSO = pSO;
}


void
BpScriptableComObject::setBrowser( const CComPtr<IWebBrowser2>& browser )
{
    m_browser = browser;
}


STDMETHODIMP
BpScriptableComObject::_bpInvoke( BSTR bstrMethodName,
                                  VARIANT vtArgs,
                                  VARIANT* pvtResult )
{
    using bp::html::ScriptableFunctionHost;
    using bp::html::ScriptFunctionHostProxyClass;
    using bp::html::BPHTMLTransactionContext;
    
//  pvtResult->vt = VT_EMPTY;
    
    // Get the "function host" for the specified method.
    string sMethod = _bstr_t( bstrMethodName );
    
    bp::html::ScriptableFunctionHost* pFuncHost =
        m_pSO->findFunctionHost( sMethod);
    if (pFuncHost == NULL)
    {
        BPLOG_ERROR_STRM( "unrecognized method \"" << sMethod << "\"." );
        return E_FAIL;
    }

    // Allocate a new transaction context.
    BPHTMLTransactionContext* tc = new BPHTMLTransactionContext;
    
    // Add our transaction context to the ScriptableFunctionHost.
    unsigned int tid =
        ScriptFunctionHostProxyClass::addTransaction( pFuncHost, tc );

    // Setup a type converter.
    bp::ie::JsTypeConverter conv( m_browser, *tc );

    // We expect to get an array of args, and we'll load these into a
    // vector<bp::Object*>.
    //
    // Note: would be nice to verify that the arg is representing a js array but
    // that check is currently failing in the case of the array passed to us
    // by the func created by createInvokerJsFunc().
    CComPtr<IDispatch> dispJsArgArr( vtArgs.pdispVal );
    
    int nArrLen;
    if (!bp::ie::getArrayLength( dispJsArgArr, nArrLen ))
    {
        BPLOG_ERROR( "bp::ie::getArrayLength failed." );
        return E_FAIL;
    }
                                
    vector<const bp::Object*> vArgs;
    for (int i = 0; i < nArrLen; i++)
    {
        CComVariant vtElem;
        if (!bp::ie::getArrayElement( dispJsArgArr, i, vtElem ))
        {
            BPLOG_ERROR( "bp::ie::getArrayElement failed." );
            return E_FAIL;
        }
        
        bp::Object* pObj = conv.toBPObject( vtElem );
        if (!pObj)
        {
            BPLOG_ERROR( "conv.toBPObject failed." );
            return E_FAIL;
        }
                
        vArgs.push_back( pObj );
    }
    
    // Invoke the method.
    bp::Object* poRet = pFuncHost->invoke( sMethod, tid, vArgs );

    // Release our transaction context.  If the function wished
    // to preserve it for callback invocation, it will have already
    // called ScriptableFunctionHost::retain().
    ScriptFunctionHostProxyClass::release( pFuncHost, tid );

    // Free the arguments.
    while (vArgs.size() > 0)
    {
        delete vArgs.back();
        vArgs.pop_back();
    }
    
    // Handle the case where the FuncHost returns 0.
    if (!poRet)
    {
        poRet = new bp::Null;
    }

    // Setup return value.
    CComVariant vtRet = conv.toJSObject( poRet );

    if (vtRet.vt == VT_ERROR)
    {
        BPLOG_ERROR( "conv.toJSObject failed." );
        return E_FAIL;
    }

    pvtResult->vt = VT_EMPTY;
//  *pvtResult = vtRet;
//  return S_OK;

    return vtRet.Detach( pvtResult );
}

