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
static Path
doGetTopDir(const string& s)
{
    Path rval(bp::paths::getProductTopDirectory());
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


Path 
bp::paths::getProductDirectory(int major,
                               int minor,
                               int micro)
{
    Path rval = getProductTopDirectory();

    // append version
    if (major == -1 && minor == -1 && micro == -1) {
        // cmake defines these
        major = BP_VERSION_MAJOR;
        minor = BP_VERSION_MINOR;
        micro = BP_VERSION_MICRO; 
    }
    return doGetTopDir(versionString(major, minor, micro));
}


bp::file::Path
bp::paths::getProductTempDirectory()
{
    return doGetTopDir("Temp");
}

    
Path
bp::paths::getPermissionsDirectory()
{
    return doGetTopDir("Permissions");
}


Path
bp::paths::getServiceDirectory()
{
    return doGetTopDir("Corelets");
}


Path
bp::paths::getServiceCacheDirectory()
{
    return doGetTopDir("PendingUpdateCache");
}


Path
bp::paths::getServiceInterfaceCachePath()
{
    return doGetTopDir("CoreletInterfaceCache");
}


Path
bp::paths::getPlatformCacheDirectory()
{
    return doGetTopDir("PlatformUpdateCache");
}


Path
bp::paths::getDaemonPath(int major,
                         int minor,
                         int micro)
{
    Path p = getProductDirectory(major, minor, micro) / "BrowserPlusCore";
    return bp::file::canonicalProgramPath(p);
}


Path
bp::paths::getRunnerPath(int major,
                         int minor,
                         int micro)
{
    Path p = getProductDirectory(major, minor, micro) / "BrowserPlusService";
    return bp::file::canonicalProgramPath(p);
}


Path
bp::paths::getUninstallerPath()
{
    Path p = getProductTopDirectory() / "BrowserPlusUninstaller";
    return bp::file::canonicalProgramPath(p);
}


Path
bp::paths::getServiceLogPath()
{
    Path path = getProductTopDirectory() / "ServiceInstallLog.txt";
    return path;
}


Path
bp::paths::getServiceDataDirectory(string name,
                                   unsigned int major_ver)
{
    Path serviceDataDir(getProductTopDirectory());
    stringstream ss;
    ss << major_ver;
    serviceDataDir /= Path("CoreletData")/name/ss.str();
    return serviceDataDir;
}


Path
bp::paths::getObfuscatedWritableDirectory(int major,
                                          int minor,
                                          int micro)
{
    Path dir(getPluginWritableDirectory(major, minor, micro));
    string sInstallId = bp::plat::getInstallID();
    if (sInstallId.empty()) {
        return dir;
    }
    dir /= sInstallId;
    return dir;
}


Path
bp::paths::getConfigFilePath(int major,
                             int minor,
                             int micro)
{
    Path path= getProductDirectory(major, minor, micro) / "BrowserPlus.config";
    return path;
}


Path
bp::paths::getLocalizedStringsPath(int major,
                                   int minor,
                                   int micro,
                                   bool useUpdateCache)
{
    Path path;
    if (!useUpdateCache) {
        path = getProductDirectory(major, minor, micro);
    } else {
        path = getPlatformCacheDirectory();
        bp::SemanticVersion v;
        v.setMajor(major);
        v.setMinor(minor);
        v.setMicro(micro);
        path /= Path(v.asString()) / "daemon";
    }
    path /= "strings.json";
    return path;
}


Path
bp::paths::getPersistentStatePath(int major,
                                  int minor,
                                  int micro)
{
    Path path = getPluginWritableDirectory(major, minor, micro) / "bpstate.data";
    return path;
}


Path
bp::paths::getCertFilePath()
{
    Path certPath = getPermissionsDirectory() / "BrowserPlus.crt";
    return certPath;
}


Path
bp::paths::getPermissionsPath()
{
    Path path = getPermissionsDirectory() / "Permissions";
    return path;
}


Path
bp::paths::getDomainPermissionsPath()
{
    Path path = getPermissionsDirectory() / "domainPermissions.json";
    return path;
}


Path
bp::paths::getInstallIDPath()
{
    Path path = getPluginWritableDirectory().parent_path() / "InstallID";
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


Path
bp::paths::getBPInstalledPath(int major,
                              int minor,
                              int micro)
{
    Path path = getProductDirectory(major, minor, micro) / ".installed";
    return path;
}


Path
bp::paths::getBPInstallingPath(int major,
                               int minor,
                               int micro)
{
    Path path = getProductDirectory(major, minor, micro) / ".installing";
    return path;
}


Path
bp::paths::getBPDisabledPath(int major,
                             int minor,
                             int micro)
{
    Path path = getProductDirectory(major, minor, micro) / ".disabled";
    return path;
}


Path
bp::paths::getComponentInstallDialogPath(const string & locale)
{
    Path rval;
    vector<string> locales = 
        bp::localization::getLocaleCandidates(locale);
    
    for (unsigned int i = 0; i < locales.size(); i++) {
        Path path = getProductDirectory()/"ui"/"install_dialog"/locales[i]/"index.html";
        if (exists(path)) {
            rval = path;
            break;
        }
    }
    return rval;
}


Path
bp::paths::getPreferencePanelUIPath(const string & locale)
{
    Path rval;
    vector<string> locales = 
        bp::localization::getLocaleCandidates(locale);
    
    for (unsigned int i = 0; i < locales.size(); i++) {
        Path path = getProductDirectory()/"ui"/"preference_panel"/locales[i]/"index.html";
        if (exists(path)) {
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
    } catch (const tFileSystemError& e) {
        string msg = "unable to create " + Path(e.path1()).utf8()
                     + ": " + e.what();
        BP_THROW_FATAL(msg);
    }
}
