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
 *  BPLogLogger.cpp
 *
 *  Created by David Grigsby on 9/18/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogLogger.h"
#include "BPLogEvent.h"

namespace bp {
namespace log {


Logger::Logger() :
    m_level( LEVEL_DEBUG ),
    m_appenders(),
    m_stopwatch(),
    m_mutex(),
    m_appending( false )
{
#ifndef WIN32    
    m_stopwatch.start();
#endif
}


Logger::~Logger()
{

}


void Logger::setLevel( const Level& level )
{
    m_level = level;
}


void Logger::addAppender( AppenderPtr apdr )
{
    bp::sync::Lock lock( m_mutex );
    m_appenders.push_back( apdr );
}


void Logger::removeAllAppenders()
{
    bp::sync::Lock lock( m_mutex );
    m_appenders.clear();
}


bool Logger::isLevelEnabled( const Level& level ) const
{
    return level >= m_level;
}


void Logger::fatal( const std::string& sMessage,
                    const LocationInfo& location )
{
    _conditionalLog( LEVEL_FATAL, sMessage, location);
}


void Logger::error( const std::string& sMessage,
                    const LocationInfo& location )
{
    _conditionalLog( LEVEL_ERROR, sMessage, location);
}


void Logger::warn( const std::string& sMessage,
                   const LocationInfo& location )
{
    _conditionalLog( LEVEL_WARN, sMessage, location);
}


void Logger::info( const std::string& sMessage,
                   const LocationInfo& location )
{
    _conditionalLog( LEVEL_INFO, sMessage, location);
}


void Logger::debug( const std::string& sMessage,
                    const LocationInfo& location )
{
    _conditionalLog( LEVEL_DEBUG, sMessage, location);
}


double Logger::elapsedSec()
{
    return m_stopwatch.elapsedSec();
}


void Logger::_conditionalLog( const Level& level,
                              const std::string& sMessage,
                              const LocationInfo& location )
{
    if (isLevelEnabled( level )) {
        _forcedLog( level, sMessage, location );
    }
}


void Logger::_forcedLog( const Level& level,
                         const std::string& sMessage,
                         const LocationInfo& location )
{
    LoggingEventPtr evt( new LoggingEvent( level, sMessage, location ) );
    appendToAllAppenders( evt );
}


void Logger::appendToAllAppenders( LoggingEventPtr evt )
{
    // Ignore re-entrant calls.
    if (m_appending) {
        return;
    }

    bp::sync::Lock lock( m_mutex );
    m_appending = true;
    
    try
    {
        for (AppenderListIt it = m_appenders.begin();
             it != m_appenders.end(); ++it)
        {
            (*it)->doAppend( evt );
        }
    }
    catch (...)
    {
    }

    m_appending = false;
}


} // log
} // bp

