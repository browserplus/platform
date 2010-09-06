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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include "api/Installer.h"
#include "api/Utils.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpfile.h"
#include "platform_utils/ProductPaths.h"

using namespace std;
using namespace bp::error;
using namespace bp::install;
using namespace bp::paths;
using namespace bp::file;
namespace bfs = boost::filesystem;

void 
Installer::preflight()
{
    BPLOG_DEBUG_STRM("Installer::preflight() does nothing");
}


// Copy our plugin to plugins folder
//
void
Installer::installPlugins()
{
    BPLOG_DEBUG_STRM("begin Installer::installPlugins()");
    Path pluginPath = "BrowserPlus_" + m_version.asString()  + ".plugin";
    
    // Get path to plugins
    Path pluginDir = utils::getFolderPath(kInternetPlugInFolderType) / pluginPath;

    // remove existing version of our plugin
    if (!remove(pluginDir)) {
        BPLOG_WARN(lastErrorString("unable to delete " + pluginDir.externalUtf8()));
    }

    // create plugin dir
    try {
        bfs::create_directories(pluginDir);
    } catch(const tFileSystemError&) {
        BP_THROW(lastErrorString("unable to create " + pluginDir.externalUtf8()));
    }

    // copy plugin into plugins dir
    Path src = m_dir / "plugins" / pluginPath;
    doCopy(src, pluginDir);
    BPLOG_DEBUG_STRM("complete Installer::installPlugins()");
}


// Copy our prefpane to prefpanes folder
//
void
Installer::installPrefPanel()
{
    BPLOG_DEBUG_STRM("begin Installer::installPrefPanel()");
    // Get path to PreferencePanels
    Path prefPaneDir = utils::getFolderPath(kPreferencePanesFolderType)
        / "BrowserPlusPrefs.prefPane";

    // remove existing version of our prefpane
    if (!remove(prefPaneDir)) {
        BPLOG_WARN(lastErrorString("unable to delete " + prefPaneDir.externalUtf8()));
    }

    // create dir
    try {
        bfs::create_directories(prefPaneDir);
    } catch(const tFileSystemError&) {
        BP_THROW(lastErrorString("unable to create " + prefPaneDir.externalUtf8()));
    }

    // copy prefpane
    Path src= m_dir / "prefPane" / "BrowserPlusPrefs.prefPane";
    doCopy(src, prefPaneDir);
    BPLOG_DEBUG_STRM("complete Installer::installPrefPanel()");
}


void 
Installer::makeLinks()
{
    BPLOG_DEBUG_STRM("Installer::makeLinks() does nothing");
}


// Just remove old plugins and platforms.
//
void 
Installer::postflight()
{
    BPLOG_DEBUG_STRM("begin Installer::postflight()");

    vector<bp::SemanticVersion> plats = utils::installedVersions();
    vector<bp::SemanticVersion>::const_iterator iter;
    for (iter = plats.begin(); iter != plats.end(); ++iter) {
        if (iter->majorVer() != m_version.majorVer()) {
            continue;
        }
        BPLOG_DEBUG_STRM("examine platform " << iter->asString());
        if (iter->compare(m_version) > 0) {
            // Hmm, a newer platform found.  How did that happen?
            BPLOG_WARN_STRM("trying to install " << m_version.asString()
                            << ", found " << iter->asString()
                            << ", ignoring it");
            continue;
        }
        if (iter->compare(m_version) < 0) {
            removePlatform(*iter);
        }
    }
    BPLOG_DEBUG_STRM("complete Installer::postflight()");
}


void
Installer::disablePlugins(const bp::SemanticVersion& version)
{
    // Must physically remove OSX plugins to keep them from
    // being found by browser
    vector<Path> plugins = getPluginPaths(version.majorVer(),
                                          version.minorVer(),
                                          version.microVer());
    for (size_t i = 0; i < plugins.size(); i++) {
        (void) remove(plugins[i]);
    }
}
