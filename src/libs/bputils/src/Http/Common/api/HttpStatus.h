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
 *  HttpStatus.h
 *
 *  Declares the HttpStatus class which manages http status codes.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPSTATUS_H_
#define _HTTPSTATUS_H_

#include <string>

namespace bp {
namespace http {

class Status
{
// Class-scope Items
public:
    // enums for the standard codes to allow comparisons
    enum Code {
        // informational
        CONTINUE                = 100,
        SWITCHING_PROTOCOLS     = 101,

        // success
        OK                      = 200,
        CREATED                 = 201,
        ACCEPTED                = 202,
        PARTIAL_INFORMATION     = 203,
        NO_RESPONSE             = 204,
        RESET_CONTENT           = 205,
        PARTIAL_CONTENT         = 206,

        // redirection
        MULTIPLE_CHOICES        = 300,
        MOVED                   = 301,
        FOUND                   = 302,
        METHOD                  = 303,
        NOT_MODIFIED            = 304,
        USE_PROXY               = 305,
        USE_TEMPORARY_REDIRECT  = 307,

        // client error
        BAD_REQUEST             = 400,
        UNAUTHORIZED            = 401,
        PAYMENT_REQUIRED        = 402,
        FORBIDDEN               = 403,
        NOT_FOUND               = 404,
        METHOD_NOT_ALLOWED      = 405,
        NOT_ACCEPTABLE          = 406,
        NO_PROXY_AUTHENICATION  = 407,
        REQUEST_TIMEOUT         = 408,
        CONFLICT                = 409,
        GONE                    = 410,
        LENGTH_REQUIRED         = 411,

        // server error
        INTERNAL_ERROR          = 500,
        NOT_IMPLEMENTED         = 501,
        BAD_GATEWAY             = 502,
        SERVICE_UNAVAILABLE     = 503,
        GATEWAY_TIMEOUT         = 504,
        VERSION_UNSUPPORTED     = 505,
    };

    // conversion of code to reason phrase
    static std::string  codeToString( Code code );

    
// Construction et. al.
public:    
    // Default ctor - initializes to OK.
    Status();

    // Initialize from a status code.
    Status( Code code);


// Public Methods
public:
    // Return a string of the form: "200 OK"
    std::string toString() const;

    
// Accessors
public:
    Code        code() const;
    void        setCode( Code code );

    
// State
private:
    Code m_nCode;
};


} // namespace http
} // namespace bp


#endif // _HTTPSTATUS_H

