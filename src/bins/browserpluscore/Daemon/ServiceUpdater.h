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
 * ServiceUpdater.h - A singleton responsible for periodic update of 
 *                    recently used services
 *
 * Created by Lloyd Hilaiel on Fri October 18th 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __SERVICEUPDATER_H__
#define __SERVICEUPDATER_H__

#include "ServiceManager/ServiceManager.h"

namespace ServiceUpdater
{
    /**
     * Start up the service updater.  should occur once per process
     * \param distroServers - a list of urls to available BrowserPlus
     *        distribution servers (i.e http://browserplus.yahoo.com/api). 
     * \param serviceRegistry - the service registry to use
     * \param pollPeriodSeconds - the maximum frequency at which we
     *                  may check for updates

     */
    void startup(std::list<std::string> distroServers,
                 std::tr1::shared_ptr<ServiceRegistry> serviceRegistry,
                 unsigned int pollPeriodSeconds);

    /**
     * shut down the service updater
     */
    void shutdown();

    /**
     * Returns whether we're currently checking for updates.  
     * This should delay daemon shutdown.
     */
    bool isBusy();
};

#endif
