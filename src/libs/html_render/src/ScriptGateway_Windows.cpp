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
 *  ScriptGateway_Windows.cpp
 *
 *  Created by David Grigsby on 9/25/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "ScriptGateway_Windows.h"
#include "HTMLScriptableComObject_Windows.h"
#include "BPUtils/bpstrutil.h"

using namespace std;


namespace bp {
namespace html {


// Creates and returns a js method that loads all its arguments into a
// js array and then calls _bpInvoke on the scriptable object, passing
// the method name and the argument array.
static bool createInvokerJsFunc( IWebBrowser2* pBrowser,
                                 const std::string& sComObjName,
                                 const std::string& sMethodName,
                                 CComPtr<IDispatch>& jsfn )
{
    stringstream ssFunc;
    ssFunc  << "[function() {"
            << "  var arr = new Array;"
            << "  arr = arguments;"
            << "  return " << sComObjName << "._bpInvoke( \""
            << sMethodName
            << "\", arr );"
            << "}][0]";

    CComVariant vtRet;
    if (!bp::ie::evaluateScript( pBrowser, ssFunc.str(), vtRet ))
    {
        BPLOG_ERROR( "evaluateScript() failed!" );
        return false;
    }

    jsfn = vtRet.pdispVal;

    return true;
}


ScriptGateway::ScriptGateway( const ScriptableObject& so,
                              const std::string& sScriptObjName ) :
    m_so( so ),
    m_sJsObjName( sScriptObjName ),
    m_sComObjName( "_bpso" ),
    m_browser(),
    m_jsoBPDialog(),
    m_bHaveNavigated( false )
{
    
}


bool ScriptGateway::onBeforeNavigate( HtmlDialog& dlg,
                                      std::string& sUrl )
{
    // For any navigation request other than the first popup a new
    // browser to that location.
    // TODO: this could be an option.
    // Note: DOCHOSTUIFLAG_OPENNEWWIN is supposed to control this
    //       behavior but it isn't currently working.

    if (m_bHaveNavigated)
    {
        // Subsequent nav (e.g. hyperlink click) popup new browser and
        // leave dialog where it is.
        wstring wsUrl = bp::strutil::utf8ToWide( sUrl );
        ShellExecuteW( ::GetActiveWindow(), L"open", wsUrl.c_str(), 
                       NULL, NULL, SW_SHOWNORMAL );
        return false;
    }
    else
    {
        // Initial navigation; allow it.
        m_bHaveNavigated = true;
        return true;
    }
}


void ScriptGateway::onDocumentComplete( HtmlDialog& dlg )
{
    // Cache a pointer to the dialog's browser.
    m_browser = dlg.getBrowser();

    // Hook up scriptable obj to JS dom at window._bpso.
    if (!addScriptableComObj())
    {
        BPLOG_ERROR( "attachScriptableComObj() failed!" );
        return;
    }

    // Add the "BPDialog" js object to the window object.
    if (!addScriptableJsObj())
    {
        BPLOG_ERROR( "addBPDialogObj() failed!" );
        return;
    }

    // Add appropriate methods to JS window.BPDialog.
    // We need a method for each of the "mounted" functions.
    typedef std::map<std::string, ScriptableFunctionHost *> tFuncMap;
    const tFuncMap& fm = m_so.functionMap();
    for (tFuncMap::const_iterator cit = fm.begin(); cit != fm.end(); ++cit)
    {
        if (!addInvoker( cit->first ))
        {
            BPLOG_ERROR( "addInvoker() failed!" );
            return;
        }
    }
}


bool ScriptGateway::addScriptableComObj()
{
    // Create our scriptable com object.
    CComObject<BpScriptableComObject>* pObj = NULL;
    HRESULT hr = CComObject<BpScriptableComObject>::CreateInstance(&pObj);
    if (FAILED(hr) || !pObj)
    {
        BPLOG_COM_ERROR( "CComObject<CScriptAgent>::CreateInstance() failed!",
                         hr );
        return false;
    }

    // Additional construction.
    pObj->setScriptableObject( &m_so );
    pObj->setBrowser( m_browser );
    
    // Get the JS window object.
    CComPtr<IHTMLWindow2> window;
    if (!bp::ie::getDocWindow( m_browser, window ))
    {
        BPLOG_ERROR( "getDocWindow() failed!" );
        return false;
    }

    // Add our com object as a property of top-level JS window object.
    if (!bp::com::addProperty( CComPtr<IDispatch>(window), m_sComObjName, 
                               CComVariant( pObj ) ))
    {
        BPLOG_ERROR( "addPropertyToObj() failed!" );
        return false;
    }

    return true;
}


bool ScriptGateway::addScriptableJsObj()
{
    // Create a new js object.
    CComPtr<IDispatchEx> dexObj;
    if (!bp::ie::createJsObject( m_browser, dexObj ))
    {
        BPLOG_ERROR( "createJsObject() failed!" );
        return false;
    }

    m_jsoBPDialog = dexObj;

    // Add the object to the window as "BPDialog".
    // Get the JS window object.
    CComPtr<IHTMLWindow2> window;
    if (!bp::ie::getDocWindow( m_browser, window ))
    {
        BPLOG_ERROR( "getDocWindow() failed!" );
        return false;
    }

    if (!bp::com::addProperty( CComPtr<IDispatch>(window), m_sJsObjName, 
                               CComVariant( m_jsoBPDialog ) ))
    {
        BPLOG_ERROR( "addProperty() failed!" );
        return false;
    }

    return true;
}


// Creates and adds an "invoker" method of the specified name to our
// scriptable object.
bool ScriptGateway::addInvoker( const std::string& sMethodName )
{
    // Create a js func that will call our _bpInvoke.
    CComPtr<IDispatch> jsfnInvoker;
    if (!createInvokerJsFunc( m_browser, m_sComObjName, sMethodName, 
                              jsfnInvoker ))
    {
        BPLOG_ERROR( "createInvokerJsFunc() failed!" );
        return false;
    }

    // Add the invoker func to window.BPDialog.
    if (!bp::com::addProperty( m_jsoBPDialog, sMethodName, 
                               CComVariant( jsfnInvoker ) ))
    {
        BPLOG_ERROR( "addProperty() failed!" );
        return false;
    }

    return true;
}


} // namespace html
} // namespace bp

