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

#ifndef __INSTALLERSKIN__
#define __INSTALLERSKIN__

#include <string>

class IInstallerSkinListener
{
  public:
    virtual ~IInstallerSkinListener() { }
    
    // the skin will invoke this function to continue installation after
    // startUp().  This pause allows us to present the user with some
    // language before begining the installation
    virtual void beginInstall() = 0;

    // the skin may invoke this function to cancel the installation
    // (instead of beginInstall)
    virtual void cancelInstallation() = 0;

    // the skin will invoke this function to shut down the application after
    // allDone() is called.  This pause allows us to message something
    // else to the end user.
    virtual void shutdown() = 0;
};

/**
 * A base class installer skin.  The skin handles interaction with
 * the user and is provided hooks and events to display to the user
 * progress and status.
 */
class InstallerSkin 
{
  public:
    InstallerSkin();
    virtual ~InstallerSkin();    

    void setListener(IInstallerSkinListener * listener);

    // the InstallManager invokes this call on the installer output
    // skin at before the installation begins.  This is an opportunity
    // for the skin to present the user with whatever interface is
    // required.  The skin should call IInstallerSkinListener::beginInstall()
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

    // percentage of progress, between 0 and 100, 0 means we haven't yet
    // started, 100 means we've finished
    virtual void progress(int pct);

    // called when installation is complete
    virtual void allDone();

    // called right before exit
    virtual void ended() { }
    
  protected:
    IInstallerSkinListener * m_listener;
};

#endif
