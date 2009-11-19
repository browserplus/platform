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
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"
#include "HTMLRender/HTMLDialog_Windows.h"
#include "HTMLRender/ScriptGateway_Windows.h"
#include "ScriptableConfigObject.h"



using namespace bp::html;


// Create global module object required by atl.
[module(name="ConfigPanel")];
CAppModule _Module;


int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE /*hinstPrev*/,
                      LPSTR /*szCmdLine*/, int /*nCmdShow*/ )
{
    HRESULT hRes = ::CoInitialize(NULL);
    ATLASSERT(SUCCEEDED(hRes));
    AtlInitCommonControls(ICC_BAR_CLASSES);	

    hRes = _Module.Init(NULL, hInst);
    ATLASSERT(SUCCEEDED(hRes));

    AtlAxWinInit();

    {
        bp::file::Path path = bp::paths::getPreferencePanelUIPath(
                                bp::localization::getUsersLocale() );
        if (path.empty())
        {
            // TODO: localize
            MessageBoxW( NULL, 
                         L"The required resource files were not found.  "
                         L"Unable to start control panel.",
                         L"BrowserPlus Control Panel",
                         MB_OK );
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
        ScriptGateway gateway( *sco.getScriptableObject(), "BPState" );
        std::string title;
        bp::localization::getLocalizedString(
            "configPanelTitle", bp::localization::getUsersLocale(), title);
        HtmlDialog dlg( title, path.url(), hIcon, &gateway );

        dlg.DoModal();
    }

    _Module.Term();
    ::CoUninitialize();

    return 0;
}

