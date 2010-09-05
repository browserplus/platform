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
 *  bpbrowserinfo.h
 *
 *  Parses a user agent string and returns browser info from it
 *
 *  Created by Gordon Durand on 08/19/10
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPBROWSERINFO_H__
#define __BPBROWSERINFO_H__

#include <string>
#include <map>
#include <vector>
#include "BPUtils/bpsemanticversion.h"

namespace bp { 
    class BrowserInfo 
    {
    public:
        // well known capability keys/values
        static const char* kDnDCapability;
        static const char* kFileBrowseCapability;
        static const char* kSupported;
        static const char* kUnsupported;
        static const char* kDnDIntercept;
        static const char* kDnDHtml5;
        static const char* kDnDActiveX;

        BrowserInfo() : m_supported(false) {
        }
        BrowserInfo(const std::string& userAgent);
        virtual ~BrowserInfo() {
        }
        std::string platform() const {
            return m_platform;
        }
        std::string browser() const {
            return m_browser;
        }
        bp::SemanticVersion version() const {
            return m_version;
        }
        bool supported() const;
        std::map<std::string, std::string> capabilities() const;
        std::string capability(const std::string& s) const;
        std::string asString() const {
            return m_platform + "/" + m_browser + "/" + m_version.asString();
        }

    private:
        std::string m_platform;
        std::string m_browser;
        bp::SemanticVersion m_version;
        bool m_supported;
        std::map<std::string, std::string> m_capabilities;

        void loadBrowserSupportInfo();
        std::vector<bp::SemanticVersion> versionRange(const std::string& s) const;
        std::string versionFromUA(const std::string& userAgent,
                                  const std::string& prefix,
                                  const std::string& separator) const;
    };
}

#endif
