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
 *  HttpVersion.h
 *
 *  Declares Version and related items.
 *
 *  Created by David Grigsby on 7/20/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPVERSION_H_
#define _HTTPVERSION_H_

#include <string>
#include "bperrorutil.h"
#include "bpstrutil.h"


namespace bp {
namespace http {


class Version
{
// Class-scope Items
public:
    enum Code { HTTP1_0, HTTP1_1 };

    static std::string  toString( Code code )
    {
        switch (code)
        {
            case HTTP1_0:   return "HTTP/1.0";
            case HTTP1_1:   return "HTTP/1.1";
            default:        BP_THROW( "Unrecognized http Version code." );
        }
    }
            
    static Code fromString( const std::string&  sIn )
    {
        if (sIn == "HTTP/1.0")
        {
            return HTTP1_0;
        }
        else if (sIn == "HTTP/1.1")
        {
            return HTTP1_1;
        }
        else
        {
            BP_THROW( "Unrecognized http Version string" );
        }
    }

    
// Construction/Destruction
public:
    // Default ctor - default to Http 1.1
    Version() : m_code( HTTP1_1 ) {}

    Version( Code code ) : m_code( code ) {}
    
    Version( const std::string& sIn ) : m_code( fromString( sIn ) ) {}

    
// Public Methods
public:
    // Whether version is HTTP 1.0
    bool        isHttp10() const { return m_code == HTTP1_0; }
    
    // Whether version is HTTP 1.1
    bool        isHttp11() const { return m_code == HTTP1_1; }

    // Return string representation of version.
    std::string toString() const { return toString( m_code ); }

    
// State    
private:
    Code m_code;
}; // Version


} // namespace http
} // namespace bp


#endif // _HTTPVERSION_H_

