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
 *  idna.h
 *
 *  Created by David Grigsby on 1/28/09.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _IDNA_H_
#define _IDNA_H_

#include <string>


namespace bp {
namespace i18n {



/**
 * Performs IDN ToASCII operation.
 * See: http://www.icu-project.org/apiref/icu4c/uidna_8h.html#_details
 *      http://www.ietf.org/rfc/rfc3490.txt
 *      
 * Input string is taken to be in UTF-8.
 *
 */ 
std::string idnaToAscii( const std::string& sIn_utf8 ); // throw IcuException

/**
 * Performs IDN ToUnicode operation.
 * See: http://www.icu-project.org/apiref/icu4c/uidna_8h.html#_details
 *      http://www.ietf.org/rfc/rfc3490.txt
 *
 * Input string is taken to be in UTF-8.
 */ 
std::string idnaToUnicode( const std::string& sIn_utf8 ); // throw IcuException


} // i18n
} // bp

#endif

