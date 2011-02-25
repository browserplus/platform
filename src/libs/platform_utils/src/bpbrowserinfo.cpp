/**
 *
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
 *  bpbrowserinfo.cpp
 *
 *  Parses a user agent string and returns browser info from it
 *
 *  Created by Gordon Durand on 08/19/10
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpbrowserinfo.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/OS.h"
#include "ProductPaths.h"

using namespace std;

namespace bp {

const char* BrowserInfo::kDnDCapability = "DnD";
const char* BrowserInfo::kFileBrowseCapability = "FileBrowse";
const char* BrowserInfo::kSupported = "supported";
const char* BrowserInfo::kUnsupported = "unsupported";
const char* BrowserInfo::kDnDIntercept = "intercept";
const char* BrowserInfo::kDnDHtml5 = "html5";
const char* BrowserInfo::kDnDActiveX = "activeX";


BrowserInfo::BrowserInfo(const std::string& userAgent) : m_supported(false)
{
    try {
        // A sample Windows IE8 userAgent
        //    'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C)'
        // A sample Windows Firefox 3.6.3 userAgent
        //    'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3'
        // A sample Windows Firefox 3.6.8 userAgent
        //    'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.8) Gecko/20100722 Firefox/3.6.8 (.NET CLR 3.5.30729; .NET4.0C)'
        // A sample OSX Safari 4.0.3 userAgent that we're snorting thru
        //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6; en-us) AppleWebKit/531.9 (KHTML, like Gecko) Version/4.0.3 Safari/531.9'
        // A sample OSX Chrome userAgent string
        //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-US) AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.38 Safari/533.4'
        // A sample OSX Titanium userAgent string
        //    'Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_3; en-US) AppleWebKit/533.4 (KHTML, like Gecko) Chrome/5.0.375.38 Safari/533.4 Titanium/1.0.0'
        // A sample OSX Opera userAgent string
        //    'Opera/9.80 (Macintosh; Intel Mac OS X 10.6.6; U; en) Presto/2.7.62 Version/11.01'

        // first grab platform
        BPLOG_DEBUG_STRM("userAgent = " << userAgent);
        if (userAgent.find("Windows") != string::npos) {
            m_platform = "Windows";
        } else if (userAgent.find("Mac OS X") != string::npos) {
            m_platform = "OSX";
        } else {
            BP_THROW("unknown platform in '" + userAgent + "'");
        }

        // Now figure out the browser.  Make sure to check
        // Safari last since Chrome and Titanium have
        // "Safari" in their user agents.  Nice.
        string browser;
        if (userAgent.find("MSIE") != string::npos) {
            m_browser = "IE";
        } else if (userAgent.find("Firefox") != string::npos) {
            m_browser = "Firefox";
        } else if (userAgent.find("Chrome") != string::npos) {
            m_browser = "Chrome";
        } else if (userAgent.find("Titanium") != string::npos) {
            m_browser = "Titanium";
        } else if (userAgent.find("Safari") != string::npos) {
            m_browser = "Safari";
        } else if (userAgent.find("Opera") != string::npos) {
            m_browser = "Opera";
        } else {
            BP_THROW("unknown browser in '" + userAgent + "'");
        }
    
        // now figure out the version
        string prefix, separator;
        if (m_browser == "IE") {
            prefix = "MSIE ";
            separator = ";";
        } else if (m_browser == "Firefox") {
            prefix = "Firefox/";
            separator = " ";
        } else if (m_browser == "Safari") {
            prefix = "Version/";
            separator = " ";
        } else if (m_browser == "Chrome") {
            prefix = "Chrome/";
            separator = " ";
        } else if (m_browser == "Opera") {
            prefix = "Version/";
            separator = " ";
        }

        string vstr = versionFromUA(userAgent, prefix, separator);
        bool gotVersion = m_version.parse(vstr);
        if (!gotVersion && m_browser == "Firefox") {
            // Could be a beta firefox
            vstr = versionFromUA(userAgent, prefix, "b");
            gotVersion = m_version.parse(vstr);
        }
        if (!gotVersion) {
            BP_THROW("unable to parse version'" + vstr
                     + "', userAgent = '" + userAgent + "'");
        }

        // now find out what's supported
        loadBrowserSupportInfo();

    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR_STRM("caught " << e.what() << ", clearing BrowserInfo");
        m_supported = false;
        m_platform.clear();
        m_browser.clear();
        m_version = bp::SemanticVersion();
        m_capabilities.clear();
    }
}


bool
BrowserInfo::supported() const
{
    return m_supported;
}


string
BrowserInfo::capability(const std::string& s) const
{
    string rval;
    map<string, string>::const_iterator it = m_capabilities.find(s);
    if (it != m_capabilities.end()) {
        rval = it->second;
    }
    return rval;
}


map<string, string>
BrowserInfo::capabilities() const
{
    return m_capabilities;
}


void
BrowserInfo::loadBrowserSupportInfo()
{
    // not necessarily an error if file not present (e.g. unit tests)
    boost::filesystem::path path = bp::paths::getPermissionsPath();
    boost::system::error_code ec;
    if (!exists(path, ec)) {
        BPLOG_INFO_STRM(path << " does not exist");
        return;
    }

    try {
        string json;
        if (!bp::strutil::loadFromFile(path, json)) {
            BP_THROW("unable to load from " + path.string());
        }
        string err;
        bp::Map* topMapPtr = 
            dynamic_cast<bp::Map*>(bp::Object::fromPlainJsonString(json, &err));
        if (!topMapPtr) {
            BP_THROW("error parsing " + path.string() + ": " + err);
        }

        // get "browserSupport" value
        const bp::Object* objPtr = topMapPtr->value("browserSupport");
        if (!objPtr){
            BP_THROW("no browserSupport key in " + path.string());
        }

        // objPtr now has browser support info, sample json:
        // // browserplus version range
        // "2" : {
        //    // platform
        //    "Windows": {
        //        // browser
        //        "IE": {
        //            // browser version range
        //            "6-8.0": {
        //                // capabilities
        //                "FileBrowse":"supported",
        //                "DnD":"activeX"
        //            }
        //        },
        //        "Firefox": {
        //            "3.0-3.6.2": {
        //                "FileBrowse":"supported",
        //                "DnD":"intercept"
        //            },
        //            "3.6.3-3.6.8": {
        //                "FileBrowse":"supported",
        //                "DnD":"html5"
        //            }
        //        }
        //
        bp::SemanticVersion bpVersion;
        bpVersion.setMajor(BP_VERSION_MAJOR);
        bpVersion.setMinor(BP_VERSION_MINOR);
        bpVersion.setMicro(BP_VERSION_MICRO);
        map<string, const bp::Object*> m = *objPtr;
        map<string, const bp::Object*>::const_iterator it;
        for (it = m.begin(); !m_supported && it != m.end(); ++it) {
            BPLOG_DEBUG_STRM("examine b+ platform "<< it->first);
            vector<bp::SemanticVersion> v = versionRange(it->first);
            if (!bpVersion.withinRange(v[0], v[1])) {
                continue;
            }
            map<string, const bp::Object*> pm = *(it->second);
            map<string, const bp::Object*>::const_iterator pmIt;
            for (pmIt = pm.begin(); !m_supported && pmIt != pm.end(); ++pmIt) {
                BPLOG_DEBUG_STRM("examine platform "<< pmIt->first);
                if (pmIt->first != m_platform) {
                    continue;
                }
                map<string, const bp::Object*> bm = *(pmIt->second);
                map<string, const bp::Object*>::const_iterator bmIt;
                for (bmIt = bm.begin(); !m_supported && bmIt != bm.end(); ++bmIt) {
                    BPLOG_DEBUG_STRM("examine browser "<< bmIt->first);
                    if (bmIt->first != m_browser) {
                        continue;
                    }
                    const map<string, const bp::Object*> vmap = *(bmIt->second);
                    map<string, const bp::Object*>::const_iterator vmIt;
                    for (vmIt = vmap.begin(); !m_supported && vmIt != vmap.end(); ++vmIt) {
                        BPLOG_DEBUG_STRM("examine range "<< vmIt->first);
                        vector<bp::SemanticVersion> v = versionRange(vmIt->first);
                        if (!m_version.withinRange(v[0], v[1])) {
                            continue;
                        }
                        BPLOG_DEBUG_STRM(asString() << " supported");
                        m_supported = true;
                        const map<string, const bp::Object*> cmap = *(vmIt->second);
                        map<string, const bp::Object*>::const_iterator cmIt;
                        for (cmIt = cmap.begin(); cmIt != cmap.end(); ++cmIt) {
                            string v = *(cmIt->second);
                            BPLOG_DEBUG_STRM("add capability " << cmIt->first << " / "<< v);
                            m_capabilities[cmIt->first] = v;
                        }
                    }
                }
            }
        }
    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR_STRM("unable to read browser support: " << e.what());
        m_supported = false;
        m_capabilities.clear();
    }
}


vector<bp::SemanticVersion>
BrowserInfo::versionRange(const std::string& s) const
{
    vector<string> svec;
    // can have a single version or a range of versions
    if (s.find('-') != string::npos) {
        svec = bp::strutil::split(s, "-");
    } else {
        svec.push_back(s);
        svec.push_back(s);
    }
    if (svec.size() != 2) {
        BP_THROW("bad version range: " + s);
    }
    bp::SemanticVersion vmin;
    bp::SemanticVersion vmax;
    if (!vmin.parse(svec[0]) || !vmax.parse(svec[1])) {
        BP_THROW("bad version range: " + s);
    }
    vector<bp::SemanticVersion> rval;
    rval.push_back(vmin);
    rval.push_back(vmax);
    return rval;
}


string
BrowserInfo::versionFromUA(const string& userAgent,
                           const string& prefix,
                           const string& separator) const
{
    string rval;
    size_t start = userAgent.find(prefix);
    if (start == string::npos) {
        BPLOG_DEBUG_STRM("didn't find '" << prefix << "' in " << userAgent);
        return rval;
    }
    start += prefix.length();
    size_t len = string::npos;
    size_t end = userAgent.find(separator, start);
    if (end != string::npos) {
        len = end - start;
    }
    rval = userAgent.substr(start, len);
    return rval;
}


void
BrowserInfo::setCapability(const string& capability,
                           const string& value)
{
    m_capabilities[capability] = value;
}


}

