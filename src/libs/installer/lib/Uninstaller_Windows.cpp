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

#include "api/Uninstaller.h"
#include <shlobj.h>
#include "api/Utils.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/OS.h"
#include "platform_utils/bplocalization.h"
#include "platform_utils/ProductPaths.h"
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

Uninstaller::Uninstaller(const bpf::Path& logFile,
                         bp::log::Level logLevel)
    : m_logFile(logFile), m_logLevel(logLevel), m_error(false)
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

    // Remove all services
    removeServices();

    // remove IE plugin and daemon registry gunk
    bpf::Path topDir = getProductTopDirectory();
    if (bpf::isDirectory(topDir)) {
        bpf::tDirIter topEnd;
        for (bpf::tDirIter topIter(topDir); topIter != topEnd; ++topIter) {
            // Unregister our IE plugins
            bpf::Path pluginsDir = topIter->path() / "Plugins";
            if (bpf::isDirectory(pluginsDir)) {
                bpf::tDirIter pluginEnd;
                for (bpf::tDirIter pluginIter(pluginsDir); pluginIter != pluginEnd;
                     ++pluginIter) {
                    // IE
                    bpf::Path plugin(pluginIter->path());
                    if (plugin.utf8().find("YBPAddon_") != string::npos) {
                        string version, typeLibGuid, activeXGuid;
                        vector<string> mtypes;
                        if (getControlInfo(plugin, version, typeLibGuid,
                                           activeXGuid, mtypes)) {
                            if (unRegisterControl(mtypes, typeLibGuid, 
                                                  plugin, activeXGuid,
                                                  axViProgid(),
                                                  axProgid(version)) != 0) {
                                BPLOG_WARN_STRM("unable to unregister " << plugin);
                                m_error = true;
                            }

                            // Remove "supress activex nattergram" entry and vista
                            // daemon elevation gunk, not fatal if it fails
                            try {
                                recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
                                                   + activeXGuid);
                                if (isVistaOrLater) {
                                    recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\"
                                                       + activeXGuid);
                                }
                            } catch (const Exception& e) {
                                BPLOG_WARN_STRM("unable to remove nattergram/uac registry entries for "
                                                << activeXGuid << ": " << e.what());
                                m_error = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // Just in case we have cruft from earlier installs/updates
    if (unregisterCruftControls(true) != 0) {
        m_error = true;
    }

    // NPAPI may be in a non-standard place due to firefox and 
    // non-ascii usernames.  The registry knows all...
    string mozKey = "HKCU\\SOFTWARE\\MozillaPlugins";
    try {
        if (keyExists(mozKey)) {
            vector<Key> keys = subKeys(mozKey);
            for (size_t i = 0; i < keys.size(); i++) {
                if (keys[i].path().find("@yahoo.com/BrowserPlus,version=") != string.npos) {
                    try {
                        // find path to npapi and remove it.
                        // make sure that path is in top dir or ugly dir
                        bpf::Path path = bpf::Path(keys[i].readString("Path"));
                        if (path.string().find(topDir.string()) != 0
                            && path.string().find(kUglyNpapiDir.string()) != 0) {
                            BPLOG_WARN_STRM("registry path to npapi was unexpected: "
                                            << path);
                            m_error = true;
                            continue;
                        }
                        BPLOG_DEBUG_STRM("remove " << path);
                        if (!bpf::remove(path)) {
                            BPLOG_DEBUG_STRM(lastErrorString("unable to delete "
                                                             + path.externalUtf8()));
                            m_error = true;
                        }
                        // deal with ugly npapi dir if it exists
                        if (bpf::isDirectory(kUglyNpapiDir)) {
                            removeDirIfEmpty(path.parent_path());
                            removeDirIfEmpty(kUglyNpapiDir);
                        }

                        // remove keys
                        recursiveDeleteKey(keys[i].fullPath());
                    } catch(const Exception& e) {
                        BPLOG_WARN_STRM("error removing " << keys[i].fullPath()
                                        << ": " << e.what());
                        m_error = true;
                    }
                }
            }
        }
    } catch (const Exception& e) {
        BPLOG_WARN_STRM("failed to get registry entries for " << mozKey
                        << ":  " << e.what());
        m_error = true;
    }

    // Remove NPAPI plugin for those folks who have firefox2 (we used to
    // support it).  
    bpf::Path ffx2Dir = ffx2PluginDir();
    if (bpf::isDirectory(ffx2Dir)) {
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

    string guid = controlPanelGuid();
    try {
        string key = string("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\")
            + "Explorer\\ControlPanel\\NameSpace\\" + guid;
        recursiveDeleteKey(key);
    } catch (const Exception& e) {
        BPLOG_WARN(e.what());
        m_error = true;
    }
    try {
        string key = "HKCU\\SOFTWARE\\Classes\\CLSID\\" + guid;
        recursiveDeleteKey(key);
    } catch (const Exception& e) {
        BPLOG_WARN(e.what());
        m_error = true;
    }

    // remove non-localized start menu stuff
    bpf::Path path = getFolderPath(CSIDL_PROGRAMS) / "Yahoo! BrowserPlus";
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
    try {
        deleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Yahoo! BrowserPlus");
    } catch (const Exception& e) {
        BPLOG_WARN_STRM("failed to get remove add/remove programs entry " << e.what());
        m_error = true;
    }

    // remove temp dir
    bpf::Path tempDir = bpf::getTempDirectory();
    if (tempDir.filename() == bpf::Path("BrowserPlus")) {
        (void) bpf::remove(tempDir);
    }

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
    if (bpf::isDirectory(dir) && bfs::is_empty(dir)) {
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
    // XXX: have installer remove this entry!!
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
