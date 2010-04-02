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
 *  BPLogTime.cpp
 *
 *  Created by David Grigsby on 7/16/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogTime.h"
#include "BPUtils/bpstrutil.h"

namespace bp {
namespace log {


std::string timeFormatToString( const TimeFormat& tf )
{
    switch (tf)
    {
        case TIME_UTC:      return "utc";
        case TIME_LOCAL:    return "local";
        case TIME_MSEC:     return "msec";
        default:            return "unknown_time_format";
    }
}


// TODO: change default to local?
TimeFormat timeFormatFromString( const std::string& s )
{
    if (bp::strutil::isEqualNoCase( s, "utc" ))
    {
        return TIME_UTC;
    }
    else if (bp::strutil::isEqualNoCase( s, "local" ))
    {
        return TIME_LOCAL;
    }
    if (bp::strutil::isEqualNoCase( s, "msec" ))
    {
        return TIME_MSEC;
    }
    else
    {
        return TIME_UTC;
    }
}


} // log
} // bp
