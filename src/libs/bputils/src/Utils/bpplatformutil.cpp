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

#include <string>
#include "BPUtils/bpplatformutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/ProcessLock.h"
#include "BPUtils/BPLog.h"

using namespace std;
using namespace bp::file;
using namespace bp::paths;

void
bp::platformutil::removePlatform(const bp::ServiceVersion& version,
                                 bool force)
{
    BPLOG_DEBUG_STRM("removePlatform(" << version.asString()
                     << ", " << force << ")");
    int major = version.majorVer();
    int minor = version.minorVer();
    int micro = version.microVer();
    bp::ProcessLock lock = NULL;
    
    if (!force) {
        // Don't try to clean up an installed platform
        if (bp::file::exists(getBPInstalledPath(major, minor, micro))) {
            BPLOG_DEBUG_STRM(version.asString() << " installed, ignored");
            return;
        }

        // Don't try to clean up an installing platform
        if (bp::file::exists(getBPInstallingPath(major, minor, micro))) {
            BPLOG_DEBUG_STRM(version.asString() << " installing, ignored");
            return;
        }

        // Don't try to cleanup a running platform.  We determine if 
        // platform is running by trying a non-blocking acquire of its
        // lock.  If we get it, platform isn't running.
        string lockName = getIPCLockName(major, minor, micro);
        lock = bp::acquireProcessLock(false, lockName);
        if (lock == NULL) {
            // platform running
            BPLOG_DEBUG_STRM(version.asString() << " running, ignored");
            return;
        } 
    }

    // nuke away
    BPLOG_DEBUG_STRM(version.asString() << " being nuked");
    (void) remove(getProductDirectory(major, minor, micro));
    vector<Path> plugins = getPluginPaths(major, minor, micro);
    for (size_t i = 0; i < plugins.size(); i++) {
        (void) remove(plugins[i]);
    }

    if (lock) {
        bp::releaseProcessLock(lock);
    }
}

