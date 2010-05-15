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
 *  ConfigPanel.cpp
 *
 *  Created by David Grigsby on 10/08/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */

#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>
#include <comutil.h>
#include "BPUtils/bplocalization.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/LogConfigurator.h"
#include "BPUtils/ProductPaths.h"
#include "HTMLRender/HTMLDialog_Windows.h"
#include "HTMLRender/ScriptGateway_Windows.h"
#include "ScriptableConfigObject.h"


using namespace bp::localization;
using namespace std;

// Create global module object required by atl.
[module(name="ConfigPanel")];
CAppModule _Module;


void setupLogging()
{
    bp::log::Configurator cfg;
    cfg.loadConfigFile();
    cfg.setPath(bp::paths::getObfuscatedWritableDirectory()/"ConfigPanel.log");
    cfg.configure();
}



int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE /*hinstPrev*/,
                      LPSTR /*szCmdLine*/, int /*nCmdShow*/ )
{
    HRESULT hRes = ::CoInitialize(NULL);
    ATLASSERT(SUCCEEDED(hRes));
    AtlInitCommonControls(ICC_BAR_CLASSES);	

    hRes = _Module.Init(NULL, hInst);
    ATLASSERT(SUCCEEDED(hRes));

    AtlAxWinInit();

    try
    {
        setupLogging();
        BPLOG_INFO( "Startup." );

        string sLocale = getUsersLocale();
        
        bp::file::Path path = bp::paths::getPreferencePanelUIPath( sLocale );
        if (path.empty())
        {
            // TODO: localize
            MessageBoxW( NULL, 
                         L"The required resource files were not found.  "
                         L"Unable to start control panel.",
                         L"BrowserPlus Control Panel",
                         MB_OK );

            BPLOG_FATAL( "getPreferencePanelUIPath failed.  Exiting." );
            return 0;
        }

        // set icons
        HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
                                         MAKEINTRESOURCE(IDR_MAINFRAME), 
                                         IMAGE_ICON,
                                         ::GetSystemMetrics(SM_CXSMICON),
                                         ::GetSystemMetrics(SM_CYSMICON),
                                         LR_DEFAULTCOLOR);
        
        ScriptableConfigObject sco;
		bp::html::ScriptGateway gateway( *sco.getScriptableObject(), "BPState" );
        string title;
        getLocalizedString( "configPanelTitle", sLocale, title );
        
        bp::html::HtmlDialog dlg( title, path.externalUtf8(), hIcon, &gateway );

        dlg.DoModal();

        BPLOG_INFO( "Normal exit." );
    }
    catch (const std::exception& exc) {
        BP_REPORTCATCH(exc);
    }
    catch (...) {
        BP_REPORTCATCH_UNKNOWN;
    }

    _Module.Term();
    ::CoUninitialize();

    return 0;
}

