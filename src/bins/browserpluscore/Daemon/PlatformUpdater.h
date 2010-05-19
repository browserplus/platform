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
 * PlatformUpdater.h - A singleton responsible for periodic platform update
 *
 * Created by Gordon Durand on Wed March 26th 2008.
 * Copyright (c) 2008-2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __PLATFORMUPDATER_H__
#define __PLATFORMUPDATER_H__

#include <list>
#include <string>

namespace PlatformUpdater
{
    /**
     * Start up the platform updater.  should occur once per process
     * \param distroServers - a list of urls to available BrowserPlus
     *        distribution servers (i.e http://browserplus.yahoo.com/api). 
     * \param poll - number of seconds between checks
     */
    void startup(std::list<std::string> distroServers, unsigned int poll);
    
    /**
     * Spawn a platform update
     */
    bool spawnUpdate(const std::string& version);
    
    /** 
     * Is version already pending?
     */
    bool updateSpawned(const std::string& version);

    /**
     * shut down the platform updater
     */
    void shutdown();

    /**
     * Returns whether we're currently checking for or installing updates.  
     * This should delay daemon shutdown.
     */
    bool isBusy();
};

#endif
