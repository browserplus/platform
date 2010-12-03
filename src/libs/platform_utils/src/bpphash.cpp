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
 *  bpphash.cpp
 *  Persistent application wide storage of string based key/value pairs
 *  
 *  Created by Lloyd Hilaiel on 10/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */

#include "bpphash.h"

#include <string>
#include <iostream>

#include "bpconfig.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bptime.h"
#include "BPUtils/bptypeutil.h"
#include "ProductPaths.h"


class PHashConfigReader : public bp::config::ConfigReader 
{
public:
    bool setStringValue(const std::string& sKey,
                        const std::string& sValue)
    {
        if (m_pConfigMap == NULL) {
            m_pConfigMap = new bp::Map;
        }
        m_pConfigMap->add(sKey, new bp::String(sValue));
        return true;
    }
    
    void removeStringValue(const std::string& sKey)
    {
        if (m_pConfigMap) {
            m_pConfigMap->kill(sKey.c_str());
        }
    }

    std::string toString()
    {
        std::string s;
        if (m_pConfigMap) s = m_pConfigMap->toPlainJsonString(true);
        return s;
    }
};

static PHashConfigReader s_cache;
static boost::filesystem::path s_phashPath;
static BPTime s_lastRead = 0L;

static bool
updateCache()
{
    if (s_phashPath.empty()) {
        s_phashPath = bp::paths::getPersistentStatePath();
        if (s_phashPath.empty()) return false;
    }

    BPTime modTime((long)0);
    try {
        modTime.set(boost::filesystem::last_write_time(s_phashPath));
    } catch (const boost::filesystem::filesystem_error&) {
        // empty
    }

    // if modTime == 0 it means the file doesn't exist
    if (modTime.compare(s_lastRead) > 0) {
        // slurp it up and parse it
        if (s_cache.load(s_phashPath)) {
            s_lastRead.set(modTime);
        } else {
            return false;
        }
    }
    return true;
}

bool
bp::phash::get(const std::string & key, std::string & outVal)
{
    return (updateCache() && s_cache.getStringValue(key, outVal));
}

bool
bp::phash::set(const std::string & key, const std::string& val)
{
    // update cache may fail if the file does not exist yet.
    updateCache();

    if (!s_cache.setStringValue(key, val)) return false;

    std::string s = s_cache.toString();

    if (!bp::strutil::storeToFile(s_phashPath, s)) {
        return false;
    }
    
    return true;
}


void
bp::phash::remove(const std::string & key)
{
    std::string val;
    if (get(key, val)) {
        s_cache.removeStringValue(key);
        (void) bp::strutil::storeToFile(s_phashPath, s_cache.toString());
    }
}
