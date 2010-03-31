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

/*
 *  HttpBody.cpp
 *
 *  Implements the HttpHeaders class and related items.
 *
 *  Created by David Grigsby on 7/27/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HttpBody.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"


namespace bp {
namespace http {


Body::Body() : m_vbBody(), m_path()
{
}


void Body::fromPath( const bp::file::Path& path )
{
    m_vbBody.clear();
    m_path = path;
}


bp::file::Path Body::path() const
{
    return m_path;
}


uintmax_t Body::size() const
{
    return bp::file::isRegularFile(m_path) ?
               bp::file::size(m_path) : m_vbBody.size();
}


bool Body::empty() const
{
    if (!m_path.empty()) {
        return size() == 0;   
    }
    return m_vbBody.empty();
}


void Body::clear()
{
    m_vbBody.clear();
}


void Body::assign( const std::string& sIn )
{
    m_path.clear();
    assign( sIn.begin(), sIn.end() );
}


void Body::assign( const tElement* pStart, int nLength )
{
    assign( pStart, pStart+nLength );
}


void Body::append( const std::string& sIn )
{
    append( sIn.begin(), sIn.end() );
}


void Body::append( const tElement* pStart, int nLength )
{
    append( pStart, pStart+nLength );
}


std::string Body::toString() const
{
    std::string rval;
    if (!m_path.empty()) {
        (void) bp::strutil::loadFromFile( m_path, rval );
    } else {
        rval = std::string( m_vbBody.begin(), m_vbBody.end() );
    }
    return rval;
}


const Body::tBuffer& 
Body::data() const
{
    return m_vbBody;
}

Body::const_iterator Body::begin() const
{
    return m_vbBody.begin();
}

Body::const_iterator Body::end() const
{
    return m_vbBody.end();
}


const unsigned char* Body::elementAddr( int nIndex ) const
{
    return &(m_vbBody[nIndex]);
}


        
} // namespace http
} // namespace bp


