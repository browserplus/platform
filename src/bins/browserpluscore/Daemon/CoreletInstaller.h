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

/**
 * CoreletInstaller, singleton responsible for all corelet installation
 *
 * Created by Lloyd Hilaiel on Fri July 29th 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __CORELETINSTALLER_H__
#define __CORELETINSTALLER_H__

#include "BPUtils/bpfile.h"
#include "CoreletManager/CoreletManager.h"
#include "DistributionClient/DistributionClient.h"


namespace CoreletInstaller
{
    // a listener for corelet installation events
    class IListener 
    {
      public:
        virtual ~IListener() { }
        

        // invoked to deliver download percentage
        virtual void installStatus(unsigned int installId,
                                   const std::string & name,
                                   const std::string & version,
                                   unsigned int pct) = 0;

        // invoked when installation is complete
        virtual void installed(unsigned int installId,
                               const std::string & name,
                               const std::string & version) = 0;

        // invoked when installation fails
        virtual void installationFailed(unsigned int installId) = 0;
    };

    /**
     * Start up the corelet installer.  should occur once per process
     * \param distroServers - a list of urls of available BrowserPlus
     *        distribution web services (i.e http://browserplus.yahoo.com/api). 
     */
    void startup(std::list<std::string> distroServers,
                 std::tr1::shared_ptr<CoreletRegistry> registry);

    /**
     * Shut down the installer.  Should be called once per process.  By
     * the time this function returns, it should be safe to
     * shut down the process without leaving the corelet database in an
     * inconsistent state
     */
    void shutdown();

    /**
     * Enqueue a corelet for installation.  Returns false if that corelet
     * may not be installed (the same version is already installed)
     *
     * \param name - the name of the corelet
     * \param version - string representation of full corelet version
     * \param dir - directory into which to install corelet
     * \param listener - who to tell when the work is done
     *
     * \returns zero on failure, otherwise a transaction id that will be
     *          the same as the eventParamter() of the raised event.
     *          the transaction id allows one listener to disambiguate
     *          multiple installation actions.
     *
     * \note if the corelet is on the installation queue, attempting to
     *       requeue it will succeed you will recieve an event when it is
     *       done.
     */
    unsigned int installCorelet(const std::string & name,
                                const std::string & version,
                                const bp::file::Path & dir,
                                std::tr1::weak_ptr<IListener> listener);
    
    /**
     * Install a corelet from an bpkg buffer.  
     * Corelet is installed into corelet cache.
     *
     * \param name - the name of the corelet
     * \param version - string representation of full corelet version
     * \param buffer - bpkg buffer
     * \param dir - directory in which to install
     * \param listener - who to tell when the work is done
     *
     * \returns zero on failure, otherwise a transaction id that will be
     *          the same as the eventParamter() of the raised event.
     *          the transaction id allows one listener to disambiguate
     *          multiple installation actions.
     */
    unsigned int installCorelet(const std::string & name,
                                const std::string & version,
                                const std::vector<unsigned char> & buffer,
                                const bp::file::Path & dir,
                                std::tr1::weak_ptr<IListener> listener);

    /**
     * Returns whether there is currently work for the installer to do.
     */
    bool isBusy();
};

#endif
