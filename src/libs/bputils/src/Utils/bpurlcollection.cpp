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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  bpurlcollection.cpp
 *  A persistent collection of URLs suitable for whitelisting or
 *  blacklisting tasks.
 *  
 *  Created by Lloyd Hilaiel on 04/29/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 */

#include "bpurlcollection.h"
#include "bpfile.h"
#include "bpstrutil.h"
#include "bpurl.h"

#include <sstream>


bp::URLCollection::URLCollection()
    : m_useDomainForHTTP(false), m_data(NULL)
{
}

bp::URLCollection::~URLCollection()
{
    if (m_data != NULL) delete m_data;
    m_data = NULL;
    m_list = NULL;
}

bool
bp::URLCollection::init(const bp::file::Path & path,
                        bool useDomainForHTTP)
{
    m_path = path;

    if (boost::filesystem::is_regular(m_path)) {
        // attempt to read
        std::string s;
        if (bp::strutil::loadFromFile(m_path, s)) {
            bp::Object * o = bp::Object::fromPlainJsonString(s);
            if (o != NULL) {
                if (o->type() == BPTMap) {
                    m_data = (bp::Map *) o;
                } else {
                    delete o;
                }
            }
            
            if (m_data) {
                // validate
                if (!m_data->has("urls", BPTList) ||
                    !m_data->has("UseDomainForHTTP", BPTBoolean))
                {
                    delete m_data;
                    m_data = NULL;
                }
                else 
                {
                    m_useDomainForHTTP =
                        ((bp::Bool *) m_data->get("UseDomainForHTTP"))->value();
                    m_list = (bp::List *) m_data->get("urls");
                }
            }
        }
    } else {
        // attempt to write
        m_data = new bp::Map;
        m_list = new bp::List;
        m_data->add("UseDomainForHTTP", new bp::Bool(useDomainForHTTP));
        m_data->add("urls", m_list);

        // now attempt to write to disk
        if (!writeToDisk()) {
            delete m_data;
            m_data = NULL;
            m_list = NULL;
        }
        m_useDomainForHTTP = useDomainForHTTP;
    }

    return m_data != NULL;
}


bool
bp::URLCollection::has(const std::string & inUrl)
{
    if (m_data == NULL || m_list == NULL) return false;

    std::string url = normalizeURL(inUrl);
    
    for (unsigned int i = 0; i < m_list->size(); i++) 
    {
        bp::String * disObj = (bp::String *) m_list->value(i);
        if (disObj && !url.compare(disObj->value()))
            return true;
    }

    return false;
}

bool
bp::URLCollection::add(const std::string & inUrl)
{
    if (m_data == NULL || m_list == NULL) return false;    

    if (!has(inUrl)) {
        std::string url = normalizeURL(inUrl);        
        m_list->append(new bp::String(url));
        if (!writeToDisk()) return false;
    }

    return true;
}

std::string
bp::URLCollection::normalizeURL(const std::string & url)
{
    if (m_useDomainForHTTP) {
        bp::url::Url up;
        if (up.parse(url)) {
            if (!up.scheme().compare("http") ||
                !up.scheme().compare("https"))
            {
                std::stringstream ss;
                ss << up.host() << ":" << up.port();
                return ss.str();
            }
        }
    }

    return url;
}


bool
bp::URLCollection::writeToDisk()
{
    if (m_data == NULL || m_list == NULL) return false;    
    std::string s = m_data->toPlainJsonString(true);
    return bp::strutil::storeToFile(m_path, s);
}
   
