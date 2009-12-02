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
 *  BPLogLevel.h
 *
 *  Created by David Grigsby on 9/20/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGLEVEL_H_
#define _BPLOGLEVEL_H_

#include <limits.h>
#include <string>

namespace bp {
namespace log {


enum Level
{
    LEVEL_OFF = INT_MAX,
    LEVEL_FATAL = 50000,
    LEVEL_ERROR = 40000,
    LEVEL_WARN = 30000,
    LEVEL_INFO = 20000,
    LEVEL_DEBUG = 10000,
    LEVEL_TRACE = 5000,
    LEVEL_ALL = INT_MIN
};


std::string levelToString( const Level& level );
Level levelFromString( const std::string& sLevel );


} // log
} // bp


#endif // _BPLOGLEVEL_H
