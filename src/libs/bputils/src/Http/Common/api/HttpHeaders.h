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
 *  HttpHeaders.h
 *
 *  Declares HttpHeaders and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPHEADERS_H_
#define _HTTPHEADERS_H_

#include <map>
#include <string>


namespace bp {
namespace http {


class Headers
{
// Class-scope items
public:
    typedef std::map<std::string,std::string>   Map;
    typedef Map::const_iterator                 const_iterator;
    
    static const char* const ksContentLength;
    static const char* const ksContentType;    
    static const char* const ksUserAgent;
    static const char* const ksIfModifiedSince;
    static const char* const ksAuthorization;
    static const char* const ksHost;
    static const char* const ksReferer;
    static const char* const ksCookie;
    static const char* const ksLocation;
    static const char* const ksRange;
    static const char* const ksConnection;
    static const char* const ksTransferEncoding;

    
// Construction/destruction
public:
    // Default ctor - no headers.
    Headers();
        
    // Load from a string of the form "n1: v1CRLFn2: v2..." 
//  Headers( const std::string& sIn );

    // Load from a vector of pairs of strings.
//  Headers( const bp::StrPairVec& vIn );

    // Load from a map of strings to strings.
//  Headers( const bp::StrStrMap& mIn );

    // Maybe a generic ctor that takes two iterators?
    
    // Dtor
    ~Headers();

    
// Queries
public:
    // Returns number of name-value pairs in the collection.
    int         size() const;

    // Returns true if the collection is empty.
    bool        empty() const;

    
// Accessors
    // returns begin() iterator for our data.
    // Pointed to vals are of type pair<string,string>
    const_iterator begin() const;
    
    // returns end() iterator for our data.
    // Pointed to vals are of type pair<string,string>
    const_iterator end() const;
    
    
// Methods
public:
    // Add a header
    void        add( const std::string& sName, const std::string& sValue );
    
    // Add headers from a string of the form "n1: v1CRLFn2: v2..." 
    void        add( const std::string& sIn );
    
    // Set sValue per requested header and return true if present
    bool        find( const std::string& sName, std::string& sValue ) const;

    // Return the value of requested header, throw if absent.
    std::string get( const std::string& sName ) const;

    // Render to a string of the form "n1: v1CRLFn2: v2..." 
    std::string toString() const;

    // re-initialize headers
    void clear();
    
// State
private:    
    Map         m_mHeaders;
}; // Headers



} // namespace http
} // namespace bp


#endif // _HTTPHEADERS_H

