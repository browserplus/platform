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
    if (bpf::isDirectory(dir)) {
        BPLOG_INFO_STRM("preflight: delete " << dir);
        (void) remove(dir);
    } else {
        BPLOG_INFO_STRM("preflight: " << dir << " doesn't exist");
    }

    // remove existing non-localized start menu items
    dir = getFolderPath(CSIDL_PROGRAMS) / "Yahoo! BrowserPlus";
    if (bpf::isDirectory(dir)) {
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

    string key;

    // disable nattergrams, not fatal if it fails
    try {
        // IE7
        key = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
               + activeXGuid() + "\\iexplore";
        writeInt(key, "Type", 1);
        writeInt(key, "Flags", 4);
        // IE8
        key = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
              + activeXGuid() + "\\iexplore\\AllowedDomains\\*";
        createKey(key);
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to disable nattergrams: " << e.what());
    }

    vector<bpf::Path> pluginPaths = getPluginPaths(m_version.majorVer(),
                                                   m_version.minorVer(),
                                                   m_version.microVer());
    bpf::Path pluginsDir = pluginPaths[0].parent_path();

    // install IE plugin and register it
    BPLOG_DEBUG("installing IE plugin");
    bpf::Path path = m_dir / "plugins" / "IE";
    doCopy(path, pluginsDir);
    
    bpf::Path iePath = pluginsDir / bpf::Path("YBPAddon_" + m_version.asString()
                                              + ".dll");
    if (registerControl(mimeTypes(), typeLibGuid(), iePath, activeXGuid(),
                        axName(), axViProgid(), axProgid(m_version)) != 0) {
        BP_THROW(lastErrorString("unable to register " + iePath.externalUtf8()));
    }

    // Install NPAPI plugin
    BPLOG_DEBUG("installing NPAPI plugin");
    bpf::Path npapiDestDir = npapiPluginDir(pluginsDir);
    bpf::Path npapiPath = npapiDestDir / bpf::Path("npybrowserplus_"
                                                   + m_version.asString()
                                                   + ".dll");

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


// Make configpanel appear as a control panel
// and just let uninstall come from add/remove programs
//
void
Installer::makeLinks()
{
    BPLOG_DEBUG_STRM("begin Installer::makeLinks()");
    string configName = getString(kConfigLink);
    std::string osVersion = bp::os::PlatformVersion();
    bpf::Path configExe = getProductDirectory(m_version.majorVer(),
                                              m_version.minorVer(),
                                              m_version.microVer()) / "BrowserPlusPrefs.exe";
    try {
        // this registry foo comes from 
        // http://msdn.microsoft.com/en-us/library/cc144195(v=VS.85).aspx,
        // modified to be user-scoped
        string productName = getString(kProductName);
        size_t i = productName.find("&trade;");
        if (i != string::npos) {
            productName = productName.substr(0, i);
        }
        string guid = controlPanelGuid();
        string key = string("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\")
            + "Explorer\\ControlPanel\\NameSpace\\" + guid;
        createKey(key);
        writeString(key, "(Default)", productName);

        key = "HKCU\\SOFTWARE\\Classes\\CLSID\\" + guid;
        createKey(key);
        writeString(key, productName);
        writeString(key, "LocalizedString", productName);
        writeString(key, "InfoTip", configName);
        writeString(key, "System.ApplicationName", "Yahoo.ConfigureBrowserPlus");
        string category = osVersion.compare("6") >= 0 ? "8" : "0";
        writeString(key, "System.ControlPanel.Category", category);

        createKey(key + "\\DefaultIcon");
        writeString(key + "\\DefaultIcon", configExe.externalUtf8() + ",-128");

        createKey(key + "\\Shell\\Open\\Command");
        writeString(key + "\\Shell\\Open\\Command", configExe.externalUtf8());
    } catch (const Exception& e) {
        // ugh, clean up
        BPLOG_WARN(e.what());
        string guid = controlPanelGuid();
        try {
            string key = string("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\")
                + "Explorer\\ControlPanel\\NameSpace\\" + guid;
            recursiveDeleteKey(key);
        } catch (const Exception& e) {
            BPLOG_WARN(e.what());
        }
        try {
            string key = string("HKCU\\SOFTWARE\\Classes\\CLSID\\") + guid;
            recursiveDeleteKey(key);
        } catch (const Exception& e) {
            BPLOG_WARN(e.what());
        }
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
    vector<bp::SemanticVersion> plats = utils::installedVersions();
    vector<bp::SemanticVersion>::const_iterator iter;
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

    // Alas, due to previous bugs (#180 and #192), we may have left registry cruft
    // on previous updates.  We must now atone for our sins and clean it up.
    (void) unregisterCruftControls(false);

    // Re-register our activex control (it could have 
    // been unregistered by above code if guid hadn't changed)
    bpf::Path thisIEPluginPath = prodDir / "Plugins" / bpf::Path("YBPAddon_"
                                                                 + m_version.asString()
                                                                 + ".dll");
    if (registerControl(mimeTypes(), typeLibGuid(), thisIEPluginPath, activeXGuid(),
                        axName(), axViProgid(), axProgid(m_version)) != 0) {
        BP_THROW(lastErrorString("unable to register " + thisIEPluginPath.externalUtf8()));
    }

    // Prevent vista UAC on first daemon launch from IE.  Any old guid will do,
    // so just use our ax guid.  Not fatal if this fails
    std::string osVersion = bp::os::PlatformVersion();
    if (osVersion.compare("6") >= 0) {
        try {
            bpf::Path daemonPath(prodDir);
            key = "HKCU\\Software\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\"
                  + activeXGuid();
            createKey(key);
            writeInt(key, "Policy", 3);
            writeString(key, "AppName", "BrowserPlusCore.exe");
            writeString(key, "AppPath", daemonPath.externalUtf8());
        } catch (const bp::error::Exception& e) {
            BPLOG_WARN_STRM("attempting to prevent UAC: " << e.what());
        }
    }    

    BPLOG_DEBUG_STRM("complete Installer::postflight()");
}


void
Installer::disablePlugins(const bp::SemanticVersion& version)
{
    string osVersion = bp::os::PlatformVersion();
    bool isVistaOrLater = osVersion.compare("6") >= 0;
    string versionStr = version.asString();

    // Disable npapi plugin by zapping the registry, not fatal if it fails
    try {
        string key = "HKCU\\SOFTWARE\\MozillaPlugins\\@yahoo.com/BrowserPlus,version="
                     + versionStr;
        recursiveDeleteKey(key);
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to disable " << versionStr
                        << " NPAPI plugin: " << e.what());
    }

    // Disable IE plugin by zapping the registry, not fatal if it fails.
    // Also note that the plugin path doesn't need to exist for the 
    // registry zapping to work.
    try {
        bpf::Path path =  getProductDirectory(version.majorVer(),
                                              version.minorVer(),
                                              version.microVer()) 
                          / "Plugins" / bpf::Path("YBPAddon_" + versionStr + ".dll");
        // unregister control
        string vers, typeLibGuid, activeXGuid;
        vector<string> mtypes;
        if (getControlInfo(path, vers, typeLibGuid, activeXGuid, mtypes)) {
            if (unRegisterControl(mtypes, typeLibGuid, path, activeXGuid,
                                  axViProgid(), axProgid(vers)) != 0) {
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
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to disable " << versionStr
                        << " IE plugin: " << e.what());
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
