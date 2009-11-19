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
 *  BPLogEvent.h
 *
 *  Created by David Grigsby on 9/20/07.
 *  Portions based on code from log4cxx.
 *  
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGEVENT_H_
#define _BPLOGEVENT_H_

#include "BPUtils/BPLogLevel.h"
#include "BPUtils/BPLogLocation.h"
#include "bptr1.h"


namespace bp {
namespace log {

class LoggingEvent
{
public:    
    LoggingEvent( const Level& level,
                  const std::string& sMessage,
                  const LocationInfo& location );
    ~LoggingEvent();

    Level           level()         { return m_level; }
    std::string     message()       { return m_sMessage; }
    LocationInfo    location()      { return m_location; }
    double          elapsedSec()    { return m_fElapsedSec; }
    time_t          timeStamp()     { return m_timeStamp; }
    std::string     threadId()      { return m_sThreadId; }
    
private:
    Level           m_level;
    std::string     m_sMessage;
    LocationInfo    m_location;
    double          m_fElapsedSec;
    time_t          m_timeStamp;
    std::string     m_sThreadId;
};

typedef std::tr1::shared_ptr<LoggingEvent> LoggingEventPtr;


} // log
} // bp


#endif // _BPLOGEVENT_H
