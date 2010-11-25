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

/**
 * ServiceInstaller, singleton responsible for all service installation
 *
 * Created by Lloyd Hilaiel on Fri July 29th 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __SERVICEINSTALLER_H__
#define __SERVICEINSTALLER_H__

#include "BPUtils/bpfile.h"
#include "ServiceManager/ServiceManager.h"
#include "DistributionClient/DistributionClient.h"


namespace ServiceInstaller
{
    // a listener for service installation events
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
     * Start up the service installer.  should occur once per process
     * \param distroServers - a list of urls of available BrowserPlus
     *        distribution web services (i.e http://browserplus.yahoo.com/api). 
     */
    void startup(std::list<std::string> distroServers,
                 std::tr1::shared_ptr<ServiceRegistry> registry);

    /**
     * Shut down the installer.  Should be called once per process.  By
     * the time this function returns, it should be safe to
     * shut down the process without leaving the service database in an
     * inconsistent state
     */
    void shutdown();

    /**
     * Enqueue a service for installation.  Returns false if that service
     * may not be installed (the same version is already installed)
     *
     * \param name - the name of the service
     * \param version - string representation of full service version
     * \param listener - who to tell when the work is done
     *
     * \returns zero on failure, otherwise a transaction id that will be
     *          the same as the eventParamter() of the raised event.
     *          the transaction id allows one listener to disambiguate
     *          multiple installation actions.
     *
     * \note if the service is on the installation queue, attempting to
     *       requeue it will succeed you will recieve an event when it is
     *       done.
     */
    unsigned int installService(const std::string & name,
                                const std::string & version,
                                std::tr1::weak_ptr<IListener> listener);
    
    /**
     * Install a service from an bpkg buffer.  
     * Service is installed into service cache.
     *
     * \param name - the name of the service
     * \param version - string representation of full service version
     * \param buffer - bpkg buffer
     * \param listener - who to tell when the work is done
     *
     * \returns zero on failure, otherwise a transaction id that will be
     *          the same as the eventParamter() of the raised event.
     *          the transaction id allows one listener to disambiguate
     *          multiple installation actions.
     */
    unsigned int installService(const std::string & name,
                                const std::string & version,
                                const std::vector<unsigned char> & buffer,
                                std::tr1::weak_ptr<IListener> listener);

    /**
     * Returns whether there is currently work for the installer to do.
     */
    bool isBusy();
};

#endif
