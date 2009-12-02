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

#include "BPUtils/BPUtils.h"
#include "api/Utils.h"

using namespace std;
using namespace bp::paths;
using namespace bp::strutil;
using namespace bp::error;
using namespace bp::install;
namespace bpf = bp::file;


static vector<string> s_mimeTypes;
static vector<bp::ServiceVersion> s_installedVersions;
#ifdef WIN32
static string s_activeXGuid;
static string s_typeLibGuid;
#endif


void 
bp::install::utils::readPlatformInfo(const bpf::Path& path)
{
    bp::config::ConfigReader reader;
    if (!reader.load(path)) {
        BP_THROW("Unable to load " + path.externalUtf8());
    }
    list<string> strList;
    if (!reader.getArrayOfStrings("mimeTypes", strList)) {
        BP_THROW("Unable to read mimeTypes from " + path.externalUtf8());
    }
    list<string>::const_iterator it;
    for (it = strList.begin(); it != strList.end(); ++it) {
        if (it->length() > 0) {
            s_mimeTypes.push_back(*it);
        }
    }
#ifdef WIN32
    if (!reader.getStringValue("activeXGuid", s_activeXGuid)) {
        BP_THROW("Unable to read activeXGuid from " + path.externalUtf8());
    }
    if (!reader.getStringValue("typeLibGuid", s_typeLibGuid)) {
        BP_THROW("Unable to read typeLibGuid from " + path.externalUtf8());
    }
#endif

    // determine what other versions of this major rev exist on disk
    bpf::Path dir = getProductTopDirectory();
    if (boost::filesystem::is_directory(dir)) {
        bpf::tDirIter end;
        for (bpf::tDirIter it(dir); it != end; ++it) {
            string s = bpf::utf8FromNative(it->filename());
            bp::ServiceVersion version;
            if (version.parse(s)) {
                bpf::Path installedPath = getBPInstalledPath(version.majorVer(),
                                                             version.minorVer(),
                                                             version.microVer());
                if (boost::filesystem::exists(installedPath)) {
                    s_installedVersions.push_back(version);
                }
            }
        }
    }
}


vector<string> 
bp::install::utils::mimeTypes()
{
    return s_mimeTypes;
}



vector<bp::ServiceVersion> 
bp::install::utils::installedVersions()
{
    return s_installedVersions;
}


#ifdef WIN32
string
bp::install::utils::activeXGuid()
{
    return s_activeXGuid;
}


string
bp::install::utils::typeLibGuid()
{
    return s_typeLibGuid;
}

#endif
