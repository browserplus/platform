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
 * Returns the level component of a string in the (current) bp log
 * configuration syntax.
 *
 * Examples: levelFromConfig( "info, standard" ) returns "info"
 *           levelFromConfig( "debug" ) returns "debug"
 *
 * This function is useful for sending level strings to setLogLevel().           
 */
std::string levelFromConfig( const std::string& sConfig );


/**
 * Removes all current appenders (file, console, etc) from the logging system.
 */
void removeAllAppenders();

/**
 * Get reference to the root logger.
 * This is normally only needed for internal operations.
 */
Logger& rootLogger();


/**
 * As a performance optimization, sets the logger's log level,
 * below which logging events receive minimal (almost no) processing.
 *
 * If the event's level meets or exceeds the logger's level, the
 * event will be sent to each appender, and will be emitted if it
 * exceeds that appender's threshold.
 *
 * The logger's default level is "debug" so if this function is *not*
 * called, by default all events will be sent to each appender for
 * further processing.
 *
 * sLevel: fatal|error|warn|info|debug
 */
void setLogLevel( const std::string& sLevel,
                  Logger& logger=rootLogger() );


/**
 * Helper to setup logging to console.
 * Adds a ConsoleAppender to the specified logger.
 *
 * sConfig: apppender configuration string of the following format:
 *              "level[, layout]"
 *          where level is: fatal|error|warn|info|debug
 *          and layout is: standard|source|ThrdLvlFuncMsg
 *          
 *          level sets the appender's threshold.
 *          If no layout is specified, "standard" will be used.
 *
 * sConsoleTitle: title to display in the console
 *
 * sTimeFormat: ""|"utc"|"local"|"msec"
 *              "" or "utc": report time in UTC
 *              "local":     report local time
 *              "msec":      report elapsed msec since root logger creation
 *
 * logger: logger to which appender is added
 * 
 * Note: the setupLogTo... functions may be invoked cumulatively.
 */
void setupLogToConsole( const std::string& sConfig,
                        const std::string& sConsoleTitle="",
                        const std::string& sTimeFormat="",
                        Logger& logger=rootLogger() );


/**
 * Helper to setup logging to debugger (e.g. VS output window or
 * sysinternals DebugView).
 * Adds a DebuggerAppender to the specified logger.
 *
 * sConfig: apppender configuration string of the following format:
 *              "level[, layout]"
 *          where level is: fatal|error|warn|info|debug
 *          and layout is: standard|source|ThrdLvlFuncMsg
 *
 *          level sets the appender's threshold.
 *          If no layout is specified, "source" will be used.
 * 
 * sTimeFormat: ""|"utc"|"local"|"msec"
 *              "" or "utc": report time in UTC
 *              "local":     report local time
 *              "msec":      report elapsed msec since root logger creation
 *
 * logger: logger to which appender is added
 * 
 * Note: DebuggerAppender currently only emits events for win32.
 *       The events may be easily seen in the VC output window or by running
 *       sysinternals Dbgview.exe
 * Note: the "source" layout will format event messages such that
 *       double-clicking them in VC will navigate the editor to the source line.      
 * Note: the setupLogTo... functions may be invoked cumulatively.
 */
void setupLogToDebugger( const std::string& sConfig,
                         const std::string& sTimeFormat="",
                         Logger& logger=rootLogger() );



/**
 * Helper to setup logging to file.
 * Adds a FileAppender to the specified logger.
 *
 * sLogFilePath: log file path
 * 
 * sConfig: appender configuration string of the following format:
 *              "level[, layout]"
 *          where level is: fatal|error|warn|info|debug
 *          and layout is: standard|source|ThrdLvlFuncMsg
 * 
 *          level sets the appender's threshold.
 *          If no layout is specified, "standard" will be used.
 *
 * mode: controls truncate/append/rollover of existing log file
 * 
 * sTimeFormat: ""|"utc"|"local"|"msec"
 *              "" or "utc": report time in UTC
 *              "local":     report local time
 *              "msec":      report elapsed msec since root logger creation
 *
 * nRolloverSizeKB: If log file exists and is greater than this size in
 *                  KB, truncate the file at first append.
 *                  
 * logger: logger to which appender is added
 * 
 * Note: the setupLogTo... functions may be invoked cumulatively.
 *
 * Note: If you are going to have multiple processes writing to the
 *       same log file, the typical pattern is to have a "master"
 *       process open the file in truncate/rollover mode and have one
 *       or more "slaves" open the file in append mode.  Since the
 *       actual truncate/rollover is performed lazily at first append,
 *       it is probably best to ensure the master does at least one
 *       append before any slaves do.
 */
void setupLogToFile( const bp::file::Path& logFilePath,
                     const std::string& sConfig,
                     FileMode mode=kTruncate,
                     const std::string& sTimeFormat="",
                     int nRolloverSizeKB=512,
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
