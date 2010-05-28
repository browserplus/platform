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
 *  bpconvert.h
 *
 *  Created by David Grigsby on 8/22/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _BPCONVERT_H_
#define _BPCONVERT_H_

#include <sstream>
#include <typeinfo>

namespace bp {
namespace conv {


// Convert seconds to milliseconds
inline int SecToMsec( double fSec )
{
    return int( fSec*1000 );
}

// Convert milliseconds to seconds
inline double MsecToSec( int Msec )
{
    return double(Msec) / 1000;
}


//////////////////////////////////////////////////////////////////////////////
///  This is a copy of an earlier version of boost::lexical_cast.
///  This earlier version doesn't handle unusual cases as well as the
///  current boost version, but that version now depends on the large
///  MPL sublibrary.
///  This version will be just fine for the simple string<->numeric
///  conversions for which we use it.
///
///  @param arg         value to be converted
///
///  @return Target     arg converted to Target type
///
///  <b>Notes:</b>
///  - CAUTION: if you try to use this from string to string, 
///    and your source arg contains embedded spaces, this
///    function will throw.  It is probably best not to use this function
///    in scenarios where string to string conversion is possible.
///
///  @par Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
///
///  Permission to use, copy, modify, and distribute this software for any
///  purpose is hereby granted without fee, provided that this copyright and
///  permissions notice appear in all copies and derivatives.
///
///  This software is provided "as is" without express or implied warranty.
///////////////////////////////////////////////////////////////////////////////
template< typename Target, typename Source >
inline Target lexical_cast( const Source& arg )
{
    std::stringstream interpreter;
    Target result;

    if (!(interpreter << arg)    ||
          !(interpreter >> result) ||
          !(interpreter >> std::ws).eof())
    {
        throw std::bad_cast();
    }

    return result;
}


// Converts the provided argument to a string.
//
// This will compile as long as there is a stringstream inserter for the
// argument's type.
//
// Note:    Don't use this function when the argument is already a
//          string type.  See comments for lexical_cast above.
//
// Example:
//          std::string s = bp::conv::toString( 3.14 );
//          std::string s = bp::conv::toString( 123 );
//          std::string s = bp::conv::toString( "pi is fun" );  // BAD
//          
template< typename TSource >
inline std::string toString( const TSource& arg )
{
    return lexical_cast<std::string>( arg );
}


} // conv
} // bp


#endif // _BPCONVERT_H_

