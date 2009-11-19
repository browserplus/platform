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

/**
 * HTMLRenderTest_Windows.cpp
 * test of html rendering & javascript scripting
 *
 * Created by David Grigsby on 9/05/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */
#include "HTMLRenderTest.h"

#include "HTMLRender/HTMLDialog_Windows.h"
#include "HTMLRender/ScriptGateway_Windows.h"


using namespace bp::html;

// Create global module object required by atl attribute stuff.
[module(name="HTMLRenderTestApp")];
CAppModule _Module;


struct RunnerState
{
    RunnerState( ScriptGateway* pLstnr_, HtmlDialog* pDlg_ ) :
        pLstnr( pLstnr_ ),
        pDlg( pDlg_ )
    {}

    ~RunnerState()
    {
        delete pLstnr;
        delete pDlg;
    }
    
    ScriptGateway* pLstnr;
    HtmlDialog*  pDlg;
};


HTMLRenderTest::JavascriptRunner::JavascriptRunner() :
    m_osSpecific( NULL )
{
}


HTMLRenderTest::JavascriptRunner::~JavascriptRunner()
{
    RunnerState* pState = (RunnerState*) m_osSpecific;

    // Close the dialog.
    pState->pDlg->DestroyWindow();
    
//  delete pState;
}


void HTMLRenderTest::JavascriptRunner::run( const bp::file::Path& path,
                                            ScriptableObject& so,
                                            const std::string& sScriptObjName)
{
    // Create dialog and listener.
    ScriptGateway* pGateway = new ScriptGateway( so, sScriptObjName );
    HtmlDialog* pDlg = new HtmlDialog( "HTMLRenderTest", path.url(), 0, pGateway );

    // Store them in our state variable for later destruction.
    m_osSpecific = new RunnerState( pGateway, pDlg );
    
    // Launch the dialog.
    pDlg->Create( NULL );
}

