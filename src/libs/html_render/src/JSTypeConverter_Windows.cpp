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
 *  JSTypeConverter_Windows.cpp
 *
 *  Created by David Grigsby on 10/04/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */

#include "JSTypeConverter_Windows.h"
#include <comutil.h>
#include "BPUtils/BPLog.h"
#include "BPUtils/bptypeutil.h"
#include "HTMLTransactionContext.h"
#include "IEUtils_Windows.h"
#include "JSFunctionWrapper.h"


namespace bp {
namespace ie {


JsTypeConverter::JsTypeConverter( const CComPtr<IWebBrowser2>& browser,
                                  bp::html::BPHTMLTransactionContext& tc ) :
    m_browser( browser ),
    m_tc( tc ),
    m_jsfnIsArray( 0 ),
    m_jsfnIsFunction( 0 )
{

}


bool JsTypeConverter::isJsArray( CComPtr<IDispatch> dispIn, bool& bIsArray )
{
    if (!m_jsfnIsArray)
    {
        if (!bp::ie::makeJsFuncIsArray( m_browser, m_jsfnIsArray ))
        {
            BPLOG_ERROR( "makeJsFuncIsArray() failed!" );
            return false;
        }
    }

    // Invoke the check function.
    CComVariant vtRet;
    if (!bp::ie::invokeJsFunction( m_jsfnIsArray, CComVariant(dispIn), vtRet ))
    {
        BPLOG_ERROR( "invokeJsFunction() failed!" );
        return false;
    }

    bIsArray = (vtRet.boolVal == VARIANT_TRUE);

    return true;
}


bool JsTypeConverter::isJsFunction( CComPtr<IDispatch> dispIn,
                                    bool& bIsFunction )
{
    if (!m_jsfnIsFunction)
    {
        if (!bp::ie::makeJsFuncIsFunction( m_browser, m_jsfnIsFunction ))
        {
            BPLOG_ERROR( "makeJsFuncIsFunction() failed!" );
            return false;
        }
    }

    // Invoke the check function.
    CComVariant vtRet;
    if (!bp::ie::invokeJsFunction( m_jsfnIsFunction, CComVariant(dispIn), 
                                   vtRet ))
    {
        BPLOG_ERROR( "invokeJsFunction() failed!" );
        return false;
    }

    bIsFunction = (vtRet.boolVal == VARIANT_TRUE);

    return true;
}


bp::Object* JsTypeConverter::toBPObject( const CComVariant& vtIn )
{
    switch (vtIn.vt)
    {
        case VT_EMPTY:
//          return 0;
            return new bp::Null;
        case VT_NULL:
            return new bp::Null;
        case VT_BOOL:
            return new bp::Bool( vtIn.boolVal );
        case VT_INT:
            return new bp::Integer( vtIn.intVal );
        case VT_I4:
            return new bp::Integer( vtIn.lVal );
        case VT_UI4:
            return new bp::Integer( vtIn.ulVal );
        case VT_R8:
            return new bp::Double( vtIn.dblVal );
        case VT_BSTR:
            return new bp::String( std::string( _bstr_t( vtIn.bstrVal) ) );
        case VT_DISPATCH:
            return toBPObject( CComPtr<IDispatch>(vtIn.pdispVal) );
        default:
            BPLOG_ERROR( "Unexpected variant type." );
            return 0;
    }
}


bp::Object* JsTypeConverter::toBPObject( CComPtr<IDispatch> dispObj )
{
    // Check if this object is a js array.
    bool bIsArray;
    if (!isJsArray( dispObj, bIsArray ))
    {
        BPLOG_ERROR( "isJsArray() failed." );
        return 0;
    }

    // Check if this object is a js function.
    bool bIsFunc;
    if (!isJsFunction( dispObj, bIsFunc ))
    {
        BPLOG_ERROR( "isJsFunction() failed." );
        return 0;
    }

    if (bIsArray)
    {
        bp::List * l = new bp::List;

        int nArrLen;
        if (!bp::ie::getArrayLength( dispObj, nArrLen ))
        {
            BPLOG_ERROR( "bp::ie::getArrayLength failed." );
            delete l;
            return 0;
        }
        
        // now iterate through the array.
        for (int i = 0; i < nArrLen; i++)
        {
            CComVariant vtElem;
            if (!bp::ie::getArrayElement( dispObj, i, vtElem ))
            {
                BPLOG_ERROR( "bp::ie::getArrayElement failed." );
                delete l;
                return 0;
            }
            
            bp::Object* poSubObj = toBPObject( vtElem );
            if (!poSubObj)
            {
                BPLOG_ERROR( "toBPObject failed." );
                delete l;
                return 0;
            }

            l->append( poSubObj );
        }

        return l;
    }
    else if (bIsFunc)
    {
        JSFunctionWrapper fw( (void*)(IWebBrowser2*)m_browser,
                             (void*)(IDispatch*)dispObj );
        bp::Object* cb = new bp::CallBack(fw.id());
        
        // Add the function to the transaction context.
        m_tc.m_callbacks[fw.id()] = fw;
        
        return cb;
    }
    else
    {
        // now we know it's a Map 
        bp::Map * m = new bp::Map;

        // get the keys
        std::vector<std::string> vsKeys;
        if (!bp::ie::enumerateProperties( dispObj, vsKeys ))
        {
            BPLOG_ERROR( "enumerateProperties() failed." );
            delete m;
            return 0;
        }

        // iterate through all values
        for (unsigned int i = 0; i < vsKeys.size(); i++)
        {
            CComVariant vtProp;
            if (!bp::ie::getObjectProperty( dispObj, vsKeys[i], vtProp ))
            {
                BPLOG_ERROR( "getObjectProperty() failed." );
                delete m;
                return 0;
            }

            bp::Object* poSubObj = toBPObject( vtProp );
            if (poSubObj)
            {
                m->add( vsKeys[i].c_str(), poSubObj );
            }
        }

        return m;
    }
}


CComVariant JsTypeConverter::toJSObject( const bp::Object* poIn )
{
    CComVariant vtRet;
    if (!bp::ie::evaluateJson( m_browser,
                               poIn->toPlainJsonString(),
                               vtRet ))
    {
        BPLOG_ERROR( "evaluateJson() failed!" );
        vtRet.Clear();
        vtRet.vt = VT_ERROR;
    }

    return vtRet;
}


} // namespace ie
} // namespace bp

