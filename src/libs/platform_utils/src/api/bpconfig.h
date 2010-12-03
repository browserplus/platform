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
 *  bpconfig.h
 *  Helps read values from configuration files.
 *  
 *  Created by David Grigsby on 8/04/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPCONFIG_H__
#define __BPCONFIG_H__

#include <list>
#include "BPUtils/bptypeutil.h"

namespace bp {
namespace config {


class ConfigReader
{
public:
    /**
     *   ctor
     */
    ConfigReader();

    /**
     * load configuration from a file.  This will flush any
     * previous configuration loaded even if the call fails.
     */
    bool load(const boost::filesystem::path& path);

    /**
     *   dtor
     */
    virtual ~ConfigReader();

    /**
     *   Fetch the specified string value
     *   \param     sKey Key
     *   \param     sValue receives read value if successful
     *   \return    true iff successful
     */
    bool getStringValue(const std::string& sKey,
                        std::string& sValue) const;

    /**
     *   Fetch a key with a value which is an array of strings
     *   \param     sKey Key
     *   \param     sValue receives read value if successful
     *   \return    true iff successful
     */
    bool getArrayOfStrings(const std::string& sKey,
                           std::list<std::string>& lsValue) const;

    /**
     *   Fetch the specified integer value
     *   \param     sKey Key
     *   \param     iValue receives read value if successful
     *   \return    true iff successful
     */
    bool getIntegerValue(const std::string& sKey,
                         long long int & iValue) const;

    /**
     *   Fetch the specified boolean value
     *   \param     sKey Key
     *   \param     iValue receives read value if successful
     *   \return    true iff successful
     */
    bool getBooleanValue(const std::string& sKey, bool & iValue) const;


    /**
     *   Fetch the specified json map value
     *   \param     sKey Key
     *   \param     poValue receives read value if successful
     *              caller does NOT own the returned object.
     *   \return    true iff successful
     */
    bool getJsonMap(const std::string& sKey,
                    const bp::Map*& poValue) const;


protected:
    bp::Map* m_pConfigMap;

private:
    // make non-copyable
    ConfigReader(const ConfigReader&);
    ConfigReader& operator=(const ConfigReader&);
};


} // namespace config
} // namespace bp

#endif
