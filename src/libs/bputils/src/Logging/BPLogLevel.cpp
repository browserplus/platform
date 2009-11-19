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
 *  BPLogLevel.cpp
 *
 *  Created by David Grigsby on 9/20/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogLevel.h"
#include "BPUtils/bpstrutil.h"

namespace bp {
namespace log {


std::string levelToString( const Level& level )
{
    switch (level)
    {
        case LEVEL_OFF:    return "OFF";
        case LEVEL_FATAL:  return "FATAL";
        case LEVEL_ERROR:  return "ERROR";
        case LEVEL_WARN:   return "WARN";
        case LEVEL_INFO:   return "INFO";
        case LEVEL_DEBUG:  return "DEBUG";
        case LEVEL_TRACE:  return "TRACE";
        case LEVEL_ALL:    return "ALL";
        default:            return "UNKN_LVL";
    }
}



Level levelFromString( const std::string& sLevel )
{
    if (bp::strutil::isEqualNoCase( sLevel, "OFF" ))
    {
        return LEVEL_OFF;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "FATAL" ))
    {
        return LEVEL_FATAL;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "ERROR" ))
    {
        return LEVEL_ERROR;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "WARN" ))
    {
        return LEVEL_WARN;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "INFO" ))
    {
        return LEVEL_INFO;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "DEBUG" ))
    {
        return LEVEL_DEBUG;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "TRACE" ))
    {
        return LEVEL_TRACE;
    }
    else if (bp::strutil::isEqualNoCase( sLevel, "ALL" ))
    {
        return LEVEL_ALL;
    }
    else
    {
        return LEVEL_OFF;
    }
}


} // log
} // bp
