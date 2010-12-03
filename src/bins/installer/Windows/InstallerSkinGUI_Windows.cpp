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

#include "../InstallerSkinGUI.h"
#include "BPInstaller/BPInstaller.h"
#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>
#include <comutil.h>
#include <string>
#include "HTMLRender/HTMLDialog_Windows.h"
#include "HTMLRender/ScriptGateway_Windows.h"


// Create global module object required by atl attribute stuff.
[module(name="BrowserPlusInstaller")];
CAppModule _Module;

InstallerSkinGUI::InstallerSkinGUI(const boost::filesystem::path & uiDirectory)
: m_uiDirectory(uiDirectory)
{
}

InstallerSkinGUI::~InstallerSkinGUI()
{
}

static bp::html::HtmlDialog * dlg;
static bp::html::ScriptGateway * gateway;

void
InstallerSkinGUI::startUp(unsigned int width, unsigned int height,
                          std::string title)
{
    m_sio.setListener(m_listener);

    gateway = new bp::html::ScriptGateway(*(m_sio.getScriptableObject()),
                                          "BPInstaller");

    boost::filesystem::path path = m_uiDirectory / "index.html";
    BPLOG_DEBUG_STRM("path to ui: " << path);;

    HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(),
                                     MAKEINTRESOURCE(IDR_MAINFRAME), 
                                     IMAGE_ICON,
                                     ::GetSystemMetrics(SM_CXSMICON),
                                     ::GetSystemMetrics(SM_CYSMICON),
                                     LR_DEFAULTCOLOR);
    
    dlg = new bp::html::HtmlDialog(title.c_str(), bp::file::urlFromPath(path), hIcon, gateway);

    dlg->Create( NULL );
    if (! *dlg) {
        BPLOG_ERROR("couldn't create HtmlDialog window!");
    }

    // Handle high DPI on windows:  When our UI is rendered on
    // a windows host with IE8 installed (high dpi aware HTML rendering),
    // then we must scale our container up based on DPI settings.
    // (YIB 2858594)
    float scale = 1.0;

    if (dlg && dlg->dpiAware())
    {
        scale = dlg->dpiScale();
    }

    dlg->show((int)(width * scale), (int)(height * scale));

    return;
}

void
InstallerSkinGUI::statusMessage(const std::string & s)
{
    m_sio.setStatus(s);
}

void
InstallerSkinGUI::errorMessage(const std::string &s)
{
    m_sio.setError(s, std::string());
}

void
InstallerSkinGUI::debugMessage(const std::string &)
{
}

void
InstallerSkinGUI::allDone()
{
    // javascript will call into scriptable object which will invoke
    // shutdown(), ending the program
    //    m_listener->shutdown();
}

void 
InstallerSkinGUI::ended()
{
}
