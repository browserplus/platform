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

#include "InstallID.h"
#include "BPUtils/bpuuid.h"
#include "BPUtils/bpstrutil.h"
#include "bpphash.h"
#include "ProductPaths.h"

namespace bp {
namespace plat {


std::string getInstallID()
{
    bp::file::Path path = bp::paths::getInstallIDPath();

    // see if InstallID exists in phash.  If so, migrate it
    // to new InstallID file
    const std::string ksInstallIdPhashKey = "InstallID";
    std::string sId;
    if (bp::phash::get( ksInstallIdPhashKey, sId ))
    {
        (void) bp::strutil::storeToFile( path, sId );
        bp::phash::remove( ksInstallIdPhashKey );
    }

    // Dig installID out of file if it exists.
    // If not, create and persist.
    // intentionally do not log on failure here, leave that
    // to higher level code
    if (!bp::strutil::loadFromFile( path, sId )) {
        if (bp::uuid::generate( sId ))
        {
            (void) bp::strutil::storeToFile( path, sId );
        }
    }

    return sId;
}


} // namespace plat
} // namespace bp

