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
 *  FormatTime.cpp
 *
 *  Created by David Grigsby on 2/19/09.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "FormatTime.h"
#include <string>

#include "bpicu.h"
#include "BPUtils/bpstrutil.h"

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable:4512 )
#endif
#include "icu/unicode/datefmt.h"
#include "icu/unicode/locid.h"
#ifdef WIN32
#pragma warning( pop )
#endif


namespace bp {
namespace i18n {

std::string formatRelativeTime( const BPTime& tmThen, 
                                const std::string& sLocale )
{
    UErrorCode stat = U_ZERO_ERROR;
    
    Locale loc( sLocale.c_str() );
    
    DateFormat* pFormatter =
        DateFormat::createDateInstance( DateFormat::kFullRelative, loc );
    if (!pFormatter)
    {
        BP_THROW_TYPE( bp::i18n::IcuException,
                       "createDateInstance returned NULL" );
    }
    
    Calendar* pCal = Calendar::createInstance( stat );
    BP_CHECK_ICU( stat );

    // Set the calendar to tmThen in msecs.
    // Note we have to be a little careful about overflow since darwin time_t
    // is typedef long.
    pCal->setTime( UDate( (long long)tmThen.get() * 1000 ), stat );
    BP_CHECK_ICU( stat );

    FieldPosition pos(0);
    UnicodeString usRes;
    pFormatter->format( *pCal, usRes, pos );

    const int knCapacity = 2048;
    char ac[knCapacity];
    usRes.extract( 0, usRes.length(), ac, knCapacity, "UTF8" );
    return ac;
}



} // i18n
} // bp

