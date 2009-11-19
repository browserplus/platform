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
 * A simple proxy abstraction which runs the Installer on a separate
 * thread.
 */

#ifndef __INSTALLER_RUNNER_H__
#define __INSTALLER_RUNNER_H__

#include "BPInstaller/BPInstaller.h"
//#include <tr1/memory>
#include "BPUtils/bptr1.h"
#include "BPUtils/bpfile.h"


class InstallerRunner : public bp::install::IInstallerListener,
                        public bp::thread::HoppingClass,
                        public std::tr1::enable_shared_from_this<InstallerRunner>

{
  public:
    InstallerRunner();
    virtual ~InstallerRunner();
    // this proxy class re-uses the listener from bp::install
    void setListener(bp::install::IInstallerListener * listener);
                
    // allocate the underlying installer and get it running
    void start(const bp::file::Path& dir, bool deleteWhenDone = false);

  private:
    std::tr1::shared_ptr<bp::install::Installer> m_installer;

    // implementation of the IInstallerListener interface
    // these functions are invoked on the installer thread
    virtual void onStatus(const std::string& msg);
    virtual void onError(const std::string& msg);
    virtual void onProgress(unsigned int pct);

    // entry point for messages from the installer thread.
    // this function is invoked on the client's thread
    virtual void onHop(void * context);

    bp::thread::Thread m_installerThread;
    bp::install::IInstallerListener * m_listener;
};

#endif
