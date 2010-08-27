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
#include "BPUtils/ProductPaths.h"
#include "Controller.h"

bp::file::Path
ServiceRunner::determineProviderPath(const bp::service::Summary & s,
                                     std::string & err)
{
    std::string name = s.usesService();
    bp::SemanticVersion version = s.usesVersion();
    bp::SemanticVersion minversion = s.usesMinversion();

    // build directory where versions of this service should be
    bp::file::Path d = bp::paths::getServiceDirectory() / name;

    if (!bp::file::isDirectory(d)) {
        err.append("No such service installed: ");
        err.append(name);
        return bp::file::Path();
    }

    // list all subdirectories
    bp::SemanticVersion winner;
    try {
        bp::file::tDirIter end;
        for (bp::file::tDirIter it(d); it != end; ++it) 
        {
            bp::file::Path p(it->path().filename());
            bp::SemanticVersion current;
            if (!current.parse(p.utf8()))
            {
                std::cerr << "skipping bogus version dir: " << p.utf8()
                << std::endl;
                continue;
            }
            
            if (bp::SemanticVersion::isNewerMatch(current, winner,
                                                  version, minversion))
            {
                winner = current;
            }
        }
    } catch (bp::file::tFileSystemError& e) {
        std::cerr << "unable to iterate thru " 
            << d.externalUtf8() << ": " << e.what();
    }
    if (winner.majorVer() >= 0) {
        d /= winner.asString();
        return d;
    }
    
    err.append("no satisfying service installed");
    return bp::file::Path();
}
