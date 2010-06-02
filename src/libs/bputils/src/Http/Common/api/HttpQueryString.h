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
 *  HttpQueryString.h
 *
 *  Declares QueryString, which represents an http url query component,
 *  and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPQUERYSTRING_H_
#define _HTTPQUERYSTRING_H_

#include "BPUtils/bpstrutil.h"


namespace bp {
namespace http {


class QueryString
{
// Construction/destruction
public:
    // Construct empty query string.
    QueryString() {}

    // Load from a string.
    // The string should not be prepended with a '?'.
    // The string is assumed to be url encoded as appropriate.
    explicit QueryString( const std::string& sIn );
    
    // Compiler-generated dtor is ok
    // Compiler-generated copy ctor is ok
    // Compiler-generated copy assign is ok

// Methods
public:
    void add( const std::string& sName, const std::string& sValue );
    
    // Set sValue per requested field and return true if present
    bool find( const std::string& sName, std::string& sValue ) const;

    // Return value of the requested field, throw if absent.
    std::string get( const std::string& sName ) const;
    
    // Returns the contents in the form "n1=v1&n2=v2..."
    // Note the contents will be appropriately url encoded.
    // Note there will not be a leading '?'.
    std::string toString() const;

// State
private:    
    bp::StrStrMap m_mFields;
}; // QueryString


} // namespace http
} // namespace bp


#endif // _HTTPQUERYSTRING_H

