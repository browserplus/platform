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

#include "api/Utils.h"
#include <shlobj.h>
#include "BPUtils/bperrorutil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpmd5.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/OS.h"
#include "RegUtils_Windows.h"

using namespace std;
using namespace bp::paths;
using namespace bp::strutil;
using namespace bp::error;
using namespace bp::install;
using namespace bp::registry;
namespace bpf = bp::file;


bpf::Path bp::install::utils::kUglyNpapiDir("C:/BrowserPlusPlugins");


bpf::Path
bp::install::utils::getFolderPath(int selector)
{
    wchar_t path[MAX_PATH] = {0};
    HRESULT b = SHGetFolderPathW(NULL, selector, NULL,
                                 SHGFP_TYPE_DEFAULT, path);
    if (b != S_OK) {
        BP_THROW(lastErrorString("unable to get CSIDL " + selector));
    }
    bpf::Path rval(path);
    BPLOG_DEBUG_STRM("getFolderPath(" << selector << ") returns " << rval);
    return rval;
}


std::string
bp::install::utils::axName()
{
    // Note: This string is exposed to the user via the "Manage Add-ons" screen.
    //       It is also used in some registry key names and data.
    return "Yahoo! BrowserPlus";
}


std::string
bp::install::utils::axViProgid()
{
    return "Yahoo.BPCtl";
}


std::string
bp::install::utils::axProgid(const std::string& version)
{
    return axViProgid() + "." + version;
}


std::string
bp::install::utils::axProgid(const bp::ServiceVersion& version)
{
    return axProgid(version.asString());
}


bpf::Path
bp::install::utils::npapiPluginDir(const bpf::Path& dir)
{
    bpf::Path rval = dir;
    bool useAsciiDir = false;
    string utf8 = dir.utf8();
    for (size_t i = 0; i < utf8.length(); i++) {
        if ((unsigned int)utf8[i] > 128) {
            useAsciiDir = true;
            break;
        }
    }
    if (useAsciiDir) {
        string id = bp::md5::hash(utf8);
        rval = kUglyNpapiDir / id;
    }
    return rval;
}


bpf::Path
bp::install::utils::ffx2PluginDir()
{
    bpf::Path rval;
    string osVersion = bp::os::PlatformVersion();
    bool isVistaOrLater = (osVersion.compare("6") >= 0);
    if (!isVistaOrLater) {
        string mozKey = "HKLM\\Software\\Mozilla";
        if (keyExists(mozKey)) {
            vector<Key> keys = subKeys("HKLM\\Software\\Mozilla");
            for (size_t i = 0; i < keys.size(); i++) {
                if (keys[i].path().find("Mozilla Firefox 2") != string::npos) {
                    string extKey = keys[i].fullPath() + "\\extensions";
                    try {
                        rval = bpf::Path(readString(extKey, "Plugins"));
                    } catch(const Exception&) {
                        BPLOG_WARN_STRM("unable to read Plugins from "
                                        << extKey);
                    }
                }
            }
        }
    }
    return rval;
}


bool
bp::install::utils::getControlInfo(const bpf::Path& path,
                                   string& version,
                                   string& typeLibGuid,
                                   string& activeXGuid,
                                   vector<string>& mimeTypes)
{
    bool rval = false;
    try {
        // dig version from path
        string filename = bpf::utf8FromNative(path.filename());
        string name = bpf::utf8FromNative(path.stem());
        size_t start = name.find('_') + 1;
        version = name.substr(start);

        // get typeLibGuid from registry HKCU\Software\Classes\AppID\<filename>
        string key = "HKCU\\Software\\Classes\\AppID\\" + filename;
        if (keyExists(key)) {
            typeLibGuid = readString(key, "AppID");
        }
          
        // get activeXGuid from HKCU\\Yahoo.BPCtl.<version>\CLSID\(Default)
        key = "HKCU\\Software\\Classes\\" + axProgid(version) + "\\CLSID";
        if (keyExists(key)) {
            activeXGuid = readString(key);
        }

        // get mimetypes based on who uses our activeXGuid
        key = "HKCU\\Software\\Classes\\MIME\\Database\\Content Type";
        if (keyExists(key)) {
            vector<Key> keys = subKeys(key);
            for (size_t i = 0; i < keys.size(); i++) {
                string guid;
                try {
                    guid = keys[i].readString("CLSID");
                } catch(const Exception&) {
                    // ok if exception
                }
                if (guid.compare(activeXGuid) == 0) {
                    bpf::Path p(keys[i].path());
                    mimeTypes.push_back(bpf::utf8FromNative(p.filename()));
                }
            }
        }
        rval = true;
    } catch(const Exception& e) {
        BPLOG_WARN_STRM("unable to get info for " << path
                        << ": " << e.what());
        version.clear();
        typeLibGuid.clear();
        activeXGuid.clear();
        mimeTypes.clear();
        rval = false;
    }
    return rval;
}


int
bp::install::utils::registerControl(const vector<string>& vsMimetypes,
                                    const string& sModuleUuid,
                                    const bpf::Path& modulePath,
                                    const string& sCoClassUuid,
                                    const string& sCoClassName,
                                    const string& sViProgid,
                                    const string& sProgid)
{
    try {
        string sRoot = "HKCU\\Software\\Classes";

        //  "RegSetValue","HKCR\AppID\{<sModuleUuid>}\(Default)","Type: REG_SZ, Data: YBPAddon"
        string sKey = sRoot + "\\AppID\\" + sModuleUuid;
        string sData = bpf::utf8FromNative(modulePath.stem());
        writeString(sKey, sData);
        
        //  "RegSetValue","HKCR\AppID\YBPAddon.DLL\AppID","Type: REG_SZ, Data: {<sModuleUuid>}"
        string sFilename = bpf::utf8FromNative(modulePath.filename());
        sKey = sRoot + "\\AppID\\" + sFilename;
        writeString(sKey, "AppID", sModuleUuid);
        
        //  "RegSetValue","HKCR\<sProgid>\(Default)","Type: REG_SZ, Data: CBPCtl Object"
        sKey = sRoot + "\\" + sProgid;
        writeString(sKey, sCoClassName);
        
        //  "RegSetValue","HKCR\<sProgid>\CLSID\(Default)","Type: REG_SZ, Data: {<sCoClassUuid>}"
        sKey = sRoot + "\\" + sProgid + "\\CLSID";
        writeString(sKey, sCoClassUuid);        
        
        //  "RegSetValue","HKCR\Yahoo.BPCtl\(Default)","Type: REG_SZ, Data: CBPCtl Object"
        sKey = sRoot + "\\" + sViProgid;
        writeString(sKey, sCoClassName);

        //  "RegSetValue","HKCR\Yahoo.BPCtl\CLSID\(Default)","Type: REG_SZ, Data: {<sCoClassUuid>}"
        sKey = sRoot + "\\" + sViProgid + "\\CLSID";
        writeString(sKey, sCoClassUuid);
        
        //  "RegSetValue","HKCR\Yahoo.BPCtl\CurVer\(Default)","Type: REG_SZ, Data: <sProgid>"
        sKey = sRoot + "\\" + sViProgid + "\\CurVer";
        writeString(sKey, sProgid);
        
        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\(Default)","Type: REG_SZ, Data: CBPCtl Object"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\AppID","Type: REG_SZ, Data: {<sProgid>}"
        sKey = sRoot + "\\CLSID\\" + sCoClassUuid;
        createKey(sKey);
        writeString(sKey, sCoClassName);
        writeString(sKey, "AppID", sModuleUuid);

        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\ProgID","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\ProgID\(Default)","Type: REG_SZ, Data: <sProgid>"
        sKey = sRoot + "\\CLSID\\" + sCoClassUuid + "\\ProgID";
        createKey(sKey);
        writeString(sKey, sProgid);

        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\VersionIndependentProgID","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\VersionIndependentProgID\(Default)","Type: REG_SZ, Data: Yahoo.BPCtl"
        sKey = sRoot + "\\CLSID\\" + sCoClassUuid + "\\VersionIndependentProgID";
        createKey(sKey);
        writeString(sKey, sViProgid);
        
        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\InprocServer32","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\InprocServer32\(Default)","Type: REG_SZ, Data: <sModulePath>
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\InprocServer32\ThreadingModel","Type: REG_SZ, Data: apartment"
        sKey = sRoot + "\\CLSID\\" + sCoClassUuid + "\\InprocServer32";
        createKey(sKey);
        writeString(sKey, bpf::utf8FromNative(modulePath.external_file_string()));
        writeString(sKey, "ThreadingModel", "apartment");

        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\Control","Desired Access: Read/Write"
        sKey = sRoot + "\\CLSID\\" + sCoClassUuid + "\\Control";
        createKey(sKey);
        
        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\ToolboxBitmap32","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\ToolboxBitmap32\(Default)","Type: REG_SZ, Data: <sModulePath>, 1"

        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\MiscStatus","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\MiscStatus\(Default)","Type: REG_SZ, Data: 0"
        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\MiscStatus\1","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\MiscStatus\1\(Default)","Type: REG_SZ, Data: 131473"


        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\TypeLib","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\TypeLib\(Default)","Type: REG_SZ, Data: {<sModuelUuid>}"

        //  "RegCreateKey","HKCR\CLSID\{<sCoClassUuid>}\Version","Desired Access: Read/Write"
        //  "RegSetValue","HKCR\CLSID\{<sCoClassUuid>}\Version\(Default)","Type: REG_SZ, Data: 1.0"
        sKey = sRoot + "\\CLSID\\" + sCoClassUuid + "\\Version";
        createKey(sKey);
        writeString(sKey, "1.0");

        // foreach mimetype
        //   "RegCreateKey","HKCR\MIME\Database\Content Type\<mimetype>","Desired Access: Read/Write"
        //   "RegSetValue","HKCR\MIME\Database\Content Type\<mimetype>\CLSID", Type: REG_SZ, Data: <sCoClassUuid>"
        for (size_t i = 0; i < vsMimetypes.size(); i++) {
            sKey = sRoot + "\\MIME\\Database\\Content Type\\" + vsMimetypes[i];
            createKey(sKey);
            writeString(sKey, "CLSID", activeXGuid());
        }
    } catch (const Exception& e) {
        BPLOG_WARN_STRM("registerControl failed: " << e.what());
        return 1;
    }
    return 0;
}


int
bp::install::utils::unRegisterControl(const std::vector<std::string>& vsMimetypes,
                                      const string& sModuleUuid,
                                      const bpf::Path& modulePath,
                                      const string& sCoClassUuid,
                                      const string& sViProgid,
                                      const string& sProgid)
{
    int rval = 0;

    if (sProgid.empty() || sViProgid.empty() 
        || sCoClassUuid.empty() || sModuleUuid.empty()
        || modulePath.empty()) {
        BPLOG_ERROR_STRM("invalid empty argument");
        return 1;
    }

    string sRoot = "HKCU\\Software\\Classes";

    // "RegDeleteKey","HKCR\<sProgid>"
    string sKey = sRoot + "\\" + sProgid;
    try {
        recursiveDeleteKey(sKey);
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to delete key " << sKey << ": " << e.what());
        rval = 1;
    }

    // "RegDeleteKey","HKCR\<sViProgid>"
    sKey = sRoot + "\\" + sViProgid;
    try {
        recursiveDeleteKey(sKey);
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to delete key " << sKey << ": " << e.what());
        rval = 1;
    }

    // "RegDeleteKey","HKCR\CLSID\<sCoClassUuid>"
    sKey = sRoot + "\\CLSID\\" + sCoClassUuid;
    try {
        recursiveDeleteKey(sKey);
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to delete key " << sKey << ": " << e.what());
        rval = 1;
    }

    // "RegDeleteKey","HKCR\AppID\<sModuleUuid>"
    sKey = sRoot + "\\AppID\\" + sModuleUuid;
    try {
        recursiveDeleteKey(sKey);
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN_STRM("unable to delete key " << sKey << ": " << e.what());
        rval = 1;
    }

    // "RegDeleteKey","HKCR\AppID\YBPAddon.dll"
    string filename = bpf::utf8FromNative(modulePath.filename());
    if (filename.empty()) {
        BPLOG_WARN_STRM("bp::file::fileName(" << modulePath
                        << ") is empty");
    } else {
        sKey = sRoot + "\\AppID\\" + filename;
        try {
            recursiveDeleteKey(sKey);
        } catch (const bp::error::Exception& e) {
            BPLOG_WARN_STRM("unable to delete key " << sKey << ": " << e.what());
            rval = 1;
        }
    }

    // "RegDeleteKey","HKCR\MIME\Database\Content Type\<mimetype>
    for (size_t i = 0; i < vsMimetypes.size(); i++) {
        sKey = sRoot + "\\MIME\\Database\\Content Type\\" + vsMimetypes[i];
        try {
            deleteKey(sKey);
        } catch (const bp::error::Exception& e) {
            BPLOG_WARN_STRM("unable to delete key " << sKey << ": " << e.what());
            rval = 1;
        }
    }

    return rval;
}


int
bp::install::utils::unregisterCruftControls(bool force)
{
    int rval = 0;

    Key appIdKey("HKCU\\Software\\Classes\\AppID");
    vector<Key> subkeys = appIdKey.subKeys();
    for (size_t i = 0; i < subkeys.size(); i++) {
        string keyPath = subkeys[i].fullPath();
        vector<string> vec = bp::strutil::split(keyPath, "\\");
        if (vec.size() < 5) {
            BPLOG_DEBUG_STRM("hrm, " << keyPath << " split into "
                             << vec.size() << " pieces, skipping");
            continue;
        }
        const string& edge = vec[vec.size() - 1];
        size_t startIndex = edge.find("YBPAddon_");
        size_t endIndex = edge.find(".dll");
        if (startIndex != string::npos && endIndex != string::npos
            && startIndex < endIndex) {
            size_t versionIndex = edge.find("_") + 1;
            string versionStr = edge.substr(versionIndex, endIndex - versionIndex);
            bp::ServiceVersion version;
            if (!version.parse(versionStr)) {
                BPLOG_DEBUG_STRM(versionStr << " not a valid version, skipping");
                continue;
            }
            bpf::Path path =  getProductDirectory(version.majorVer(),
                                                  version.minorVer(),
                                                  version.microVer())
                              / "Plugins" / edge;
            if (!force && bpf::exists(path)) {
                continue;
            }

            // kill the control
            BPLOG_DEBUG_STRM("try to remove registry cruft for version " 
                             << versionStr << ", key = " << keyPath);
            string vers, typeLibGuid, activeXGuid;
            vector<string> mtypes;
            if (getControlInfo(path, vers, typeLibGuid, activeXGuid, mtypes)) {
                if (unRegisterControl(mtypes, typeLibGuid, path, activeXGuid,
                                      axViProgid(), axProgid(vers)) != 0) {
                    BPLOG_WARN_STRM("unable to unregister " << path);
                    rval = 1;
                }

                // Remove "supress activex nattergram" entry and vista
                // daemon elevation gunk, not fatal if it fails
                if (force || activeXGuid != utils::activeXGuid()) {
                    try {
                        bool isVistaOrLater = bp::os::PlatformVersion().compare("6") >= 0;
                        recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\"
                                           + activeXGuid);
                        if (isVistaOrLater) {
                            recursiveDeleteKey("HKCU\\SOFTWARE\\Microsoft\\Internet Explorer\\Low Rights\\ElevationPolicy\\"
                                               + activeXGuid);
                        }
                    } catch (const Exception& e) {
                        BPLOG_WARN_STRM("unable to remove nattergram/uac registry entries for "
                                        << activeXGuid << ": " << e.what());
                        rval = 1;
                    }
                }
            }
        }
    }
    return rval;
}


void
bp::install::utils::recursiveDeleteKey(const string& key)
{
    BPLOG_DEBUG_STRM("recursively delete key " << key);
    if (keyExists(key)) {
        vector<Key> keys = subKeys(key);
        for (size_t i = 0; i < keys.size(); i++) {
            recursiveDeleteKey(keys[i].fullPath());
        }
        deleteKey(key);
    }
}
