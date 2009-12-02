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
 *  HttpMethod.h
 *
 *  Declares Method and related items.
 *
 *  Created by David Grigsby on 7/20/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPMETHOD_H_
#define _HTTPMETHOD_H_

#include <string>
#include "bperrorutil.h"
#include "bpstrutil.h"


namespace bp {
namespace http {


class Method
{
// Class-scope items    
public:
    // Would love to use "GET", "HEAD", etc, but unfortunately some
    // windows headers #define DELETE. :(
    enum Code { HTTP_GET, HTTP_HEAD, HTTP_POST, HTTP_PUT, HTTP_TRACE,
                HTTP_OPTIONS, HTTP_DELETE };

    // Static method to convert an http method into a string
    static std::string toString( Code code );

    // Static method to get an http method code from a string
    static Code fromString( const std::string& sIn );

    
// Construction/Destruction
public:
    // Default ctor - default to GET.
    Method();
    
    Method( Code code );
    
    Method( const std::string& sIn );

    Code code() const;

    std::string toString() const;

    
private:
    Code m_code;
}; // Method


} // namespace http
} // namespace bp


#endif // _HTTPMETHOD_H_

