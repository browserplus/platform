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

#include "BPHandle.h"
#include "BPUtils/bptypeutil.h"

BPHandle::BPHandle(const std::string& type,
                   int id,
                   const std::string& safeName,
                   boost::uintmax_t size,
                   const std::set<std::string>& mimeTypes) 
    : m_type(type), m_id(id), m_name(safeName),
      m_size(size), m_mimeTypes(mimeTypes)
{
}


BPHandle::~BPHandle()
{
}


std::string 
BPHandle::type() const 
{ 
    return m_type;
}


int 
BPHandle::id() const 
{ 
    return m_id;
}


std::string 
BPHandle::name() const 
{ 
    return m_name;
}

boost::uintmax_t
BPHandle::size() const
{
    return m_size;
}

std::set<std::string>
BPHandle::mimeTypes() const
{
    return m_mimeTypes;
}

bool 
BPHandle::operator<(const BPHandle& other) const
{
    return m_id < other.m_id;
}


bp::Map*
BPHandle::toBPMap() const
{
    using namespace bp;
    Map* m = new Map;
    m->add(BROWSERPLUS_HANDLETYPE_KEY, new String(type().c_str()));
    m->add(BROWSERPLUS_HANDLEID_KEY, new Integer(id()));

    // deprecated old style 
    m->add(DEPRECATED_BROWSERPLUS_HANDLENAME_KEY, new String(name().c_str()));

    // new style
    m->add(BROWSERPLUS_HANDLENAME_KEY, new String(name().c_str()));
    m->add(BROWSERPLUS_HANDLESIZE_KEY, new Integer(size()));
    std::set<std::string> mt = mimeTypes();
    std::set<std::string>::const_iterator it;
    List* l = new List;
    for (it = mt.begin(); it != mt.end(); ++it) {
        l->append(new String(it->c_str()));
    }
    m->add(BROWSERPLUS_HANDLEMIMETYPE_KEY, l);

    return m;
}
    
