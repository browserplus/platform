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

/////////////////////////
//
#include "idna.h"

#include <vector>
#include "bpicu.h"
#include "icu/unicode/uidna.h"
#include "icu/unicode/unistr.h"
#include "icu/unicode/ustring.h"




namespace bp {
namespace i18n {


std::string idnaToAscii( const std::string& sIn_utf8 )
{
    UnicodeString us( sIn_utf8.c_str(), -1, "UTF8" );

    const int knCapacity = 2048;
    UChar auc[knCapacity];
    UErrorCode err = U_ZERO_ERROR;
    int nucWritten = uidna_IDNToASCII( us.getTerminatedBuffer(),
                                       us.length(),
                                       auc, knCapacity,
                                       UIDNA_DEFAULT, NULL, &err );
    BP_CHECK_ICU( err );

    return std::string( auc, auc+nucWritten );
}


std::string idnaToUnicode( const std::string& sIn_utf8 )
{
    UnicodeString us( sIn_utf8.c_str(), -1, "UTF8" );
    
    const int knCapacity = 2048;
    UChar auc[knCapacity];
    UErrorCode err = U_ZERO_ERROR;
    int nucWritten = uidna_IDNToUnicode( us.getTerminatedBuffer(),
                                         sIn_utf8.length(),
                                         auc, knCapacity,
                                         UIDNA_DEFAULT, NULL, &err );
    BP_CHECK_ICU( err );

    // Convert from UTF-16 to UTF-8.
    char ac[knCapacity];
    int ncWritten;
    u_strToUTF8( ac, knCapacity, &ncWritten,
                 auc, nucWritten, &err );
    BP_CHECK_ICU( err );
    
    return std::string( ac );
}


} // i18n
} // bp

