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

#include "api/Uninstaller.h"
#include <shlobj.h>
#include "api/Utils.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/OS.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bplocalization.h"
#include "RegUtils_Windows.h"

using namespace std;
using namespace bp::paths;
using namespace bp::error;
using namespace bp::install;
using namespace bp::install::utils;
using namespace bp::registry;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;


namespace bp {
namespace install {

Uninstaller::Uninstaller() : m_error(false)
{
    m_dir = bpf::getTempPath(bpf::getTempDirectory(), "BrowserPlusUninstaller");
    try {
        bfs::create_directories(m_dir);
    } catch(const bpf::tFileSystemError&) {
        BP_THROW("unable to create " + m_dir.externalUtf8());
    }  
}


Uninstaller::~Uninstaller() 
{
    BPLOG_DEBUG_STRM("remove " << m_dir);
    if (!bpf::remove(m_dir)) {
        BPLOG_WARN(lastErrorString("unable to delete " + m_dir.externalUtf8()));
    }
}


void
Uninstaller::run(bool fromRunonce)
{
    BPLOG_DEBUG("begin uninstall");

    std::string osVersion = bp::os::PlatformVersion();
    bool isVistaOrLater = (osVersion.compare("6") >= 0);

    // remove IE plugin and daemon registry gunk
    bpf::Path topDir = getProductTopDirectory();
    if (bfs::is_directory(topDir)) {
        bpf::tDirIter topEnd;
        for (bpf::tDirIter topIter(topDir); topIter != topEnd; ++topIter) {
            // Unregister our IE plugins
            bpf::Path pluginsDir = topIter->path() / "Plugins";
            if (bfs::is_directory(pluginsDir)) {
                bpf::tDirIter pluginEnd;
                for (bpf::tDirIter pluginIter(pluginsDir); pluginIter != pluginEnd; ++pluginIter) {
                    // IE
                    bpf::Path plugin(pluginIter->path());
                    if (plugin.utf8().find("YBPAddon_") != string::npos) {
                        string version, typeLibGuid, activeXGuid;
                        vector<string> mtypes;
                        if (getControlInfo(plugin, version, typeLibGuid,
                                           activeXGuid, mtypes)) {
                            if (unRegisterControl(mtypes, typeLibGuid, 
                                                  plugin, activeXGuid,
                                                  "CBPCtl Object", "Yahoo.BPCtl",
                                                  "Yahoo.BPCtl." + version) != 0) {
                                BPLOG_WARN_STRM("unable to unregister " << plugin);
                                m_error = true;
                            }
                        }

                        // Remove "supress activex nattergram" entry and vista daemon elevation gunk
                        recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
                                           + activeXGuid);
                        if (isVistaOrLater) {
                            recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\"
                                               + activeXGuid);
                        }
                    }
                }
            }
        }
    }

    // NPAPI may be in a non-standard place due to firefox and 
    // non-ascii usernames.  The registry knows all...
    string mozKey = "HKCU\\SOFTWARE\\MozillaPlugins";
    if (keyExists(mozKey)) {
        vector<Key> keys = subKeys(mozKey);
        for (size_t i = 0; i < keys.size(); i++) {
            if (keys[i].path().find("@yahoo.com/BrowserPlus,version=") != string.npos) {
                bpf::Path path;
                try {
                    path = bpf::Path(keys[i].readString("Path"));
                    BPLOG_DEBUG_STRM("remove " << path);
                    if (!bpf::remove(path)) {
                        BPLOG_DEBUG_STRM(lastErrorString("unable to delete " + path.externalUtf8()));
                        m_error = true;
                    }
                    // deal with ugly npapi dir if it exists
                    if (bfs::is_directory(kUglyNpapiDir)) {
                        removeDirIfEmpty(path.parent_path());
                        removeDirIfEmpty(kUglyNpapiDir);
                    }
                } catch(const Exception& e) {
                    BPLOG_WARN_STRM("unable to read Path from " << keys[i].fullPath()
                                    << ", " << e.what());
                }
                recursiveDeleteKey(keys[i].fullPath());
            }
        }
    }

    // Remove NPAPI plugin for those folks who have firefox2 (we used to
    // support it).  
    bpf::Path ffx2Dir = ffx2PluginDir();
    if (bfs::is_directory(ffx2Dir)) {
        bpf::tDirIter end;
        for (bpf::tDirIter it(ffx2Dir); it != end; ++it) {
            bpf::Path plugin(it->path());
            if (plugin.utf8().find("npybrowserplus_") == 0) {
                BPLOG_DEBUG_STRM("remove " << plugin);
                if (!remove(plugin)) {
                    BPLOG_WARN_STRM("unable to delete " << plugin);
                    m_error = true;
                }
            }
        }
    }

    // remove start menu stuff
    string locale = bp::localization::getUsersLocale();
    string productName;
    bp::localization::getLocalizedString("productNameShort",
                                         locale, productName);
    bpf::Path path = getFolderPath(CSIDL_PROGRAMS) / productName;
    BPLOG_DEBUG_STRM("remove " << path);
    if (!bpf::remove(path)) {
        BPLOG_WARN(lastErrorString("unable to delete " + path.externalUtf8()));
        m_error = true;
    }

    // remove non-localized start menu stuff
    path = getFolderPath(CSIDL_PROGRAMS) / "Yahoo! BrowserPlus";
    BPLOG_DEBUG_STRM("remove " << path);
    if (!bpf::remove(path)) {
        BPLOG_WARN(lastErrorString("unable to delete " + path.externalUtf8()));
        m_error = true;
    }

    // remove vista plugin writable dir
    if (isVistaOrLater) {
        path = getFolderPath(CSIDL_LOCAL_APPDATA);
        path = path.parent_path() / "LocalLow" / "Yahoo!" / "BrowserPlus";
        BPLOG_DEBUG_STRM("remove " << path);
        if (!bpf::remove(path)) {
            BPLOG_WARN(lastErrorString("unable to delete " + path.externalUtf8()));
            m_error = true;
        }
        removeDirIfEmpty(path.parent_path());
    }


    // remove add/remove programs entry
    deleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Yahoo! BrowserPlus");

    // Runonce script from install/update
    deleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce\\RemoveOldBrowserPlus");

    // Remove platform, must be last.
    BPLOG_DEBUG_STRM("remove " << topDir);
    if (!bpf::remove(topDir)) {
        BPLOG_WARN(lastErrorString("unable to delete " + topDir.externalUtf8()));
        m_error = true;
    }
    removeDirIfEmpty(topDir.parent_path());

#ifdef NOTDEF
    // To solve the problem of the windows uninstaller being unable to
    // completely remove browserplus (YIB-2892424):
    //   - add args to BrowserPlusUninstaller to specify
    //     "headless" and "fromRunonce"
    //      - if headless, no gui alerts
    //      - if fromRunonce, invoke Uninstaller::run(true)
    //   - make sure that installer removes the runonce key
    //     set by scheduleRunonce

    // if we hit a problem, try a headless uninstall at next login
    if (m_error && !fromRunonce) {
        scheduleRunonce();
    }
#endif

    BPLOG_DEBUG("complete uninstall");
}


void
Uninstaller::removeDirIfEmpty(const bpf::Path& dir)
{
    if (dir.utf8().compare("C:") == 0) {
        // be paranoid
        return;
    }
    if (bfs::is_directory(dir) && bfs::is_empty(dir)) {
        BPLOG_DEBUG_STRM("remove " << dir);
        if (!bpf::remove(dir)) {
            BPLOG_WARN(lastErrorString("unable to delete " + dir.externalUtf8()));
            m_error = true;
        }
    }
}


void
Uninstaller::scheduleRunonce()
{
#ifdef NOTDEF
    // Schedule a runonce to try uninstall again.
    bpf::Path exePath = getUninstallerPath();
    bpf::Path runoncePath = getTempDirectory() / exePath.filename();
    (void) bpf::remove(runoncePath);
    if (bpf::copy(exePath, runoncePath)) {
        writeString("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                    "BrowserPlusUninstall", 
                    runoncePath.externalUtf8() + " -headless -fromRunonce");
    } else {
        BPLOG_WARN_STRM("unable to copy " << exePath
                        << " -> " << runoncePath);
    }
#endif
}

}
}
