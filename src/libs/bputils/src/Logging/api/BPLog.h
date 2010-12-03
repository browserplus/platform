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
 *  BPLog.h
 *
 *  Declares client api for the BPLog logging system.
 *
 *  Created by David Grigsby on 9/18/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLogFile.h"
#include "BPUtils/BPLogLevel.h"
#include "BPUtils/BPLogLocation.h"
#include "BPUtils/BPLogLogger.h"


#define BPLOG_LEVEL_LOGGER( level, logger, message ) \
{ \
    if (logger.isLevelEnabled( level )) { \
        logger._forcedLog( level, message, BPLOG_LOCATION ); \
    } \
}

#define BPLOG_LEVEL( level, message ) \
BPLOG_LEVEL_LOGGER( level, bp::log::rootLogger(), message )

#define BPLOG_FATAL( message ) \
BPLOG_LEVEL( bp::log::LEVEL_FATAL, message )

#define BPLOG_ERROR( message ) \
BPLOG_LEVEL( bp::log::LEVEL_ERROR, message )

#define BPLOG_WARN( message ) \
BPLOG_LEVEL( bp::log::LEVEL_WARN, message )

#define BPLOG_INFO( message ) \
BPLOG_LEVEL( bp::log::LEVEL_INFO, message )

#define BPLOG_DEBUG( message ) \
BPLOG_LEVEL( bp::log::LEVEL_DEBUG, message )




// These macros use a stringstream to process arguments.
// You can send anything for which there is an ostream operator<<;
// Example:
//     std::string sColor;
//     int nAge;
//     ...
//     BPLOG_INFO_STRM( "my cat is " << sColor << "and she's " << nAge );                               

#define BPLOG_LEVEL_LOGGER_STRM( level, logger, x ) \
{ \
    if (logger.isLevelEnabled( level )) \
    { \
        std::stringstream ss; \
        ss << x; \
        BPLOG_LEVEL_LOGGER( level, logger, ss.str() ); \
    } \
}

#define BPLOG_FATAL_STRM( x ) \
BPLOG_LEVEL_LOGGER_STRM( bp::log::LEVEL_FATAL, bp::log::rootLogger(), x )

#define BPLOG_ERROR_STRM( x ) \
BPLOG_LEVEL_LOGGER_STRM( bp::log::LEVEL_ERROR, bp::log::rootLogger(), x )

#define BPLOG_WARN_STRM( x ) \
BPLOG_LEVEL_LOGGER_STRM( bp::log::LEVEL_WARN, bp::log::rootLogger(), x )

#define BPLOG_INFO_STRM( x ) \
BPLOG_LEVEL_LOGGER_STRM( bp::log::LEVEL_INFO, bp::log::rootLogger(), x )

#define BPLOG_DEBUG_STRM( x ) \
BPLOG_LEVEL_LOGGER_STRM( bp::log::LEVEL_DEBUG, bp::log::rootLogger(), x )



// Use this to log entry/exit from a function using debug level log events.
#define BPLOG_FUNC_SCOPE \
bp::log::ScopeLogger sl( __FILE__, __BP_FUNC__, __LINE__ )


namespace bp {
namespace log {


/**
 * Get reference to the root logger.
 * This is normally only needed for internal operations.
 */
Logger& rootLogger();


/**
 * Removes all current appenders (file, console, etc) from the logging system.
 */
void removeAllAppenders( Logger& logger=rootLogger() );


/**
 * As a performance optimization, sets the logger's log level,
 * below which logging events receive minimal (almost no) processing.
 *
 * If the event's level meets or exceeds the logger's level, the
 * event will be sent to each appender, where it will be emitted if it
 * exceeds that appender's threshold.
 *
 * The logger's default level is "debug" so if this function is *not*
 * called, by default all events will be sent to each appender for
 * further processing.
 *
 * level: events at and above this level will be processed
 */
void setLogLevel( const Level& level,
                  Logger& logger=rootLogger() );


/**
 * Helper to setup logging to console.
 * Adds a ConsoleAppender to the specified logger.
 *
 * level:  events at and above this level will be logged
 *
 * sConsoleTitle: console title
 *
 * timeFormat: time format
 *
 * sLayout: layout name, typical values "standard"|"source"|"raw" 
 *
 * logger: logger to which appender is added
 * 
 * Note: the setupLogTo... functions may be invoked cumulatively.
 */
void setupLogToConsole( const Level& level,
                        const std::string& sConsoleTitle="",
                        const TimeFormat& timeFormat=TIME_UTC,
                        const std::string& sLayout="standard",
                        Logger& logger=rootLogger() );


/**
 * Helper to setup logging to debugger (e.g. VS output window or
 * sysinternals DebugView).
 * Adds a DebuggerAppender to the specified logger.
 *
 * level:  events at and above this level will be logged
 *
 * timeFormat: time format
 *
 * sLayout: layout name, typical values "standard"|"source"|"raw" 
 * 
 * logger: logger to which appender is added
 * 
 * Notes: DebuggerAppender currently only emits events for win32.
 *        The events may be easily seen in the VC output window or by running
 *        sysinternals Dbgview.exe
 *        
 *        The "source" layout will format event messages such that
 *        double-clicking them in VC will navigate the editor to the source line.      
 *
 *        The setupLogTo... functions may be invoked cumulatively.
 */
void setupLogToDebugger( const Level& level,
                         const TimeFormat& timeFormat=TIME_UTC,
                         const std::string& sLayout="source",
                         Logger& logger=rootLogger() );



/**
 * Helper to setup logging to file.
 * Adds a FileAppender to the specified logger.
 *
 * sLogFilePath: log file path
 * 
 * level:  events at and above this level will be logged
 *
 * sLayout: layout name, typical values "standard"|"source"|"raw" 
 *
 * fileMode: controls truncate/append/rollover of existing log file
 * 
 * timeFormat: time format
 *
 * nRolloverSizeKB: If log file exists and is greater than this size in
 *                  KB, truncate the file at first append.
 *                  
 * logger: logger to which appender is added
 * 
 * Notes: If you are going to have multiple processes writing to the
 *        same log file, the typical pattern is to have a "master"
 *        process open the file in truncate/rollover mode and have one
 *        or more "slaves" open the file in append mode.  Since the
 *        actual truncate/rollover is performed lazily at first append,
 *        it is probably best to ensure the master does at least one
 *        append before any slaves do.
 *
 *        The setupLogTo... functions may be invoked cumulatively.
 */
void setupLogToFile( const boost::filesystem::path& logFilePath,
                     const Level& level,
                     const FileMode& fileMode=kTruncate,
                     const TimeFormat& timeFormat=TIME_UTC,
                     const std::string& sLayout="standard",
                     unsigned int nRolloverSizeKB=kDefaultRolloverKB,
                     Logger& logger=rootLogger() );


/**
 * Use an instance of this class to automatically report entry/exit from
 * a scope.
 */
class ScopeLogger
{
public:
    ScopeLogger( const char* cszFile, const char* cszFunc, int nLine );
    ScopeLogger();
    ~ScopeLogger();
    
private:
    LocationInfo m_location;
};


/**
 * Layout factory.
 * This is normally only needed for internal operations.
 */
LayoutPtr layoutFromString( const std::string& sName );


} // log
} // bp

#endif // _LOG_H
