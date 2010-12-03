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

#include <string>
#include <vector>
#include "BPUtils/bpsemanticversion.h"
#include "BPUtils/bpfile.h"

namespace bp {
namespace install {
namespace utils {
    
    boost::filesystem::path getFolderPath(int selector);

    void readPlatformInfo(const boost::filesystem::path& path);

    std::vector<std::string> mimeTypes();
    std::vector<bp::SemanticVersion> installedVersions();

#ifdef WIN32
    std::string activeXGuid();
    std::string typeLibGuid();
    std::string controlPanelGuid();
#endif

#ifdef WIN32
    // Return coclass name of our activex control.
    std::string axName();
    
    // Return version-independent progid of our activex control.
    std::string axViProgid();

    // Return progid of our activex control.
    std::string axProgid(const std::string& version);
    std::string axProgid(const bp::SemanticVersion& version);
    
    // Return dir into which to install npapi plugin.
    // For users with non-ascii usernames, Firefox can't
    // find the npapi plugin if it is installed under their
    // home dir.  Apparently Firefox can't handle the path,
    // although Safari can.  So, if "dir" has a non-ascii char,
    // this returns a dir under kUglyNpapiDir, else it returns
    // "dir".  Ugly as sin, but allows us to work with Firefox 
    // and non-ascii usernames.
    //
    boost::filesystem::path npapiPluginDir(const boost::filesystem::path& dir);
    extern boost::filesystem::path kUglyNpapiDir;  // C:/BrowserPlusPlugins

    // Return directory for Firefox 2 plugins.  Firefox 2
    // doesn't grok the "MozillaPlugins" registry stuff.
    // We no longer support Firefox 2, so this is just
    // here to allow deal with removing/replacing legacy installs.
    boost::filesystem::path ffx2PluginDir();

    bool getControlInfo(const boost::filesystem::path& path,
                        std::string& version,
                        std::string& typeLibGuid,
                        std::string& activeXGuid,
                        std::vector<std::string>& mimeTypes);

    // Returns 0 on success, non-zero otherwise.
    int registerControl(const std::vector<std::string>& vsMimetypes,
                        const std::string& sModuleUuid,
                        const boost::filesystem::path& sModulePath,
                        const std::string& sCoClassUuid,
                        const std::string& sCoClassName,
                        const std::string& sViProgid,
                        const std::string& sProgid);
    
    // Returns 0 on success, non-zero otherwise.
    int unRegisterControl(const std::vector<std::string>& vsMimetypes,
                          const std::string& sModuleUuid,
                          const boost::filesystem::path& sModulePath,
                          const std::string& sCoClassUuid,
                          const std::string& sViProgid,
                          const std::string& sProgid);

    // Unregister "cruft" YBPAddon_xxx.dll controls, meaning
    // controls found in the registry but not on the filesystem.
    // Alas, this can happen because of a bug we had.  If "force"
    // is true, removal will happen even if control is found
    // on the filesystem (used during uninstall).
    // Returns 0 on success, non-zero otherwise
    int unregisterCruftControls(bool force);

    // Delete a registry key and all of its subkeys.
    //
    // BE VERY, VERY CAREFUL USING THIS METHOD, IT CAN DESTROY A SYSTEM.
    // CHECK, DOUBLE CHECK, AND THEN TRIPLE CHECK THE KEY THAT YOU PASS.
    //   
    void recursiveDeleteKey(const std::string& key);
#endif

}}}
