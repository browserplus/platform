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
 *  HttpQueryString.cpp
 *
 *  Implements the QueryString class and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HttpQueryString.h"

#include <sstream>
#include "bperrorutil.h"
#include "bpurl.h"


namespace bp {
namespace http {


// We expect: "name1=val1&name2=val2...", where name and val are url encoded.
QueryString::QueryString( const std::string& sIn )
{
    if (sIn.empty())
    {
        return;
    }
    
    bp::StrVec vsPairs = bp::strutil::split( sIn, "&" );

    for (bp::StrVecCIt it = vsPairs.begin(); it != vsPairs.end(); ++it)
    {
        bp::StrVec vsPair = bp::strutil::split( *it, "=" );
        if (vsPair.size() != 2)
        {
            BP_THROW( "Unexpected query string format." );
        }

        add( bp::url::urlDecode( vsPair[0] ), bp::url::urlDecode( vsPair[1] ) );
    }
}


void QueryString::add( const std::string& sName, const std::string& sValue )
{
    m_mFields.insert( std::make_pair( sName, sValue ) );
}


bool QueryString::find( const std::string& sName, std::string& sValue ) const
{
    bp::StrStrMapCIt it = m_mFields.find( sName );

    if (it == m_mFields.end())
    {
        return false;
    }
    else
    {
        sValue = it->second;
        return true;
    }
}
    

std::string QueryString::get( const std::string& sName ) const
{
    bp::StrStrMapCIt it = m_mFields.find( sName );

    if (it == m_mFields.end())
    {
        std::stringstream ssErr;
        ssErr << "Expected query string field '" << sName << "' not found.";
        BP_THROW( ssErr.str() );
    }

    return it->second;
}


std::string QueryString::toString() const
{
    std::string sRet;

    for (bp::StrStrMapCIt it = m_mFields.begin(); it != m_mFields.end(); )
    {
        sRet += bp::url::urlEncode( it->first ) + "=" +
                bp::url::urlEncode( it->second );

        if (++it != m_mFields.end())
        {
            sRet += "&";
        }
    }

    return sRet;
}



} // namespace http
} // namespace bp


