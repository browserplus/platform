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

/*
 *  ProductPaths.cpp
 *  Provides access to bp product-specific paths, etc.
 *
 *  Created by David Grigsby on 8/01/07.
 *  Based on code by Ashit Gandhi and Gordon Durand
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include <sstream>
#include <list>

#include "ProductPaths.h"
#include "bplocalization.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpsemanticversion.h"
#include "BPUtils/BPLog.h"
#include "InstallID.h"

using namespace std;
using namespace bp::file;
namespace bfs = boost::filesystem;


// common code to create a top-level product dir
static bfs::path
doGetTopDir(const string& s)
{
    bfs::path rval(bp::paths::getProductTopDirectory());
    rval /= s;
    return rval;
}


string 
bp::paths::getCompanyName()
{
    return "Yahoo!";
}
 
 
string
bp::paths::getProductName()
{
    return "BrowserPlus";
}


bfs::path
bp::paths::getProductDirectory(int major,
                               int minor,
                               int micro)
{
    bfs::path rval = getProductTopDirectory();

    // append version
    if (major == -1 && minor == -1 && micro == -1) {
        // cmake defines these
        major = BP_VERSION_MAJOR;
        minor = BP_VERSION_MINOR;
        micro = BP_VERSION_MICRO; 
    }
    return doGetTopDir(versionString(major, minor, micro));
}


boost::filesystem::path
bp::paths::getProductTempDirectory()
{
    return doGetTopDir("Temp");
}

    
bfs::path
bp::paths::getPermissionsDirectory()
{
    return doGetTopDir("Permissions");
}


bfs::path
bp::paths::getServiceDirectory()
{
    return doGetTopDir("Corelets");
}


bfs::path
bp::paths::getServiceCacheDirectory()
{
    return doGetTopDir("PendingUpdateCache");
}


bfs::path
bp::paths::getServiceInterfaceCachePath()
{
    return doGetTopDir("CoreletInterfaceCache");
}


bfs::path
bp::paths::getPlatformCacheDirectory()
{
    return doGetTopDir("PlatformUpdateCache");
}


bfs::path
bp::paths::getDaemonPath(int major,
                         int minor,
                         int micro)
{
    bfs::path p = getProductDirectory(major, minor, micro) / "BrowserPlusCore";
    return bp::file::absoluteProgramPath(p);
}


bfs::path
bp::paths::getDaemonLogPath(int major,
                             int minor,
                             int micro)
{
    bfs::path path = getObfuscatedWritableDirectory(major, minor, micro) / "BrowserPlusCore.log";
    return path;
}


bfs::path
bp::paths::getRunnerPath(int major,
                         int minor,
                         int micro)
{
    bfs::path p = getProductDirectory(major, minor, micro) / "BrowserPlusService";
    return bp::file::absoluteProgramPath(p);
}


bfs::path
bp::paths::getServiceInstallerPath(int major,
                                   int minor,
                                   int micro)
{
    bfs::path p = getProductDirectory(major, minor, micro) / "ServiceInstaller";
    return bp::file::absoluteProgramPath(p);
}


bfs::path
bp::paths::getUninstallerPath()
{
    bfs::path p = getProductTopDirectory() / "BrowserPlusUninstaller";
    return bp::file::absoluteProgramPath(p);
}


bfs::path
bp::paths::getServiceDataDirectory(string name,
                                   unsigned int major_ver)
{
    bfs::path serviceDataDir(getProductTopDirectory());
    stringstream ss;
    ss << major_ver;
    serviceDataDir /= bfs::path("CoreletData")/name/ss.str();
    return serviceDataDir;
}


bfs::path
bp::paths::getObfuscatedWritableDirectory(int major,
                                          int minor,
                                          int micro)
{
    bfs::path dir(getPluginWritableDirectory(major, minor, micro));
    string sInstallId = bp::plat::getInstallID();
    if (sInstallId.empty()) {
        return dir;
    }
    dir /= sInstallId;
    return dir;
}


bfs::path
bp::paths::getConfigFilePath(int major,
                             int minor,
                             int micro)
{
    bfs::path path= getProductDirectory(major, minor, micro) / "BrowserPlus.config";
    return path;
}


bfs::path
bp::paths::getLocalizedStringsPath(int major,
                                   int minor,
                                   int micro,
                                   bool useUpdateCache)
{
    bfs::path path;
    if (!useUpdateCache) {
        path = getProductDirectory(major, minor, micro);
    } else {
        path = getPlatformCacheDirectory();
        bp::SemanticVersion v;
        v.setMajor(major);
        v.setMinor(minor);
        v.setMicro(micro);
        path /= bfs::path(v.asString()) / "daemon";
    }
    path /= "strings.json";
    return path;
}


bfs::path
bp::paths::getPersistentStatePath(int major,
                                  int minor,
                                  int micro)
{
    bfs::path path = getPluginWritableDirectory(major, minor, micro) / "bpstate.data";
    return path;
}


bfs::path
bp::paths::getCertFilePath()
{
    bfs::path certPath = getPermissionsDirectory() / "BrowserPlus.crt";
    return certPath;
}


bfs::path
bp::paths::getPermissionsPath()
{
    bfs::path path = getPermissionsDirectory() / "Permissions";
    return path;
}


bfs::path
bp::paths::getDomainPermissionsPath()
{
    bfs::path path = getPermissionsDirectory() / "domainPermissions.json";
    return path;
}


bfs::path
bp::paths::getInstallIDPath()
{
    bfs::path path = getPluginWritableDirectory().parent_path() / "InstallID";
    return path;
}


string
bp::paths::versionString(int major,
                         int minor,
                         int micro)
{
    if (major == -1 && minor == -1 && micro == -1) {
        // cmake defines these
        major = BP_VERSION_MAJOR;
        minor = BP_VERSION_MINOR;
        micro = BP_VERSION_MICRO; 
    }
    stringstream sstr;
    sstr << major << '.' << minor << '.' << micro;
    return sstr.str();
}


bfs::path
bp::paths::getBPInstalledPath(int major,
                              int minor,
                              int micro)
{
    bfs::path path = getProductDirectory(major, minor, micro) / ".installed";
    return path;
}


bfs::path
bp::paths::getBPInstallingPath(int major,
                               int minor,
                               int micro)
{
    bfs::path path = getProductDirectory(major, minor, micro) / ".installing";
    return path;
}


bfs::path
bp::paths::getBPDisabledPath(int major,
                             int minor,
                             int micro)
{
    bfs::path path = getProductDirectory(major, minor, micro) / ".disabled";
    return path;
}


bfs::path
bp::paths::getComponentInstallDialogPath(const string & locale)
{
    bfs::path rval;
    vector<string> locales = 
        bp::localization::getLocaleCandidates(locale);
    
    for (unsigned int i = 0; i < locales.size(); i++) {
        bfs::path path = getProductDirectory()/"ui"/"install_dialog"/locales[i]/"index.html";
        if (pathExists(path)) {
            rval = path;
            break;
        }
    }
    return rval;
}


bfs::path
bp::paths::getPreferencePanelUIPath(const string & locale)
{
    bfs::path rval;
    vector<string> locales = 
        bp::localization::getLocaleCandidates(locale);
    
    for (unsigned int i = 0; i < locales.size(); i++) {
        bfs::path path = getProductDirectory()/"ui"/"preference_panel"/locales[i]/"index.html";
        if (pathExists(path)) {
            rval = path;
            break;
        }
    }
    return rval;
}


void
bp::paths::createDirectories(int major,
                             int minor,
                             int micro) 
{
    try {
        bfs::create_directories(getProductTopDirectory());
        bfs::create_directories(getProductDirectory(major, minor, micro));
        bfs::create_directories(getProductTempDirectory());
        bfs::create_directories(getPermissionsDirectory());
        bfs::create_directories(getServiceDirectory());
        bfs::create_directories(getServiceCacheDirectory());
        bfs::create_directories(getServiceInterfaceCachePath());
        bfs::create_directories(getProductTopDirectory() / "CoreletData");
        bfs::create_directories(getPluginWritableDirectory(major, minor, micro));
        bfs::create_directories(getObfuscatedWritableDirectory(major, minor, micro));
        bfs::create_directories(getPlatformCacheDirectory());
    } catch (const bfs::filesystem_error& e) {
        string msg = "unable to create " + e.path1().string() + ": " + e.what();
        BP_THROW_FATAL(msg);
    }
}
