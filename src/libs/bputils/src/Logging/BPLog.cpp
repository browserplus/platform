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
 *  BPLog.cpp
 *
 *  Created by David Grigsby on 9/20/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLog.h"

#include "BPLogConsoleAppender.h"
#include "BPLogDebuggerAppender.h"
#include "BPLogFileAppender.h"
#include "BPLogLogger.h"
#include "BPLogStockLayouts.h"
#include "BPUtils/bpstrutil.h"


namespace bp {
namespace log {


static Logger s_rootLogger;


std::string levelFromConfig( const std::string& sConfig )
{
    std::vector<std::string> vsVals = bp::strutil::splitAndTrim( sConfig, "," );
    return vsVals.empty() ? "" : vsVals[0];
}


void removeAllAppenders()
{
    rootLogger().removeAllAppenders();
}


Logger& rootLogger()
{
    return s_rootLogger;
}


void setLogLevel( const std::string& sLevel,
                  Logger& logger )
{
    logger.setLevel( levelFromString( sLevel ) );
}


void setupLogToConsole( const std::string& sConfig,
                        const std::string& sConsoleTitle,
                        const std::string& sTimeFormat,
                        Logger& logger )
{
    std::vector<std::string> vsVals = bp::strutil::splitAndTrim( sConfig, "," );
    if (vsVals.empty())
        return;

    bp::log::Level threshold = bp::log::levelFromString( vsVals[0] );

    std::string sLayout = vsVals.size() >= 2 ? vsVals[1] : "standard";
    LayoutPtr layout( layoutFromString( sLayout ) );
    layout->setTimeFormat( timeFormatFromString( sTimeFormat ) );

    ConsoleAppenderPtr apdr( new ConsoleAppender( layout, sConsoleTitle ) );
    apdr->setThreshold( threshold );
    logger.addAppender( apdr ); 
}


void setupLogToDebugger( const std::string& sConfig,
                         const std::string& sTimeFormat,
                         Logger& logger )
{
    std::vector<std::string> vsVals = bp::strutil::splitAndTrim( sConfig, "," );
    if (vsVals.empty())
        return;

    bp::log::Level threshold = bp::log::levelFromString( vsVals[0] );

    std::string sLayout = vsVals.size() >= 2 ? vsVals[1] : "source";
    LayoutPtr layout( layoutFromString( sLayout ) );
    layout->setTimeFormat( timeFormatFromString( sTimeFormat ) );

    DebuggerAppenderPtr apdr( new DebuggerAppender( layout ) );
    apdr->setThreshold( threshold );
    logger.addAppender( apdr ); 
}


void setupLogToFile( const bp::file::Path& logFilePath,
                     const std::string& sConfig,
                     FileMode mode,
                     const std::string& sTimeFormat,
                     int nRolloverSizeKB,
                     Logger& logger )
{
    std::vector<std::string> vsVals = bp::strutil::splitAndTrim( sConfig, "," );
    if (vsVals.empty())
        return;

    bp::log::Level threshold = bp::log::levelFromString( vsVals[0] );

    std::string sLayout = vsVals.size() >= 2 ? vsVals[1] : "standard";
    LayoutPtr layout( layoutFromString( sLayout ) );
    layout->setTimeFormat( timeFormatFromString( sTimeFormat ) );

    FileAppenderPtr apdr( new FileAppender( logFilePath, layout,
                                            mode, nRolloverSizeKB ) );
    apdr->setThreshold( threshold );
    logger.addAppender( apdr ); 
}


ScopeLogger::ScopeLogger( const char* cszFile, const char* cszFunc, int nLine )
{
    // We'll treat scope messages as debug level.
    if (bp::log::rootLogger().isLevelEnabled( bp::log::LEVEL_DEBUG ))
    {
        // Lazily init this member for performance.
        m_location = LocationInfo( cszFile, cszFunc, nLine );
        
        std::string sMsg = "> Entry.";
        bp::log::rootLogger()._forcedLog( bp::log::LEVEL_DEBUG, sMsg,
                                          m_location );
    } 
}


ScopeLogger::~ScopeLogger()
{
    // We'll treat scope messages as debug level.
    if (bp::log::rootLogger().isLevelEnabled( bp::log::LEVEL_DEBUG ))
    {
        std::string sMsg = "< Exit.";
        bp::log::rootLogger()._forcedLog( bp::log::LEVEL_DEBUG, sMsg,
                                          m_location );
    } 
}


LayoutPtr layoutFromString( const std::string& sLayout )
{
    if (bp::strutil::isEqualNoCase( sLayout, "raw" ) )
    {
        return LayoutPtr( new RawLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "source" ))
    {
        return LayoutPtr( new SourceLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "standard" ) )
    {
        return LayoutPtr( new StandardLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "ThrdLvlFuncMsg" ) )
    {
        return LayoutPtr( new ThrdLvlFuncMsgLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "TimeLvlMsg" ) )
    {
        return LayoutPtr( new TimeLvlMsgLayout );
    }
    else if (bp::strutil::isEqualNoCase( sLayout, "FuncMsg" ) )
    {
        return LayoutPtr( new FuncMsgLayout );
    }
    else
    {
        return LayoutPtr( new StandardLayout );
    }
}


} // log
} // bp
