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
 *  ConfigReader.cpp
 *  Helps read values from (json, currently) configuration files.
 *
 *  Created by David Grigsby on 8/04/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpconfig.h"
#include <iostream>
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"


namespace bp
{
namespace config
{

    
// Reads a config file at a place specified by bp::paths.
// The file currently must be in JSON format.
ConfigReader::ConfigReader()
    : m_pConfigMap(NULL)
{
}


bool
ConfigReader::load( const bp::file::Path& path )
{
    // allow the same object to be re-used
    delete m_pConfigMap;
    m_pConfigMap = NULL;

    std::string sConfig;
    if (!bp::strutil::loadFromFile( path, sConfig ))
    {
        return false;
    }

    // Read the JSON string into a BP object.
    std::string sParseErr;
    bp::Object* pObj = bp::Object::fromPlainJsonString( sConfig, &sParseErr );
    if (pObj == NULL)
    {
        BPLOG_ERROR_STRM( "fromPlainJsonString() failed: " << sParseErr );
        return false;
    }

    bp::Map* pMap = dynamic_cast<bp::Map*>( pObj );
    if (!pMap)
    {
        BPLOG_ERROR( "Config file not a json map as expected." );
        return false;
    }

    m_pConfigMap = pMap;
    
    return true;
}


ConfigReader::~ConfigReader()
{
    delete m_pConfigMap;
    m_pConfigMap = NULL;
}


bool
ConfigReader::getStringValue(const std::string& sKey, std::string& sValue) const
{
    if (!m_pConfigMap) {
        return false;
    }

    return m_pConfigMap->getString( sKey, sValue );
}


bool
ConfigReader::getArrayOfStrings(const std::string& sKey,
                                std::list<std::string>& sValue) const
{
    sValue.clear();

    if (!m_pConfigMap) {
        return false;
    }

    const bp::List* arr;
    if (!m_pConfigMap->getList(sKey, arr)) return false;
    
    for (unsigned int i = 0; i < arr->size(); i++) {
        const bp::Object* str = arr->value(i);
        if (str->type() != BPTString) return false;

        sValue.push_back(((bp::String *)str)->value());
    }

    return true;
}


bool
ConfigReader::getIntegerValue(const std::string& sKey,
                              long long int & iValue) const
{
    if (!m_pConfigMap) {
        return false;
    }
    
    return m_pConfigMap->getLong( sKey, iValue );
}

bool
ConfigReader::getBooleanValue(const std::string& sKey, bool & iValue) const
{
    if (!m_pConfigMap) {
        return false;
    }
    
    return m_pConfigMap->getBool( sKey, iValue );
}
    

bool
ConfigReader::getJsonMap(const std::string& sKey,
                         const bp::Map*& pmValue) const
{
    if (!m_pConfigMap) {
        return false;
    }

    return m_pConfigMap->getMap( sKey, pmValue );
}



} // namespace config
} // namespace bp
