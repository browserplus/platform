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
 * DiskScanner.h - a utility for scanning the disk and detecting the
 *                 available services.
 */

#ifndef __DISKSCANNER_H__
#define __DISKSCANNER_H__

#include <map>

#include "BPUtils/bpfile.h"
#include "platform_utils/ServiceDescription.h"
#include "platform_utils/ServiceSummary.h"

namespace DiskScanner {
    
    // scan the disk, building a full set of the available services
    // along with their descriptions.  
    std::map<bp::service::Summary, bp::service::Description>
        scanDiskForServices(
            const bp::file::Path & path,
            std::map<bp::service::Summary, bp::service::Description> lastScan,
            std::set<bp::service::Summary> running,
            const std::string & logLevel,
            const bp::file::Path & logFile);
};

#endif 
