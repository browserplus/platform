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
 *  ProductPaths_Windows.cpp
 *
 *  Created by David Grigsby on 8/01/07.
 *  Based on code by Ashit Gandhi and Gordon Durand
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "api/ProductPaths.h"

#include <atlpath.h>
#include <iostream>
#include <ShellApi.h>
#include <shlobj.h>
#include <sstream>
#include <windows.h>

#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/OS.h"

using namespace std;
using namespace bp::file;

static Path
getCSIDL(int csidl)
{
    wchar_t wcPath[MAX_PATH] = {0};
    HRESULT bStatus = SHGetFolderPathW(NULL,
                                       csidl | CSIDL_FLAG_CREATE,
                                       NULL,
                                       SHGFP_TYPE_DEFAULT,
                                       wcPath);
    if (bStatus != S_OK) {
        stringstream ss;
        ss << "Unable to get Folder Path for CSIDL: " << csidl;
        BP_THROW_FATAL(ss.str());
    }

    Path productDir(wcPath);

    return productDir;
}


Path
bp::paths::getProductTopDirectory()
{
    Path prodDir = getCSIDL(CSIDL_LOCAL_APPDATA) 
                   / getCompanyName() / getProductName();
    return prodDir;
}


Path
bp::paths::getPluginWritableDirectory(int major,
                                      int minor,
                                      int micro)
{
    string osVersion = bp::os::PlatformVersion();
    bool isVistaOrLater = (osVersion.compare("6") >= 0);
    Path productDir;
    if (isVistaOrLater) {
#ifdef NOTDEF
        // Protected mode Internet Explorer can read/write to
        // "LocalLow" in AppData (~\AppData\LocalLow).  See
        // http://msdn2.microsoft.com/en-us/library/bb250462.aspx
        PWSTR path;
        if (SHGetKnownFolderPath(FOLDERID_LocalAppDataLow, KF_FLAG_CREATE,
                                 NULL, &path) == S_OK) {
            productDir /= path;
            CoTaskMemFree(path);
        }
#else
        // TODO: using above requires Vista SDK, so we'll manually
        //       build up the LocalLow path (YIB-1623201)
        // (lth) isn't there a way we can make a runtime switch to use
        //       the vista call if present?
        productDir = getCSIDL(CSIDL_LOCAL_APPDATA);
        productDir.remove_filename();
        productDir /= L"LocalLow";
#endif
    } else {
        productDir = getCSIDL(CSIDL_LOCAL_APPDATA);
    }
    productDir /= Path(getCompanyName())/getProductName()/versionString(major, minor, micro);
    return productDir;
}


string
bp::paths::getIPCName()
{
    string name;
    name.append("BrowserPlus_");
    name.append(versionString());
    name.append("_");
    name.append(bp::os::CurrentUser());
    return name;
}


string
bp::paths::getEphemeralIPCName()
{
    static unsigned int counter = 1;

    stringstream name;
    name << "BrowserPlus_" << versionString() << "_"
         << bp::os::CurrentUser() << "_" << bp::process::currentPid()
         << "_" << counter++;
    return name.str();
}


string
bp::paths::getIPCLockName(int major,
                          int minor,
                          int micro)
{
    string name;
    name.append("Global\\BrowserPlus_");
    name.append(versionString(major, minor, micro));    
    name.append("_");    
    name.append(bp::os::CurrentUser());
    return name;
}


vector<Path> 
bp::paths::getPluginPaths(int major,
                          int minor,
                          int micro)
{
    vector<Path> rval;
    Path pluginDir = getProductDirectory(major, minor, micro);
    pluginDir /= "Plugins";

    Path ax(pluginDir);
    ax /= Path("YBPAddon_" + versionString(major, minor, micro) + ".dll");
    rval.push_back(ax);

    Path npapi(pluginDir);
    npapi /= Path("npybrowserplus_" + versionString(major, minor, micro) + ".dll");
    rval.push_back(npapi);

    return rval;
}
