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
 *  HttpHeaders.cpp
 *
 *  Implements the HttpHeaders class and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HttpHeaders.h"

#include <sstream>
#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/BPLog.h"


namespace bp {
namespace http {


const char* const Headers::ksContentLength      = "Content-Length";
const char* const Headers::ksContentType        = "Content-Type";    
const char* const Headers::ksUserAgent          = "User-Agent";
const char* const Headers::ksIfModifiedSince    = "If-Modified-Since";
const char* const Headers::ksAuthorization      = "Authorization";
const char* const Headers::ksHost               = "Host";
const char* const Headers::ksReferer            = "Referer";
const char* const Headers::ksCookie             = "Cookie";
const char* const Headers::ksLocation           = "Location";
const char* const Headers::ksRange              = "Range";
const char* const Headers::ksConnection         = "Connection";
const char* const Headers::ksTransferEncoding   = "Transfer-Encoding";


Headers::Headers()
{

}


Headers::~Headers()
{

}


int Headers::size() const
{
    return m_mHeaders.size();
}


bool Headers::empty() const
{
    return m_mHeaders.empty();
}


Headers::const_iterator Headers::begin() const
{
    return m_mHeaders.begin();
}


Headers::const_iterator Headers::end() const
{
    return m_mHeaders.end();
}


void Headers::add( const std::string& sName, const std::string& sValue )
{
    m_mHeaders.insert( make_pair( sName, sValue ) );
}


// Add headers from a string of the form "n1: v1CRLFn2: v2..." 
void Headers::add( const std::string& sIn )
{
    if (sIn.empty())
    {
        return;
    }
    
    bp::StrVec vs = bp::strutil::split( sIn, "\r\n" );

    for (StrVecCIt it = vs.begin(); it != vs.end(); ++it)
    {
        // Ignore blank lines.  
        // Wininet gives a header string with some blank lines at the end.
        if (it->length() == 0)
            continue;

        bp::StrVec vsLine = bp::strutil::split( *it, ": " );
        if (vsLine.size() == 2)
        {
            add( vsLine[0], vsLine[1] );
        }
        else
        {
            BPLOG_ERROR("Unexpected header format" );
        }
    }
}

bool Headers::find( const std::string& sName, std::string& sValue ) const
{
    bp::StrStrMapCIt it = m_mHeaders.find( sName );
    if (it == m_mHeaders.end())
    {
        return false;
    }

    sValue = it->second;
    return true;
}


std::string Headers::get( const std::string& sName ) const
{
    std::string sValue;
    if (!find( sName, sValue ))
    {
        BPLOG_ERROR_STRM( "Requested header <" << sName << "> not found." );
    }    
    return sValue;
}


std::string Headers::toString() const
{
    std::stringstream ss;
    for (bp::StrStrMapCIt it = m_mHeaders.begin(); it != m_mHeaders.end(); ++it)
    {
        ss << it->first << ": " << it->second << "\r\n";
    }
    
    return ss.str();
}

void
Headers::clear()
{
    m_mHeaders.clear();
}

} // namespace http
} // namespace bp


