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


#include <shlobj.h>
#include <ShellAPI.h>

#include "api/Installer.h"
#include "api/Utils.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/OS.h"
#include "RegUtils_Windows.h"

using namespace std;
using namespace bp::paths;
using namespace bp::strutil;
using namespace bp::error;
using namespace bp::install;
using namespace bp::install::utils;
using namespace bp::registry;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;


void 
Installer::preflight()
{
    BPLOG_DEBUG_STRM("begin Installer::preflight()");

    // remove existing start menu items
    bpf::Path dir = getFolderPath(CSIDL_PROGRAMS) / getString(kProductNameShort);
    if (bfs::is_directory(dir)) {
        BPLOG_INFO_STRM("preflight: delete " << dir);
        (void) remove(dir);
    } else {
        BPLOG_INFO_STRM("preflight: " << dir << " doesn't exist");
    }

    // remove existing non-localized start menu items
    dir = getFolderPath(CSIDL_PROGRAMS) / "Yahoo! BrowserPlus";
    if (bfs::is_directory(dir)) {
        BPLOG_INFO_STRM("preflight: delete " << dir);
        (void) remove(dir);
    } else {
        BPLOG_INFO_STRM("preflight: " << dir << " doesn't exist");
    }
    BPLOG_DEBUG_STRM("complete Installer::preflight()");
}


// Install IE and NPAPI.
//
void
Installer::installPlugins()
{
    BPLOG_DEBUG_STRM("begin Installer::installPlugins()");

    // IE7
    string key = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
        + activeXGuid() + "\\iexplore";
    writeInt(key, "Type", 1);
    writeInt(key, "Flags", 4);
    // IE8
    key = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
        + activeXGuid() + "\\iexplore\\AllowedDomains\\*";
    createKey(key);

    vector<bpf::Path> pluginPaths = getPluginPaths(m_version.majorVer(),
                                                   m_version.minorVer(),
                                                   m_version.microVer());
    bpf::Path pluginsDir = pluginPaths[0].parent_path();

    // install IE plugin and register it
    BPLOG_DEBUG("installing IE plugin");
    bpf::Path path = m_dir / "plugins" / "IE";
    doCopy(path, pluginsDir);
    
    bpf::Path iePath = pluginsDir / bpf::Path("YBPAddon_" + m_version.asString() + ".dll");
    if (registerControl(mimeTypes(), typeLibGuid(), iePath, activeXGuid(),
                        "CBPCtl Object", "Yahoo.BPCtl",
                        "Yahoo.BPCtl." + m_version.asString()) != 0) {
        BP_THROW(lastErrorString("unable to register " + iePath.externalUtf8()));
    }

    // Install NPAPI plugin
    BPLOG_DEBUG("installing NPAPI plugin");
    bpf::Path npapiDestDir = npapiPluginDir(pluginsDir);
    bpf::Path npapiPath = npapiDestDir / bpf::Path("npybrowserplus_" + m_version.asString() + ".dll");

    bpf::Path npapiSrc = m_dir / "plugins" / "NPAPI";
    doCopy(npapiSrc, npapiDestDir);
    key = "HKEY_CURRENT_USER\\SOFTWARE\\MozillaPlugins\\@yahoo.com/BrowserPlus,version="
        + m_version.asString();
    writeString(key, "Path", npapiPath.externalUtf8());
    writeString(key, "ProductName", "Yahoo! BrowserPlus");
    writeString(key, "Version", m_version.asString());
    writeString(key, "Vendor", "Yahoo! Inc.");
    vector<string> mtypes = mimeTypes();
    for (size_t i = 0; i < mtypes.size(); i++) {
        string subkey = key + "\\MimeTypes\\" + mtypes[i];
        writeString(subkey, "Description", "Yahoo! BrowserPlus");
        writeString(subkey, "Suffixes", "");
    }

    BPLOG_DEBUG_STRM("complete Installer::installPlugins()");
}


// Install preference panel
//
void
Installer::installPrefPanel()
{
    BPLOG_DEBUG_STRM("begin Installer::installPrefPanel()");
    bpf::Path dest = getProductDirectory(m_version.majorVer(),
                                         m_version.minorVer(),
                                         m_version.microVer());
    bpf::Path src = m_dir / "prefPane";
    doCopy(src, dest);
    BPLOG_DEBUG_STRM("complete Installer::installPrefPanel()");
}


// Make start menu links to configpanel, uninstaller 
//
void
Installer::makeLinks()
{
    BPLOG_DEBUG_STRM("begin Installer::makeLinks()");
    bpf::Path dir = getFolderPath(CSIDL_PROGRAMS) / getString(kProductNameShort);
    (void) remove(dir);
    try {
        bfs::create_directories(dir);
    } catch(const bpf::tFileSystemError&) {
        BP_THROW(lastErrorString("unable to create " + dir.externalUtf8()));
    }
    bpf::Path lnk = dir / bpf::Path(getString(kConfigLink) + ".lnk");
    bpf::Path target = getProductDirectory(m_version.majorVer(),
                                           m_version.minorVer(),
                                           m_version.microVer()) / ("BrowserPlusPrefs.exe");
    if (!createLink(lnk, target)) {
        BP_THROW(lastErrorString("unable to create " +lnk.externalUtf8()));
    }
    lnk = lnk.parent_path() / bpf::Path(getString(kUninstallLink) + ".lnk");
    if (!createLink(lnk, getUninstallerPath())) {
        BP_THROW(lastErrorString("unable to create " + lnk.externalUtf8()));
    }
    BPLOG_DEBUG_STRM("begin Installer::makeLinks()");
}


// Cleanup
//
void 
Installer::postflight()
{
    BPLOG_DEBUG_STRM("begin Installer::postflight()");

    // Add/remove programs entry.
    bpf::Path prodDir = getProductDirectory(m_version.majorVer(),
                                            m_version.minorVer(),
                                            m_version.microVer());

    // uninstaller    
    string key = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Yahoo! BrowserPlus";
    bpf::Path topDir = prodDir.parent_path();
    bpf::Path uninstaller = topDir / "BrowserPlusUninstaller.exe";
    bpf::Path icon = topDir / "ybang.ico";
    string displayName = "Yahoo! BrowserPlus " + m_version.asString();
    writeString(key, "DisplayName", displayName);
    writeString(key, "DisplayIcon", icon.externalUtf8());
    writeString(key, "UninstallString", uninstaller.externalUtf8());
    writeString(key, "Publisher", "Yahoo! Inc.");
    writeInt(key, "NoModify", 1);
    writeInt(key, "NoRepair", 1);

    // Remove replaced platforms. 
    vector<bp::ServiceVersion> plats = utils::installedVersions();
    vector<bp::ServiceVersion>::const_iterator iter;
    for (iter = plats.begin(); iter != plats.end(); ++iter) {
        if (iter->majorVer() != m_version.majorVer()) {
            continue;
        }
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

    // Try to delete our dir.  It will likely only partially succeed since
    // some files are open and ntfs can't handle that.
    (void) remove(m_dir);

    // Re-register our activex control (it could have 
    // been unregistered by above code if guid hadn't changed)
    bpf::Path thisIEPluginPath = prodDir / "Plugins" / bpf::Path("YBPAddon_" + m_version.asString() + ".dll");
    if (registerControl(mimeTypes(), typeLibGuid(), thisIEPluginPath, activeXGuid(),
                        "CBPCtl Object", "Yahoo.BPCtl",
                        "Yahoo.BPCtl." + m_version.asString()) != 0) {
        BP_THROW(lastErrorString("unable to register " + thisIEPluginPath.externalUtf8()));
    }

    // Prevent vista UAC on first daemon launch from IE.  Any old guid will do,
    // so just use our ax guid
    std::string osVersion = bp::os::PlatformVersion();
    if (osVersion.compare("6") >= 0) {
        bpf::Path daemonPath(prodDir);
        key = "HKCU\\Software\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\"
            + activeXGuid();
        createKey(key);
        writeInt(key, "Policy", 3);
        writeString(key, "AppName", "BrowserPlusCore.exe");
        writeString(key, "AppPath", daemonPath.externalUtf8());
    }

    BPLOG_DEBUG_STRM("complete Installer::postflight()");
}


void
Installer::disablePlugins(const bp::ServiceVersion& version)
{
    string osVersion = bp::os::PlatformVersion();
    bool isVistaOrLater = (osVersion.compare("6") >= 0);
    string versionStr = version.asString();

    // Disable npapi plugin by zapping the registry
    string key = "HKCU\\SOFTWARE\\MozillaPlugins\\@yahoo.com/BrowserPlus,version=" + versionStr;
    recursiveDeleteKey(key);

    // Disable IE plugin by zapping the registry
    bpf::Path path =  getProductDirectory(version.majorVer(),
                                          version.minorVer(),
                                          version.microVer()) 
                       / "Plugins" / bpf::Path("YBPAddon__" + versionStr + ".dll");
    if (bfs::is_regular(path)) { 
        // unregister control
        string version, typeLibGuid, activeXGuid;
        vector<string> mtypes;
        if (getControlInfo(path, version, typeLibGuid, activeXGuid, mtypes)) {
            if (unRegisterControl(mtypes, typeLibGuid, 
                                  path, activeXGuid,
                                  "CBPCtl Object", "Yahoo.BPCtl",
                                  "Yahoo.BPCtl." + version) != 0) {
                BPLOG_WARN_STRM("unable to unregister " << path);
            }
        }

        // Remove "supress activex nattergram" entry and vista daemon elevation gunk if
        // this plugin has a different activex guid
        if (activeXGuid.compare(utils::activeXGuid()) != 0) {
            recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
                               + activeXGuid);
            if (isVistaOrLater) {
                recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\"
                                   + activeXGuid);
            }
        }
    }

    // Remove legacy ffx2 NPAPI plugin.
    bpf::Path ffx2Dir = utils::ffx2PluginDir();
    if (!ffx2Dir.empty()) {
        bpf::Path path = ffx2Dir / bpf::Path("npybrowserplus_" + versionStr + ".dll");
        if (!bpf::remove(path)) {
            BPLOG_DEBUG_STRM(lastErrorString("unable to delete " + path.externalUtf8()));
        }
    }
}
