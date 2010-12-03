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

#include "BPUtils/BPUtils.h"

#include <string>
#include <sstream>
#include <iostream>

InstallerSkinGUI::InstallerSkinGUI(const boost::filesystem::path & uiDirectory)
		: m_uiDirectory(uiDirectory)
{
}

InstallerSkinGUI::~InstallerSkinGUI()
{
}

void
InstallerSkinGUI::startUp(unsigned int width, unsigned int height,
                          std::string title)
{
    // update the listener of our scriptable object so it can
    // callback into the skin listener
    m_sio.setListener(m_listener);
}

void
InstallerSkinGUI::statusMessage(const std::string & s)
{
    m_sio.setStatus(s);
}

void
InstallerSkinGUI::errorMessage(const std::string & s)
{
    m_sio.setError(s, std::string());
}

void
InstallerSkinGUI::debugMessage(const std::string & s)
{
}

void
InstallerSkinGUI::allDone()
{
}

void
InstallerSkinGUI::ended()
{
}
