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
 *  BPCtl.cpp
 *
 *  Created by David Grigsby on 06/11/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

// TODO:
//   logging/tracing
//   ISupportErrorInfo
//   Maybe have impl funcs that are pure C++, no COM-isms.

#include "stdafx.h"
#include "BPCtl.h"

#include <ShlGuid.h>

#include "AxObject.h"
#include "AxVariant.h"
#include "DnDPlugletFactoryAx.h"
#include "BPUtils/bpdebug.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/OS.h"
#include "HTMLRender/ComUtils_Windows.h"
#include "HTMLRender/IEUtils_Windows.h"
#include "PluginCommonLib/BPSession.h"
#include "PluginCommonLib/DnDPluglet.h"
#include "PluginCommonLib/FileBrowsePluglet.h"
#include "PluginCommonLib/LogPluglet.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Initialization / Teardown
//
CBPCtl::CBPCtl() :
m_pSession( 0 )
{
}

CBPCtl::~CBPCtl()
{
}


HRESULT
CBPCtl::FinalConstruct()
{
    bp::debug::breakpoint( "ax.FinalConstruct" );
    BPLOG_FUNC_SCOPE;

    return S_OK;
}


void
CBPCtl::FinalRelease()
{
    BPLOG_FUNC_SCOPE;
}



////////////////////////////////////////////////////////////////////////////////
// IBPCtl Support
//

STDMETHODIMP 
CBPCtl::Initialize( VARIANT vtArg, VARIANT vtCallback, VARIANT* pvtResult )
{
    BPLOG_FUNC_SCOPE;

    // Type-check the callback arg.
    if (vtCallback.vt != VT_DISPATCH)
    {
        BPLOG_ERROR( "Invalid callback provided!" );
        return E_FAIL;
    }

    // Check the result arg.
    if (!pvtResult)
    {
        BPLOG_ERROR( "Invalid result arg provided!" );
        return E_FAIL;
    }

    if (!m_pSession)
    {
        BPLOG_ERROR( "m_pSession == 0" );
        return E_FAIL;
    }
    
    AxVariant avtArg( vtArg );
    AxObject oCallback( vtCallback.pdispVal );
    AxVariant vtResult;
    if (!m_pSession->initialize( &avtArg, &oCallback, &vtResult ))
    {
        BPLOG_ERROR( "m_pSession->initialize() failed!" );
        *pvtResult = vtResult.data();
        return E_FAIL;
    }

    *pvtResult = vtResult.data();
    return S_OK;
}


STDMETHODIMP 
CBPCtl::EnumerateServices( VARIANT vtCallback, VARIANT* pvtResult )
{
    BPLOG_FUNC_SCOPE;

    // Type-check the callback arg.
    if (vtCallback.vt != VT_DISPATCH)
    {
        BPLOG_ERROR( "Invalid callback provided!" );
        return E_FAIL;
    }

    // Check the result arg.
    if (!pvtResult)
    {
        BPLOG_ERROR( "Invalid result arg provided!" );
        return E_FAIL;
    }

    if (!m_pSession)
    {
        BPLOG_ERROR( "m_pSession == 0" );
        return E_FAIL;
    }
    
    AxObject oCallback( vtCallback.pdispVal );
    AxVariant vtResult;
    if (!m_pSession->enumerateServices( &oCallback, &vtResult ))
    {
        BPLOG_ERROR( "m_pSession->enumerateServices() failed!" );
        *pvtResult = vtResult.data();
        return E_FAIL;
    }

    *pvtResult = vtResult.data();
    return S_OK;
}


STDMETHODIMP 
CBPCtl::RequireService( VARIANT vtArg, VARIANT vtCallback, VARIANT* pvtResult )
{
    BPLOG_FUNC_SCOPE;
    
    // Setup the arg.
    AxVariant avtArg( vtArg );
    
    // Type-check the callback arg.
    if (vtCallback.vt != VT_DISPATCH)
    {
        BPLOG_ERROR( "Invalid callback provided!" );
        return E_FAIL;
    }

    // Check the result arg.
    if (!pvtResult)
    {
        BPLOG_ERROR( "Invalid result arg provided!" );
        return E_FAIL;
    }

    if (!m_pSession)
    {
        BPLOG_ERROR( "m_pSession == 0" );
        return E_FAIL;
    }
    
    AxObject oCallback( vtCallback.pdispVal );
    AxVariant vtResult;
    if (!m_pSession->requireServices( &avtArg, &oCallback, &vtResult ))
    {
        BPLOG_ERROR( "m_pSession->requireService() failed!" );
        *pvtResult = vtResult.data();
        return E_FAIL;
    }

    *pvtResult = vtResult.data();
    return S_OK;
}


STDMETHODIMP CBPCtl::DescribeService( VARIANT vtArg,
                                      VARIANT vtCallback,
                                      VARIANT* pvtResult )
{
    BPLOG_FUNC_SCOPE;
    
    // Setup the arg.
    AxVariant avtArg( vtArg );

    // Type-check the callback arg.
    if (vtCallback.vt != VT_DISPATCH)
    {
        BPLOG_ERROR( "Invalid callback provided!" );
        return E_FAIL;
    }

    // Check the result arg.
    if (!pvtResult)
    {
        BPLOG_ERROR( "Invalid result arg provided!" );
        return E_FAIL;
    }

    if (!m_pSession)
    {
        BPLOG_ERROR( "m_pSession == 0" );
        return E_FAIL;
    }
    
    AxObject oCallback( vtCallback.pdispVal );
    AxVariant vtResult;
    if (!m_pSession->describeService( &avtArg, &oCallback, &vtResult ))
    {
        BPLOG_ERROR( "m_pSession->requireService() failed!" );
        *pvtResult = vtResult.data();
        return E_FAIL;
    }

    *pvtResult = vtResult.data();
    return S_OK;
}


STDMETHODIMP
CBPCtl::ExecuteMethod( BSTR bstrServiceName, BSTR bstrServiceVer,
                       BSTR bstrMethodName, VARIANT vtArg,
                       VARIANT vtCallback,
                       VARIANT* pvtResult )
{
    // Setup the arg.
    AxVariant avtArg( vtArg );

    // Type-check the callback arg.
    if (vtCallback.vt != VT_DISPATCH)
    {
        BPLOG_ERROR( "Invalid callback provided!" );
        return E_FAIL;
    }

    // Check the result arg.
    if (!pvtResult)
    {
        BPLOG_ERROR( "Invalid result arg provided!" );
        return E_FAIL;
    }

    if (!m_pSession)
    {
        BPLOG_ERROR( "m_pSession == 0" );
        return E_FAIL;
    }
    
    AxObject oCallback( vtCallback.pdispVal );
    AxVariant vtResult;
    if (!m_pSession->executeMethod( std::string( _bstr_t( bstrServiceName ) ),
                                    std::string( _bstr_t( bstrServiceVer ) ),
                                    std::string( _bstr_t( bstrMethodName) ),
                                    &avtArg, &oCallback, &vtResult ) )
    {
        BPLOG_ERROR( "m_pSession->executeMethod() failed!" );
        *pvtResult = vtResult.data();
        return E_FAIL;
    }

    *pvtResult = vtResult.data();
    return S_OK;
}


STDMETHODIMP 
CBPCtl::Ping()
{
    BPLOG_FUNC_SCOPE;
    return S_OK;
}


STDMETHODIMP CBPCtl::Info( VARIANT* pvtResult )
{
    BPLOG_FUNC_SCOPE;
    
    // Check the result arg.
    if (!pvtResult)
    {
        BPLOG_ERROR( "Invalid version arg provided!" );
        return E_FAIL;
    }

    bp::Map jmInfo;
    
    std::stringstream ss;
    ss << BP_VERSION_MAJOR << "."
       << BP_VERSION_MINOR << "."
       << BP_VERSION_MICRO;
    jmInfo.add( "version", new bp::String( ss.str() ) );
    
    jmInfo.add( "os", new bp::String( bp::os::PlatformAsString() ) );

    jmInfo.add( "buildType", new bp::String( bp::pluginutil::getBuildType() ) );
    
    AxVariant vtResult;
    if (!evaluateJSON( &jmInfo, &vtResult ))
    {
        BPLOG_ERROR( "evaluateJSON() failed!" );
        *pvtResult = vtResult.data();
        return E_FAIL;
    }

    *pvtResult = vtResult.data();    
    return S_OK;
}



////////////////////////////////////////////////////////////////////////////////
// IOleObject Support
//
STDMETHODIMP
CBPCtl::SetClientSite(IOleClientSite* pClientSite)
{
    BPLOG_FUNC_SCOPE;
    BPLOG_INFO_STRM( "SetClientSite: pClientSite=" << pClientSite );

    // See http://support.microsoft.com/kb/257717/.
    
    if (pClientSite)
    {
        if (!HandleBrowserAttach( pClientSite ))
        {
            BPLOG_ERROR( "HandleBrowserAttach() failed!" );
        }
    }
    else
    {
        if (!HandleBrowserDetach())
        {
            BPLOG_ERROR( "HandleBrowserDetach() failed!" );
        }
    }
    
    return __super::SetClientSite(pClientSite);
}


////////////////////////////////////////////////////////////////////////////////
// BPPlugin Support
//
plugin::Variant*
CBPCtl::allocVariant() const
{
    return new AxVariant();
}


bool
CBPCtl::callJsFunction( const plugin::Object* poFunc,
                        plugin::Variant* args[], int nArgCount,
                        plugin::Variant* pvtRet ) const
{
    BPLOG_FUNC_SCOPE;

    vector<CComVariant> vArgs;
    for (int i=0; i<nArgCount; ++i)
    {
        vArgs.push_back( dynamic_cast<AxVariant*>(args[i])->data() );
    }

    CComPtr<IDispatch> func( dynamic_cast<const AxObject*>(poFunc)->data() );
    
    CComVariant vtRet;
    if (!bp::ie::invokeJsFunction( func, vArgs, vtRet ))
    {
        BPLOG_ERROR( "invokeJsFunction() failed!" );
        return false;
    }

    // Set the caller's arg.
    dynamic_cast<AxVariant*>(pvtRet)->data() = vtRet;
    
    return true;
}


list<Pluglet*>
CBPCtl::createPluglets( const std::string& sName ) const
{
    list<Pluglet*> rval;
    BPPlugin* pPlugin = const_cast<CBPCtl*>(this);

    if (sName == "DragAndDrop")
    {
        // Have to do 2-step casting in this case, since we had to derive from
        // AxDropManager for connection point reasons.
        const IDropManager* pCDM = this;
        IDropManager* pDropMgr = const_cast<IDropManager*>(pCDM);
        DnDPlugletFactoryAx factory;
        rval = factory.createPluglets( pPlugin, pDropMgr );
    }
    else if (sName == "FileBrowser")
    {
        FileBrowsePlugletFactory factory;
        rval = factory.createPluglets( pPlugin);
    }
    else if (sName == "Log")
    {
        LogPlugletFactory factory;
        rval = factory.createPluglets( pPlugin );
    }
    return rval;
}


bool
CBPCtl::enumerateProperties( const plugin::Object* pObj,
                             std::vector<std::string>& vsProps ) const
{
    CComPtr<IDispatch> dispObj = dynamic_cast<const AxObject*>(pObj)->data();
    if (!bp::ie::enumerateProperties( dispObj, vsProps ))
    {
        BPLOG_ERROR( "bp::ie::enumerateProperties() failed!" );
        return false;
    }

    return true;
}


bool 
CBPCtl::evaluateJSON( const bp::Object* poJson,
                      plugin::Variant* pvtRet ) const
{
    std::string sScript = poJson->toPlainJsonString();
    
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
   
    if (poJson->type() == BPTMap)
    {
        sScript = "[" + sScript + "][0]";
    }
    
    CComVariant vtRet;
    if (!bp::ie::evaluateScript( m_spLocalBrowser, sScript, vtRet ))
    {
        BPLOG_ERROR( "evaluateScript() failed!" );
        return false;
    }

    // TODO: When to release this?!
    if (vtRet.vt == VT_DISPATCH)
    {
        vtRet.pdispVal->AddRef();
    }
    
    // Set the caller's arg.
    dynamic_cast<AxVariant*>(pvtRet)->data() = vtRet;
    
    return true;
}


void
CBPCtl::freeVariant( plugin::Variant* pvt ) const
{
    delete pvt;
}


bool
CBPCtl::getArrayElement( const plugin::Object* pObj,
                         int nIdx,
                         plugin::Variant* pvtElem ) const
{
    CComPtr<IDispatch> dispObj = dynamic_cast<const AxObject*>(pObj)->data();
    CComVariant vtVal;
    if (!bp::ie::getArrayElement( dispObj, nIdx, vtVal ))
    {
        BPLOG_ERROR( "bp::ie::getArrayElement() failed." );
        return false;
    }

    // Set the caller's arg.
    dynamic_cast<AxVariant*>(pvtElem)->data() = vtVal;

    return true;
}


bool
CBPCtl::getArrayLength( const plugin::Object* pObj, int& nLength ) const
{
    CComPtr<IDispatch> dispObj = dynamic_cast<const AxObject*>(pObj)->data();
    CComVariant vtVal;
    if (!bp::ie::getArrayLength( dispObj, nLength ))
    {
        // Here we just do info-level logging because client code currently
        // increments array idx until a failure reported (i.e. failure
        // is expected).
        BPLOG_ERROR( "bp::ie::getArrayLength() failed." );
        return false;
    }

    return true;
}


bool
CBPCtl::getCurrentURL( std::string& sUrl ) const
{
    // For reference, this returns url of top:
    // CComPtr<IHTMLDocument2> doc2;
    // if (!bp::ie::getHtmlDoc2( m_spTopBrowser, doc2 ))
    // {
    //     BPLOG_ERROR( "getHtmlDoc2() failed!" );
    //     return false;
    // }
    // CComBSTR bstrUrl;
    // HRESULT hr = doc2->get_URL( &bstrUrl );
    // if (FAILED( hr ))
    // {
    //     BPLOG_COM_ERROR( "doc2->get_URL() failed!", hr );
    //     return false;
    // }
    //
    // So does this:
    // CComBSTR bstrUrl;
    // HRESULT hr = m_spTopBrowser->get_LocationURL( &bstrUrl );
    // if (FAILED( hr ))
    // {
    //     BPLOG_COM_ERROR( "m_spTopBrowser->get_LocationURL() failed!", hr );
    //     return false;
    // }

    // Get location from the "local" browser,
    // meaning we will show url of frames, for example.
    if (!m_spLocalBrowser)
    {
        BPLOG_ERROR( "m_spLocalBrowser == 0" );
        return false;
    }
    
    CComBSTR bstrUrl;
    HRESULT hr = m_spLocalBrowser->get_LocationURL( &bstrUrl );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "m_spLocalBrowser->get_LocationURL() failed!", hr );
        return false;
    }
    
    sUrl = bp::strutil::wideToUtf8( wstring( bstrUrl ) );

    // If the url starts out file://, but it's not file:///xyz and it's
    // not file://localhost, change file:// to file:///
    // See RFC 1738.  What we're doing is setting host to empty.
    if (sUrl.find("file://") == 0)
    {
        if ((sUrl.find("file://localhost") != 0) &&
            (sUrl.find("file:///") != 0))
        {
            sUrl.insert( 7, "/" );
        }
    }
    
    return true;
}


bool
CBPCtl::getElementProperty( const plugin::Variant* pvtIn,
                            const std::string& sPropName,
                            plugin::Variant* pvtVal ) const
{
    if (!pvtIn->isObject())
    {
        BPLOG_ERROR( "pvtIn->isObject() returned false!" );
        return false;
    }

    plugin::Object* pObj;
    if (!pvtIn->getObjectValue( pObj ))
    {
        BPLOG_ERROR( "pvtIn->getObjectValue() failed!" );
        return false;
    }

    return getProperty( pObj, sPropName, pvtVal );
}


bool
CBPCtl::getProperty( const plugin::Object* pObj,
                     const std::string& sPropName,
                     plugin::Variant* pvtVal ) const
{
    CComPtr<IDispatch> dispObj = dynamic_cast<const AxObject*>(pObj)->data();
    CComVariant vtVal;
    if (!bp::ie::getObjectProperty( dispObj, sPropName, vtVal ))
    {
        BPLOG_INFO_STRM( "getObjectProperty( \"" << sPropName << "\" ) " <<
                         "returned false." );
        return false;
    }

    // Set the caller's arg.
    dynamic_cast<AxVariant*>(pvtVal)->data() = vtVal;

    return true;
}


void*
CBPCtl::getWindow() const
{
    return reinterpret_cast<void*>( m_hwndBrowser );
}


// Note our return doesn't discriminate between non-array-hood and
// error during check.
bool
CBPCtl::isJsArray( const plugin::Object* pObj ) const
{
    if (!m_jsfnIsArray)
    {
        if (!bp::ie::makeJsFuncIsArray( m_spLocalBrowser, m_jsfnIsArray ))
        {
            BPLOG_ERROR( "makeJsFuncIsArray() failed!" );
            return false;
        }
    }

    // Invoke the check function.
    CComPtr<IDispatch> dispObj( dynamic_cast<const AxObject*>(pObj)->data() );
    CComVariant vtRet;
    if (!bp::ie::invokeJsFunction( m_jsfnIsArray, CComVariant( dispObj ), 
                                   vtRet ))
    {
        BPLOG_ERROR( "invokeJsFunction() failed!" );
        return false;
    }

    return vtRet.boolVal == VARIANT_TRUE;
}

std::string
CBPCtl::getUserAgent() const
{
    string sUserAgent;
    bool bStat = bp::ie::getUserAgent( m_spLocalBrowser, sUserAgent );

    // TODO: not sure I like this way for the plugin to indicate an error.
    // Most other bp::plugin methods have a way to explicitly indicate error.
    // Leave it this way for now.
    return bStat ? sUserAgent : "unknown";
}


// Note our return doesn't discriminate between non-function-hood and
// error during check.
bool
CBPCtl::isJsFunction( const plugin::Object* pObj ) const
{
    if (!m_jsfnIsFunction)
    {
        if (!bp::ie::makeJsFuncIsFunction( m_spLocalBrowser, m_jsfnIsFunction ))
        {
            BPLOG_ERROR( "makeJsFuncIsFunction() failed!" );
            return false;
        }
    }

    // Invoke the check function.
    CComPtr<IDispatch> dispObj( dynamic_cast<const AxObject*>(pObj)->data() );
    CComVariant vtRet;
    if (!bp::ie::invokeJsFunction( m_jsfnIsFunction, CComVariant( dispObj ),
                                   vtRet ))
    {
        BPLOG_ERROR( "invokeJsFunction() failed!" );
        return false;
    }

    return vtRet.boolVal == VARIANT_TRUE;
}


////////////////////////////////////////////////////////////////////////////////
// ATL Support
//



////////////////////////////////////////////////////////////////////////////////
// Internal Methods
//
bool
CBPCtl::FetchBrowserWnd( IWebBrowser2* pWebBrowser2,
                         HWND& hwndBrowser )
{
    // For IE7, IWebBrowser2::get_HWND will return handle to top-level window,
    // not handle to current tab.  We (probably) need the latter.
    // See msdn doc for IWebBrowser2::get_HWND, currently at:
    //   http://msdn2.microsoft.com/en-us/library/aa752126.aspx
/*    
    long lwnd = NULL;
    HRESULT hr = pWebBrowser2->get_HWND( &lwnd );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "pWebBrowser2->get_HWND() failed!", hr );
        return false;
    }

    // Set caller's arg.
    hwndBrowser = HWND(lwnd);
*/

    // Sequence is:
    //   IWebBrowser2->IServiceProvider->SShellBrowser->IOleWindow->HWND
    if (!pWebBrowser2)
    {
        BPLOG_ERROR( "pWebBrowser2 == 0" );
        return false;
    }
    
    CComPtr<IServiceProvider> isp = NULL;
    HRESULT hr;
    hr = pWebBrowser2->QueryInterface( IID_IServiceProvider, 
                                       reinterpret_cast<void**>(&isp) );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query webbrowser2 for service provider failed!", hr );
        return false;
    }
    
    CComPtr<IOleWindow> iow = NULL;
    hr = isp->QueryService( SID_SShellBrowser, 
                            IID_IOleWindow,
                            reinterpret_cast<void**>(&iow) );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query service provider for IOleWindow failed!", hr );
        return false;
    }

    HWND hwnd = NULL;
    hr = iow->GetWindow( &hwnd );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "iow->GetWindow() failed!", hr );
        return false;
    }
    
    // Set caller's arg.
    hwndBrowser = hwnd;

    return true;
}


bool
CBPCtl::FetchLocalBrowser( IOleClientSite* pClientSite,
                           CComPtr<IWebBrowser2>& browser )
{
    // See http://support.microsoft.com/kb/257717/.

    // For "local" browser pointer, sequence is:
    //   IOleClientSite->IOleContainer->IServiceProvider->
    //     SWebBrowserApp->IWebBrowser2

    if (!pClientSite)
    {
        BPLOG_ERROR( "pClientSite == 0" );
        return false;
    }

    CComPtr<IOleContainer> ioc = NULL;
    HRESULT hr = pClientSite->GetContainer(&ioc);
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "pClientSite->GetContainer() failed!", hr );
        return false;
    }

    CComPtr<IServiceProvider> isp = NULL;
    hr = ioc->QueryInterface( IID_IServiceProvider,
                              reinterpret_cast<void**>(&isp) );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query container for service provider failed!", hr );
        return false;
    }

    CComPtr<IWebBrowser2> brsr = NULL;
    hr = isp->QueryService( SID_SWebBrowserApp,
                            IID_IWebBrowser2,
                            reinterpret_cast<void**>(&brsr) );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query service provider for IWebBrowser2 failed!", hr );
        return false;
    }

    // Set the caller's arg.
    browser = brsr;

    return true;
}


bool
CBPCtl::FetchTopBrowser( IOleClientSite* pClientSite,
                         CComPtr<IWebBrowser2>& browser )
{
    // See http://support.microsoft.com/kb/257717/.

    // For "top-level" browser pointer, sequence is:
    //   IOleClientSite->IOleContainer->IServiceProvider->
    //     STopLevelBrowser->IServiceProvider->SWebBrowserApp->IWebBrowser2
    
    if (!pClientSite)
    {
        BPLOG_ERROR( "pClientSite == 0" );
        return false;
    }

    CComPtr<IOleContainer> ioc = NULL;
    HRESULT hr = pClientSite->GetContainer(&ioc);
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "pClientSite->GetContainer() failed!", hr );
        return false;
    }
    
    CComPtr<IServiceProvider> isp = NULL;
    hr = ioc->QueryInterface(IID_IServiceProvider,
                             reinterpret_cast<void**>(&isp));
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query container for service provider failed!", hr );
        return false;
    }

    CComPtr<IServiceProvider> isp2 = NULL;
    hr = isp->QueryService(SID_STopLevelBrowser,
                           IID_IServiceProvider,
                           reinterpret_cast<void**>(&isp2));
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query service provider for TopLevelBrowser failed!", hr );
        return false;
    }

    CComPtr<IWebBrowser2> brsr = NULL;
    hr = isp2->QueryService(SID_SWebBrowserApp,
                            IID_IWebBrowser2,
                            reinterpret_cast<void**>(&brsr));
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "Query TopLevelBrowser for IWebBrowser2 failed!", hr );
        return false;
    }

    // Set the caller's arg.
    browser = brsr;
    
    return true;
}


bool
CBPCtl::HandleBrowserAttach( IOleClientSite* pClientSite )
{
    BPLOG_FUNC_SCOPE;
    
    // Squirrel away the client site pointer in case we need it later.
    m_pClientSite = pClientSite;

    if (!FetchLocalBrowser( m_pClientSite, m_spLocalBrowser ))
    {
        BPLOG_ERROR( "FetchLocalBrowser() failed!" );
        return false;
    }

    if (!FetchTopBrowser( m_pClientSite, m_spTopBrowser ))
    {
        BPLOG_ERROR( "FetchTopBrowser() failed!" );
        return false;
    }

    if (!FetchBrowserWnd( m_spTopBrowser, m_hwndBrowser ))
    {
        BPLOG_ERROR( "FetchBrowserWnd() failed!" );
        return false;
    }

    // Notify drop manager.
    AxDropManager::handleBrowserAttach( m_spLocalBrowser );

    // Startup new session with the daemon.
    m_pSession = new BPSession( this );

    return true;
}


bool
CBPCtl::HandleBrowserDetach()
{
    BPLOG_FUNC_SCOPE;
    
    // Kill our daemon session.
    delete m_pSession;
    m_pSession = 0;
    
    // We're getting disconnected.
    // Notify drop manager.
    AxDropManager::handleBrowserDetach();

    m_pClientSite = 0;
    m_spLocalBrowser = NULL;
    m_spTopBrowser = NULL;
    m_hwndBrowser = NULL;

    // Clear out our cached js function pointers.
    m_jsfnIsArray = 0;
    m_jsfnIsFunction = 0;

    return true;
}


