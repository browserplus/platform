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
 *  HttpBody.h
 *
 *  Declares Body and related items.
 *
 *  Created by David Grigsby on 7/20/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPBODY_H_
#define _HTTPBODY_H_

#include <string>
#include <vector>

#include "BPUtils/bpurl.h"
#include "BPUtils/bpfile.h"

namespace bp {
namespace http {


// A body can get its contents from a contained buffer,
// from a file, or from a URL.  The choices are mutually
// exclusive.
//
class Body
{
// Class-scope items
public:
    typedef unsigned char               tElement;
    // TODO: use a queue or streambuf instead?
    typedef std::vector<tElement>       tBuffer;
    typedef tBuffer::iterator           iterator;
    typedef tBuffer::const_iterator     const_iterator;


// Construction/destruction
public:
    // Default ctor - empty body.
    Body();

    // Construct from any two iterators that point to compatible
    // element type.
    template< class TIter >
    Body( TIter itFirst, TIter itLast )
    {
        assign( itFirst, itLast );
    }
    
    // Default copy ctor is ok
    // Default copy assignment is ok
    // Default Dtor is ok

    
// Queries
public:
    // Returns body size (0 for URL body)
    int     size() const;

    // Returns whether body empty (false for URL body)
    bool    empty() const;

    
// Methods
public:
        
    // Clear contents
    void        clear();
    
    // Return a string holding a copy of body contents
    std::string toString() const;
    
    // ------------- Byte buffer methods -------------------------------
        
    // Set the body per the specified range.
    template< class TIter >
    void        assign( TIter itFirst, TIter itLast )
    {
        m_vbBody.assign( itFirst, itLast );
    }

    // Set the body to the specified string.
    void        assign( const std::string& sIn );

    // TODO: make clients use the template form?
    // Set the body from the specified start and length.
    void        assign( const tElement* pStart, int nLength );
    
    // Append a range of bytes to the body.
    template< class TIter >
    void        append( TIter itFirst, TIter itLast )
    {
        m_vbBody.insert( m_vbBody.end(), itFirst, itLast );
    }
    
    // Append a string to the body.
    void        append( const std::string& sIn );

    // TODO: make clients use the template form?
    // Append to the body from the specified start and length.
    void        append( const tElement* pStart, int nLength );
     
    // ------------- file methods -------------------------------

    // Set the body from the specified file
    // Clears any 
    void        fromPath( const bp::file::Path& path );
    
    // Return path assigned to body
    bp::file::Path path() const;
    
// Accessors, only valid for byte buffer body
public:
    // returns reference to body
    const tBuffer& data() const;

    // returns begin() iterator for our data.
    // Pointed to vals are of type tElement
    const_iterator begin() const;

    // returns end() iterator for our data.
    // Pointed to vals are of type tElement
    const_iterator end() const;

    const tElement*   elementAddr( int nIndex ) const;

    
// State
private:
    // TODO: use a queue or streambuf instead?
    // Queue vs. vector: smaller realloc's vs. locality of reference
    //                   Read once semantics vs. random access semantics
    tBuffer        m_vbBody;
    
    bp::file::Path m_path;
}; // Body


} // namespace http
} // namespace bp


#endif // _HTTPBODY_H_

