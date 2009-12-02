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
 * PendingUpdateCache - an abstraction around the disk based cache
 *                      of pending updates
 */

#ifndef __PENDINGUPDATECACHE_H__
#define __PENDINGUPDATECACHE_H__

#include "CoreletQuery.h"
#include "DistQueryTypes.h"
//#include "BPUtils/BPUtils.h"

namespace PendingUpdateCache {
    // enumerate all currently cached corelets
    std::list<bp::service::Summary> cached();

    // unpack and save a downloaded corelet to the cache
    bool save(std::string name, std::string version,
              const std::vector<unsigned char> & buf);

    // purge all corelets from the cache
    bool purge();

    // install a corelet from the cache 
    bool install(std::string name, std::string version);

    bool isCached(std::string name, std::string version);

    bool getSummary(std::string name, std::string version,
                    bp::service::Summary & summary);
};

#endif
