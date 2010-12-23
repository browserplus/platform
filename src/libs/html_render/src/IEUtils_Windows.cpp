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
 *  IEUtils_Windows.cpp
 *
 *  Internet Explorer utilities.
 *
 *  Created by David Grigsby on 10/10/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "IEUtils_Windows.h"
#include "BPUtils/bpconvert.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "ComUtils_Windows.h"


namespace bp {
namespace ie {


bool createJsObject( IWebBrowser2* pBrowser, CComPtr<IDispatchEx>& dexObj )
{
    // Get the scripts.
    CComPtr<IDispatch> dispScripts;
    bool bOk = getJsScripts( pBrowser, dispScripts );
    if (!bOk)
    {
        BPLOG_ERROR( "GetJsScripts() failed!" );
        return false;
    }
    CComQIPtr<IDispatchEx> dexScripts( dispScripts );

    // Get the dispid for the JS Object function.
    CComBSTR bstrObject( "Object" );
    DISPID dispid;
    HRESULT hr = dexScripts->GetDispID( bstrObject, 0, &dispid );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "dexScripts->GetDispID(\"Object\") failed!", hr );
        return false;
    }

    // Invoke Object to create an object.
    DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
    CComVariant vtRet;
    hr = dexScripts->InvokeEx( dispid, LOCALE_USER_DEFAULT, 
                               DISPATCH_CONSTRUCT, &dispparamsNoArgs, 
                               &vtRet, NULL, NULL );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "dexScripts->InvokeEx() failed!", hr );
        return false;
    }

    // Load the caller's arg.
    dexObj = CComQIPtr<IDispatchEx>( vtRet.pdispVal );

    return true;
}


bool enumerateProperties( CComPtr<IDispatch> dispObj,
                          std::vector<std::string>& vsPropNames,
                          bool bOwnPropsOnly /*=true*/ )
{
    CComQIPtr<IDispatchEx> dexObj = dispObj;

    // Note: 'fdexEnumAll' may also be provided, but for the current
    // purposes it is reporting members we don't want.
    DISPID dispid;
    HRESULT hr = dexObj->GetNextDispID( fdexEnumDefault, DISPID_STARTENUM,
                                        &dispid );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "dexObj.GetNextDispID() failed!", hr );
        return false;
    }

    while (hr == NOERROR)
    {
        CComBSTR bstrName;
        hr = dexObj->GetMemberName( dispid, &bstrName );
        if (SUCCEEDED( hr ))
        {
            std::string sPropName = _bstr_t( bstrName );

            bool bAddProp = true;

            if (bOwnPropsOnly)
            {
                // Ignore properties from prototypes.
                bool bHasOwnProp;
                if (!hasOwnProperty( dispObj, sPropName, bHasOwnProp ))
                {
                    BPLOG_ERROR( "hasOwnProperty() failed!" );
                    return false;
                }

                bAddProp = bHasOwnProp;
            }

            if (bAddProp )
            {
                vsPropNames.push_back( sPropName );
            }
        }

        hr = dexObj->GetNextDispID(fdexEnumDefault, dispid, &dispid);
    }

    return true;
}


bool evaluateJson( IWebBrowser2* pBrowser, const std::string& sJson,
                   CComVariant& vtRet )
{
    // For firefox on mac, evaluating an object yields null.  specifically:
    //   "{foo: 'bar'}" -> null
    //   "[{foo: 'bar'}][0]" -> object
    // don't ask me why. (lth)
    // 
    // Similar situation on IE.
    //   "2 + 3;" -> VT_I4 5
    //   "{\"foo\":25}" -> hr error -2146827284
    //   "{'foo':25}" -> hr error -2146827284
    //   "{foo:25}" -> VT_I4 25
    //   "var fooobj = {\"foo\":25};" -> VT_EMPTY
    //   "[{foo: 'bar'}][0]" -> VT_DISPATCH
    //   "[{\"somekey\": \"someval\"}][0]" -> VT_DISPATCH

    std::string sScript = "[" + sJson + "][0]";

    if (!bp::ie::evaluateScript( pBrowser, sScript, vtRet ))
    {
        BPLOG_ERROR( "evaluateScript() failed!" );
        return false;
    }

    // If we're returning an object, bump its reference count.
    if (vtRet.vt == VT_DISPATCH)
    {
        vtRet.pdispVal->AddRef();
    }


    return true;
}


// Evaluate script using the js eval() method.
bool evaluateScript( IWebBrowser2* pBrowser, const std::string& sScript,
                     CComVariant& vtRet )
{
    //BPLOG_FUNC_SCOPE;

    CComVariant vtArg = bp::strutil::utf8ToWide( sScript ).c_str();
    bool bOk = invokeGlobalJsFunction( pBrowser, "eval", vtArg, vtRet );
    if (!bOk)
    {
        BPLOG_ERROR( "invokeGlobalJsFunction(\"eval\") failed!" );
        return false;
    }

    // TODO: bump ref count for objects created here?
    //       or maybe it should be caller's responsibilty
    
    return true;
}


bool executeScript( IWebBrowser2* pBrowser,
                    const std::string& sScript )
{
    CComPtr<IHTMLWindow2> win2;
    if (!getDocWindow( pBrowser, win2 ))
    {
        BPLOG_ERROR( "getDocWindow() failed!" );
        return false;
    }
    
    CComBSTR bstrScript( sScript.c_str() );
    CComVariant vtRet;
    // Note: vtRet does not receive the results of the script execution.
    // See: http://msdn.microsoft.com/en-us/library/aa741364.aspx
    HRESULT hr = win2->execScript( bstrScript, NULL, &vtRet );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "win2->execScript() failed!", hr );
        return false;
    }

    return true;
}


// Get the specified array element of the object.
bool getArrayElement( CComPtr<IDispatch> dispObj,
                      int nIdx,
                      CComVariant& vtVal )
{
    // For IE, array elements are accessed by properties named "0", "1", etc.
    if (!getObjectProperty( dispObj, bp::conv::toString( nIdx ), vtVal ))
    {
        BPLOG_ERROR( "getObjectProperty() failed." );
        return false;
    }

    return true;
}


// Get the length of the specified js array.
bool getArrayLength( CComPtr<IDispatch> dispObj, int& nSize )
{
    CComVariant vtVal;
    if (!getObjectProperty( dispObj, "length", vtVal ))
    {
        BPLOG_ERROR( "getObjectProperty() failed." );
        return false;
    }

    vtVal.ChangeType( VT_INT );
    nSize = vtVal.intVal;
    
    return true;
}


bool getContentSize( IWebBrowser2* pBrowser, int& nWidth, int& nHeight )
{
    CComPtr<IHTMLDocument2> doc;
    if (!bp::ie::getHtmlDoc2( pBrowser, doc ))
    {
        BPLOG_ERROR( "getHtmlDoc2() failed!" );
        return false;
    }

    CComPtr<IHTMLElement> elemBody;
    HRESULT hr = doc->get_body( &elemBody );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "doc->get_body() failed!", hr );
        return false;
    }

    long lWidth;
    hr = elemBody->get_offsetWidth( &lWidth );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "elemBody->get_offsetWidth() failed!", hr );
        return false;
    }

    long lHeight;
    hr = elemBody->get_offsetHeight( &lHeight );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "elemBody->get_offsetHeight() failed!", hr );
        return false;
    }

    nWidth  = (int)lWidth;
    nHeight = (int)lHeight;
    
    return true;
}


bool getDocElementById( IWebBrowser2* pBrowser, const std::string& sId,
                        CComPtr<IHTMLElement>& elem )
{
    if (!pBrowser)
    {
        BPLOG_ERROR( "pBrowser == 0" );
        return false;
    }

    // Get the currently loaded document.
    CComDispatchDriver dispDocument;
    HRESULT hr = pBrowser->get_Document(&dispDocument);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "browser->get_Document() failed!", hr );
        return false;
    }

    // TODO: use CComQIPtr.
    CComPtr<IHTMLDocument3> doc;
    hr = dispDocument->QueryInterface(IID_IHTMLDocument3, (void**)&doc);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR("dispDocument->QueryInterface(IHTMLDocument3) failed!",
                       hr);
        return false;
    }

    // Get the element we're interested in.
    hr = doc->getElementById( _bstr_t(sId.c_str()), &elem);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR("doc->getElementById failed!", hr);
        return false;
    }

    return true;
}

bool getDocWindow( IWebBrowser2* pBrowser, CComPtr<IHTMLWindow2>& win2 )
{
    CComPtr<IHTMLDocument2> doc2;
    bool bOk = getHtmlDoc2( pBrowser, doc2 );
    if (!bOk)
    {
        BPLOG_ERROR( "getHtmlDoc2() failed!" );
        return false;
    }

    doc2->get_parentWindow( &win2 );
    if (!win2)
    {
        BPLOG_ERROR( "doc2->get_parentWindow() failed!" );
        return false;
    }

    return true;
}


bool getHtmlDoc2( IWebBrowser2* pBrowser, CComPtr<IHTMLDocument2>& doc2 )
{
    if (!pBrowser)
    {
        BPLOG_ERROR( "pBrowser == 0" );
        return false;
    }

    // Get IDispatch interface to the document.
    CComPtr<IDispatch> dispDoc;
    HRESULT hr = pBrowser->get_Document(&dispDoc);
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "pBrowser->get_Document() failed!", hr );
        return false;
    }

    // Get IHTMLDocument2 interface to the document.
    CComQIPtr<IHTMLDocument2> qiDoc2 = dispDoc;
    if (!qiDoc2)
    {
        BPLOG_ERROR( "QI for IHTMLDocument2 failed!" );
        return false;
    }

    doc2 = qiDoc2;
    
    return true;
}


bool getJsScripts( IWebBrowser2* pBrowser, CComPtr<IDispatch>& dispScripts )
{
    CComQIPtr<IHTMLDocument2> doc2;
    bool bOk = getHtmlDoc2( pBrowser, doc2 );
    if (!bOk)
    {
        BPLOG_ERROR( "getHtmlDoc2() failed!" );
        return false;
    }
    
    // Get IDispatch interface to the document's scripts.
    HRESULT hr = doc2->get_Script( &dispScripts );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "doc2->get_Script() failed!", hr );
        return false;
    }

    return true;
}


bool getObjectProperty( CComPtr<IDispatch> dispObj,
                        const std::string& sPropName,
                        CComVariant& vtVal )
{
    HRESULT hr = dispObj.GetPropertyByName( CComBSTR( sPropName.c_str() ),
                                            &vtVal );
    if (FAILED( hr ))
    {
        if (hr == DISP_E_UNKNOWNNAME)
        {
            BPLOG_INFO_STRM( "property \"" << sPropName << "\" not found." );
        }
        else
        {
            BPLOG_COM_ERROR( "m_data.GetPropertyByName() failed!", hr );
        }
        return false;
    }

    return true;
}


bool getSourceId( IHTMLEventObj* pEvtObj, std::string& sSourceId )
{
    sSourceId.clear();

    // Figure out id of element that fired the event.
    CComPtr<IHTMLElement> elem;
    HRESULT hr = pEvtObj->get_srcElement(&elem);
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "pEvtObj->get_srcElement() failed!", hr );
        return false;
    }
    
    CComBSTR bstrId;
    hr = elem->get_id(&bstrId);
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "elem->get_id() failed!", hr );
        return false;
    }
    
    // get_id will succeed and return a NULL pointer 
    // if the element does not have an .id attribute (YIB-2869494) 
    if (NULL != bstrId) 
    {
        sSourceId = _bstr_t(bstrId);
    }

    return true;
}


bool getUserAgent( IWebBrowser2* pBrowser, std::string& sUserAgent )
{
    // Get document.parentWindow.navigator.userAgent
    
    CComPtr<IHTMLWindow2> win2;
    if (!getDocWindow( pBrowser, win2 ))
    {
        BPLOG_ERROR( "getDocWindow() failed!" );
        return false;
    }

    CComPtr<IOmNavigator> nav;
    HRESULT hr = win2->get_navigator( &nav );
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "win2->get_navigator() failed!", hr );
        return false;
    }

    CComBSTR bstrAgent;
    hr = nav->get_userAgent( &bstrAgent );
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "nav->get_userAgent() failed!", hr );
        return false;
    }
    
    sUserAgent = _bstr_t( bstrAgent );
    
    return true;
}
   

// Get whether the object has an own property of the specified name.
bool hasOwnProperty( CComPtr<IDispatch> dispObj, const std::string& sPropName,
                     bool& bHasProp )
{
    CComBSTR bstrFunc( "hasOwnProperty" );
    CComBSTR bstrProp( sPropName.c_str() );
    CComVariant vtProp( bstrProp );
    CComVariant vtRet;
    
    HRESULT hr = dispObj.Invoke1( bstrFunc, &vtProp, &vtRet );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "dispObj->Invoke1() failed!", hr );
        return false;
    }

    bHasProp = vtRet.boolVal == VARIANT_TRUE;
    
    return true;
}



// TODO: this could be changed to a two step process.
//  1) User calls getJsFunction( name, disp )
//  2) User calls invokeJsFunction( disp, arg(s), ret )

bool invokeGlobalJsFunction( IWebBrowser2* pBrowser,
                             const std::string& sFuncName, CComVariant vtArg,
                             CComVariant& vtRet )
{
    //BPLOG_FUNC_SCOPE;

    CComPtr<IDispatch> dispScripts;
    bool bOk = getJsScripts( pBrowser, dispScripts );
    if (!bOk)
    {
        BPLOG_ERROR( "GetJsScripts() failed!" );
        return false;
    }

    // Must Invoke via an IDispatchEx.  In IE9 beta, IDispatch::Invoke fails
    CComQIPtr<IDispatchEx> dexObj = dispScripts;

    CComBSTR bstrFunc( sFuncName.c_str() );
    HRESULT hr = dexObj.Invoke1( bstrFunc, &vtArg, &vtRet );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "dispScript->Invoke1() failed!", hr );
        return false;
    }

    return true;
}


/*
__int64 getAvailStack()
{
    MEMORY_BASIC_INFORMATION mi;

    // Find SP address
    LPBYTE lpPage;
    _asm mov lpPage, esp;

    // Get allocation base of stack
    VirtualQuery(lpPage, &mi, sizeof(mi));

    int i;

    __int64 lCurrAddr = (__int64) &i;
    __int64 lBase = (__int64) mi.AllocationBase;

    __int64 nAvail = lCurrAddr - lBase;

    return nAvail;
}
*/


bool invokeJsFunction( CComPtr<IDispatch> func, 
                       CComVariant vtArg,
                       CComVariant& vtRet )
{
//  BPLOG_FUNC_SCOPE;
    
    HRESULT hr = func.Invoke1( DISPID(DISPID_VALUE), &vtArg, &vtRet );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "func.Invoke1() failed!", hr );
        return false;
    }

    return true;
}


bool invokeJsFunction( CComPtr<IDispatch> func, 
                       std::vector<CComVariant>& vArgs,
                       CComVariant& vtRet )
{
//  BPLOG_FUNC_SCOPE;

    if (vArgs.empty())
    {
        HRESULT hr = func.Invoke0( DISPID(DISPID_VALUE), &vtRet );
        if (FAILED( hr ))
        {
            BPLOG_COM_ERROR( "func.Invoke0() failed!", hr );
            return false;
        }
    }
    else
    {
        // Have to pass in reverse order.
        // TODO: should probably do this on a copy
        std::reverse( vArgs.begin(), vArgs.end() );

        HRESULT hr = func.InvokeN( DISPID(DISPID_VALUE),
                                   &vArgs[0], vArgs.size(), 
                                   &vtRet );
        if (FAILED( hr ))
        {
            BPLOG_COM_ERROR( "func.InvokeN() failed!", hr );
            return false;
        }
    }

    return true;
}


bool makeJsFuncIsArray( IWebBrowser2* pBrowser,
                        CComPtr<IDispatch>& jsfnIsArray )
{
    // Create the function we will use to check array-hood.

    // Note: the comment below and the implementation are taken
    // from YUI's yahoo.js:
    //   Testing typeof/instanceof/constructor of arrays across frame 
    //   boundaries isn't possible in Safari unless you have a reference
    //   to the other frame to test against its Array prototype.  To
    //   handle this case, we test well-known array properties instead.
    std::string sScriptIsArray = "[function(x) {"
                                 "  return (typeof x.length === 'number') &&"
                                 "         isFinite(x.length) &&"
                                 "         (typeof x.splice === 'function');"
                                 "}][0]";
/*    
    std::string sScriptIsArray = "[function(x) {"
                                 "  return (x.constructor == Array);"
                                 "}][0]";
*/
    
    CComVariant vtRet;
    if (!evaluateScript( pBrowser, sScriptIsArray, vtRet ))
    {
        BPLOG_ERROR( "evaluateScript() failed!" );
        return false;
    }

    jsfnIsArray = vtRet.pdispVal;

    return true;
}


bool makeJsFuncIsFunction( IWebBrowser2* pBrowser,
                           CComPtr<IDispatch>& jsfnIsFunction )
{
    // Create the function we will use to check constructor.
    std::string sScriptIsFunc = "[function(x) {"
                                "  return (x.constructor == Function);"
                                "}][0]";


    CComVariant vtRet;
    if (!evaluateScript( pBrowser, sScriptIsFunc, vtRet ))
    {
        BPLOG_ERROR( "evaluateScript() failed!" );
        return false;
    }

    jsfnIsFunction = vtRet.pdispVal;

    return true;
}



} // bp
} // ie


