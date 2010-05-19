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

#ifndef __INSTALLERSKINGUI__
#define __INSTALLERSKINGUI__

#include "InstallerSkin.h"
#include "ScriptableInstallerObject.h"
#include "BPUtils/bpfile.h"
#include <string>

/**
 * A graphical installer skin that handles spawning a window which
 * renders HTML.
 */
class InstallerSkinGUI : public InstallerSkin
{
  public:
    // instantiate a GUI skin, passing the path to the ui files.
    // GUIs will look for an index.html inside said directory.
    InstallerSkinGUI(const bp::file::Path & uiDirectory);
    virtual ~InstallerSkinGUI();

    // the InstallManager invokes this call on the installer output
    // skin at before the installation begins.  This is an opportunity
    // for the skin to present the user with whatever interface is
    // required.  The skin should call IInstallerSkinListener::carryOn()
    // to continue the installation based on interaction with the
    // user, or IInstallerSkinListener::cancelInstallation() to stop
    // the install.
    virtual void startUp(unsigned int width, unsigned int height,
                         std::string title);
    
    // a localized (eventually) status message for the end user
    virtual void statusMessage(const std::string & sMsg);

    // a localized (eventually) error message for the end user
    virtual void errorMessage(const std::string & sMsg);

    // an english debug message for the end user (geeks only)
    virtual void debugMessage(const std::string & sMsg);

    // called by the InstallerManager when install is complete.
    virtual void allDone();

    virtual void progress(int pct) { m_sio.setProgress(pct); }

    // absolute last thing called by the InstallerManager
    void ended();

  private:
    bp::file::Path m_uiDirectory;

    ScriptableInstallerObject m_sio;
};

#endif
