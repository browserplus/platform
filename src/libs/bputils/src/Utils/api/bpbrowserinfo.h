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

#include "bpserviceversion.h"

namespace bp { 
    class BrowserInfo 
    {
    public:
        BrowserInfo(const std::string& userAgent);
        virtual ~BrowserInfo() {
        }
        std::string platform() const {
            return m_platform;
        }
        std::string browser() const {
            return m_browser;
        }
        bp::ServiceVersion version() const {
            return m_version;
        }
        std::string asString() const {
            return m_platform + "/" + m_browser + "/" + m_version.asString();
        }
    protected:
        std::string m_platform;
        std::string m_browser;
        bp::ServiceVersion m_version;
    };
}

#endif
