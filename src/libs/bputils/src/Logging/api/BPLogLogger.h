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
 *  LogLogger.h
 *
 *  Created by David Grigsby on 9/18/07.
 *  Portions based on code from log4cxx.
 *  
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _LOGLOGGER_H_
#define _LOGLOGGER_H_

#include <string>
#include <vector>

#include "BPUtils/BPLogAppender.h"
#include "BPUtils/BPLogEvent.h"
#include "BPUtils/BPLogLevel.h"
#include "BPUtils/BPLogLocation.h"
#include "BPUtils/bpstopwatch.h"


namespace bp {
namespace log {


class Logger
{
public:
    /**
     * Construction / Destruction
     */
    Logger();
    virtual ~Logger();
    
    /**
     * Add an appender
     */
    void addAppender( AppenderPtr apdr );
    
    /**
     * Remove all appenders
     */
    void removeAllAppenders();

    /**
     *
     */
    bool isLevelEnabled( const Level& level ) const;
            
    /**
     * Report a fatal event
     */
    void fatal( const std::string& sMessage,
                const bp::log::LocationInfo& location );

    /**
     * Report an error event
     */
    void error( const std::string& sMessage,
                const bp::log::LocationInfo& location );

    /**
     * Report a warn event
     */
    void warn( const std::string& sMessage,
               const bp::log::LocationInfo& location );

    /**
     * Report an info event
     */
    void info( const std::string& sMessage,
               const bp::log::LocationInfo& location );

    /**
     * Report a debug event
     */
    void debug( const std::string& sMessage,
                const bp::log::LocationInfo& location );
    
    /**
     * Set the level of this logger
     */
    void setLevel( const Level& level );

    /**
     * Get the elapsed seconds since this logger created.
     */
    double elapsedSec();
    
    /**
     * Fire an event without checking level
     * This method is primarily for internal use but needs to be public.
     */
    void _conditionalLog( const Level& level,
                          const std::string& message,
                          const LocationInfo& location );
    /**
     * Fire an event without checking level
     * This method is primarily for internal use but needs to be public.
     */
    void _forcedLog( const Level& level,
                     const std::string& message,
                     const LocationInfo& location );

private:
    void appendToAllAppenders( LoggingEventPtr evt );
    
    Level                   m_level;
    AppenderList            m_appenders;
#ifdef WIN32
    bp::time::PerfStopwatch m_stopwatch;
#else
    bp::time::Stopwatch     m_stopwatch;
#endif
    bp::sync::Mutex         m_mutex;
    bool                    m_appending;
    
    Logger( const Logger& );
    Logger& operator=( const Logger& );
};

typedef std::tr1::shared_ptr<Logger> LoggerPtr;


} // bp
} // log

#endif // _LOGLOGGER_H
