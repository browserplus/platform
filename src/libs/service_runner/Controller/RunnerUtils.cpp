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

#include "BPUtils/bpfile.h"
#include "platform_utils/ProductPaths.h"
#include "Controller.h"

boost::filesystem::path
ServiceRunner::determineProviderPath(const bp::service::Summary & s,
                                     std::string & err)
{
    std::string name = s.usesService();
    bp::SemanticVersion version = s.usesVersion();
    bp::SemanticVersion minversion = s.usesMinversion();

    // build directory where versions of this service should be
    boost::filesystem::path d = bp::paths::getServiceDirectory() / name;

    if (!bp::file::isDirectory(d)) {
        err.append("No such service installed: ");
        err.append(name);
        return boost::filesystem::path();
    }

    // list all subdirectories
    bp::SemanticVersion winner;
    try {
        boost::filesystem::directory_iterator end;
        for (boost::filesystem::directory_iterator it(d); it != end; ++it) 
        {
            bp::SemanticVersion current;
            if (!current.parse(it->path().filename().string()))
            {
                std::cerr << "skipping bogus version dir: " << it->path().filename()
                          << std::endl;
                continue;
            }
            
            if (bp::SemanticVersion::isNewerMatch(current, winner,
                                                  version, minversion))
            {
                winner = current;
            }
        }
    } catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "unable to iterate thru " << d << ": " << e.what();
    }
    if (winner.majorVer() >= 0) {
        d /= winner.asString();
        return d;
    }
    
    err.append("no satisfying service installed");
    return boost::filesystem::path();
}
