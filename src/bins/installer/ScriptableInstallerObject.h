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

// ScriptableInstallerObject.  An object which exposes callable functions
// to the javascript running in the installer UI.

#ifndef __SCRIPTABLEINSTALLEROBJECT_H__
#define __SCRIPTABLEINSTALLEROBJECT_H__

#include "HTMLRender/HTMLRender.h"
#include "InstallerSkin.h"

class ScriptableInstallerObject : public bp::html::ScriptableFunctionHost
{
  public:
    ScriptableInstallerObject();
    ~ScriptableInstallerObject();    

    /* set progress, between 0 and 100 */
    void setProgress(int pct);

    /* set a human readable, localized string that describes the current
     * state of the installation for the end user */
    void setStatus(const std::string & status);

    void setListener(IInstallerSkinListener * listener);

    /* Indicate to the end user that the installation failed in error,
     * localized is a localized human readable string, while details
     * is more information about the failure, it is not necesarily
     * localized */
    void setError(const std::string & localized, const std::string & details);

    bp::html::ScriptableObject * getScriptableObject();
  private:
    bp::html::ScriptableObject m_so;

    // dispatch function for when javascript calls into us
    virtual bp::Object * invoke(const std::string & functionName,
                                unsigned int id,
                                std::vector<const bp::Object *> args); 

    IInstallerSkinListener * m_listener;
    int m_progress;

    // the current state of the installation.  will be returned up to
    // javascript when js calls 
    // 
    // state is one of
    // "started"
    // "installing"
    // "complete"
    // "error"
    std::string m_state;

    // a human readable, localized string which offers more information
    // about the current installation state.  This may be shown to the
    // end user during installation
    std::string m_desc;

    // details of the 
    std::string m_errorDetails;

};

#endif
