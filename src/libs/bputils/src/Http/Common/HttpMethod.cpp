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
 *  HttpMethod.cpp
 *
 *  Implements the http::Method class and related items.
 *
 *  Created by David Grigsby on 8/05/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HttpMethod.h"


namespace bp {
namespace http {



std::string Method::toString( Code code )
{
    switch (code)
    {
        case HTTP_GET:      return "GET";
        case HTTP_HEAD:     return "HEAD";
        case HTTP_POST:     return "POST";
        case HTTP_PUT:      return "PUT";
        case HTTP_TRACE:    return "TRACE";
        case HTTP_OPTIONS:  return "OPTIONS";
        case HTTP_DELETE:   return "DELETE";
        default:            BP_THROW( "Unrecognized http Method code." );
    }
}


Method::Code Method::fromString( const std::string& sIn )
{
    if (sIn == "GET")
    {
        return HTTP_GET;
    }
    else if (sIn == "HEAD")
    {
        return HTTP_HEAD;
    }
    else if (sIn == "POST")
    {
        return HTTP_POST;
    }
    else if (sIn == "PUT")
    {
        return HTTP_PUT;
    }
    else if (sIn == "TRACE")
    {
        return HTTP_TRACE;
    }
    else if (sIn == "OPTIONS")
    {
        return HTTP_OPTIONS;
    }
    else if (sIn == "DELETE")
    {
        return HTTP_DELETE;
    }
    else
    {
        BP_THROW( "Unrecognized http Method string" );
    }
}



Method::Method() :
    m_code( HTTP_GET )
{}


Method::Method( Code code ) :
    m_code( code )
{}


Method::Method( const std::string& sIn ) :
    m_code( fromString( sIn ) )
{}


Method::Code Method::code() const
{
    return m_code;
}

std::string Method::toString() const
{
    return toString( m_code );
}



} // namespace http
} // namespace bp


