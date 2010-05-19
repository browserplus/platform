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
 *  BPLogLayout.cpp
 *
 *  Created by David Grigsby on 9/26/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogLayout.h"
#include "bpconvert.h"
#include "bpprocess.h"
#include <sstream>
#include <time.h>

using namespace bp::conv;


namespace bp {
namespace log {


// forward decls
static void appendUtcTime( std::string& sOut, const LoggingEventPtr& evt );
static void appendLocalTime( std::string& sOut, const LoggingEventPtr& evt );
static void appendMsecTime( std::string& sOut, const LoggingEventPtr& evt );


Layout::Layout() :
    m_timeFormat( TIME_UTC )
{

}


Layout::~Layout()
{

}


void Layout::setTimeFormat( const TimeFormat& tf )
{
    m_timeFormat = tf;
}


void Layout::appendFile( std::string& sOut, const LoggingEventPtr& evt )
{
    sOut.append( evt->location().file() );
}


void Layout::appendFunc( std::string& sOut, const LoggingEventPtr& evt )
{
    sOut.append( evt->location().func() );
}


void Layout::appendLevel( std::string& sOut, const LoggingEventPtr& evt )
{
    std::string sLevel = levelToString( evt->level() );
    size_t nPadding = 5 - sLevel.length();
    if (nPadding > 0)
    {
        sLevel.append( nPadding, ' ' );
    }
    
    sOut.append( sLevel );
}


void Layout::appendLine( std::string& sOut, const LoggingEventPtr& evt )
{
    std::stringstream ss;
    ss << evt->location().line();
    sOut.append( ss.str() );
}


void Layout::appendMessage( std::string& sOut, const LoggingEventPtr& evt )
{
    sOut.append( evt->message() );
}


void Layout::appendThread( std::string& sOut, const LoggingEventPtr& evt )
{
    sOut.append( evt->threadId() );
}


void Layout::appendPid( std::string& sOut, const LoggingEventPtr& evt )
{
    // pid doesn't change, so we'll cache it here
    static std::string pid;
    if (pid.empty()) {
        std::stringstream ss;
        ss << bp::process::currentPid();
        pid = ss.str();
    }
    sOut.append( pid );
}


void Layout::appendTime( std::string& sOut, const LoggingEventPtr& evt )
{
    switch (m_timeFormat)
    {
        case TIME_LOCAL:
            appendLocalTime( sOut, evt );
            break;
        case TIME_MSEC:
            appendMsecTime( sOut, evt );
            break;
        case TIME_UTC:
        default:
            appendUtcTime( sOut, evt );
            break;
    }
}


void appendUtcTime( std::string& sOut, const LoggingEventPtr& evt )
{
    // ISO8601 format (one of them): yyyy-mm-dd hh:mm:ssZ
    // 'Z' means UTC timezone.
    const int nBUFLEN = 80;
    char szTime[nBUFLEN];
    time_t timeEvent = evt->timeStamp();
#if defined(_MSC_VER )
    tm tmGmt;
    gmtime_s( &tmGmt, &timeEvent );
    strftime( szTime, nBUFLEN, "%Y-%m-%d %H:%M:%SZ", &tmGmt );
#else
    strftime( szTime, nBUFLEN, "%Y-%m-%d %H:%M:%SZ", gmtime( &timeEvent ) );
#endif
    sOut.append( szTime );
}


void appendLocalTime( std::string& sOut, const LoggingEventPtr& evt )
{
    // ISO8601 format (one of them): yyyy-mm-dd hh:mm:ss
    const int nBUFLEN = 80;
    char szTime[nBUFLEN];
    time_t timeEvent = evt->timeStamp();
#if defined(_MSC_VER )
    tm tmLoc;
    localtime_s( &tmLoc, &timeEvent );
    strftime( szTime, nBUFLEN, "%Y-%m-%d %H:%M:%S", &tmLoc );
#else
    strftime( szTime, nBUFLEN, "%Y-%m-%d %H:%M:%S", localtime( &timeEvent ) );
#endif
    sOut.append( szTime );
}


void appendMsecTime( std::string& sOut, const LoggingEventPtr& evt )
{
    sOut.append( toString( SecToMsec( evt->elapsedSec() ) ) );
}


} // log
} // bp



