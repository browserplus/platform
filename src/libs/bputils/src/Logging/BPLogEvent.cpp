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
 *  BPLogEvent.cpp
 *
 *  Created by David Grigsby on 9/20/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogEvent.h"

#include <sstream>
#include <time.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "BPLog.h"
#include "BPLogLevel.h"


namespace bp {
namespace log {


LoggingEvent::LoggingEvent( const Level& level,
                            const std::string& sMessage,
                            const LocationInfo& location ) :
    m_level( level ),
    m_sMessage( sMessage ),
    m_location( location ),
    m_fElapsedSec(),
    m_timeStamp(),
    m_sThreadId()
{
    m_fElapsedSec = rootLogger().elapsedSec();
	::time( &m_timeStamp );

    // TODO: use textual threadname if it exists.
    std::stringstream ss;
#if defined(_MSC_VER)    
    ss << GetCurrentThreadId();
#else
    ss << pthread_self();
#endif
    m_sThreadId = ss.str();
}


LoggingEvent::~LoggingEvent()
{

}


} // log
} // bp
