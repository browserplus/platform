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
 *  HttpStatus.cpp
 *
 *  Declares the HttpStatus class which manages http status codes.
 *
 *  Created by Lloyd Hilaiel on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include <sstream>
#include "api/BPLog.h"
#include "api/HttpStatus.h"


namespace bp {
namespace http {


std::string Status::codeToString( Code code )
{
    switch (code)
    {
        case 100:   return "Continue";
        case 101:   return "Switching protocols";
        case 200:   return "OK";
        case 201:   return "Created";
        case 202:   return "Accepted";
        case 203:   return "Partial information";
        case 204:   return "No response";
        case 205:   return "Reset content";
        case 206:   return "Partial content";
        case 300:   return "Multiple choices";
        case 301:   return "Moved";
        case 302:   return "Found";
        case 303:   return "Method";
        case 304:   return "Not modified";
        case 305:   return "Use proxy";
        case 307:   return "Use temporary redirect";
        case 400:   return "Bad request";
        case 401:   return "Unauthorized";
        case 402:   return "Payment required";
        case 403:   return "Forbidden";
        case 404:   return "Not found";
        case 405:   return "Method not allowed";
        case 406:   return "Not acceptable";
        case 407:   return "No proxy authentication";
        case 408:   return "Request timeout";
        case 409:   return "Conflict";
        case 410:   return "Gone";
        case 411:   return "Length required";
        case 500:   return "Internal error";
        case 501:   return "Not implemented";
        case 502:   return "Bad gateway";
        case 503:   return "Service unavailable";
        case 504:   return "Gateway timeout";
        case 505:   return "Version unsupported";
        default:    BPLOG_ERROR_STRM("Unrecognized http status code " << code);
                    return "Unknown response code from server";
    }
}


Status::Status() :
    m_nCode( OK )
{

}


Status::Status( Code code ) :
    m_nCode( code )
{

}


std::string Status::toString() const
{
    std::stringstream ss;
    ss << m_nCode << " " << codeToString(m_nCode);
    return ss.str();
}


Status::Code Status::code() const
{
    return m_nCode;
}


void Status::setCode( Code code )
{
    m_nCode = code;
}


} // namespace http
} // namespace bp

