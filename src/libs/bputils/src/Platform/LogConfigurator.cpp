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
 *  LogConfigurator.cpp
 *
 *  Implementation of class which eases logging subsytem configuration.
 *  
 *  Created by David Grigsby on Thu April 15 2010.
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 */
#include "LogConfigurator.h"
#include "BPUtils/bpconfig.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"

using namespace std;
using namespace bp::strutil;

namespace bp {
namespace log {


Configurator::Configurator() :
    m_level( LEVEL_INFO ),
    m_dest( kDestConsole ),
    m_path(),
    m_layout( "standard" ),
    m_timeFormat( TIME_UTC ),
    m_fileMode( kSizeRollover ),
    m_rolloverKB( kDefaultRolloverKB ),
    m_consoleTitle(),
    m_serviceLogMode( kServiceLogCombined )
{
}


void Configurator::loadConfigFile( const bp::file::Path& path )
{
    bp::config::ConfigReader reader;
    if (!reader.load( path )) 
    {
        cerr << "couldn't read config file at: " << path << endl;
        return;
    }

    const bp::Map* map;
    if (!reader.getJsonMap( "Logging", map )) {
        cerr << "couldn't find 'Logging' map in config file." << endl;
        return;
    }

    string level;
    if (map->getString( "level", level )) {
        m_level = levelFromString( level );
    }

    string dest;
    if (map->getString( "dest", dest )) {
        if (isEqualNoCase( dest, "console" )) {
            m_dest = kDestConsole;
        } else if (isEqualNoCase( dest, "file" )) {
            m_dest = kDestFile;
        } else if (isEqualNoCase( dest, "win32" )) {
            m_dest = kDestWin32;
        } else {
        }
    }

    string layout;
    if (map->getString( "layout", layout )) {
        m_layout = layout;
    }

    string timeFormat;
    if (map->getString( "timeFormat", timeFormat )) {
        m_timeFormat = timeFormatFromString( timeFormat );
    }

    int rolloverKB;
    if (map->getInteger( "fileRolloverKB", rolloverKB )) {
        m_rolloverKB = rolloverKB;
    }

    string serviceLogMode;
    if (map->getString( "serviceLogMode", serviceLogMode )) {
        if (isEqualNoCase( serviceLogMode, "combined" )) {
            m_serviceLogMode = kServiceLogCombined;
        } else if (isEqualNoCase( serviceLogMode, "separate" )) {
            m_serviceLogMode = kServiceLogSeparate;
        } else {
        }
    }
    
}


void Configurator::configure( Logger& logger )
{
    bp::log::removeAllAppenders( logger );

    bp::log::setLogLevel( m_level, logger );

    // If they've requested file dest but didn't give us a path,
    // revert to console.
    if (m_dest == kDestFile && m_path.empty()) {
        m_dest = kDestConsole;
    }
    
    if (m_dest == kDestFile) {
        setupLogToFile( m_path, m_level, m_fileMode, m_timeFormat, m_layout, 
                        m_rolloverKB, logger );
    }
    else if (m_dest == kDestWin32) {
        setupLogToDebugger( m_level, m_timeFormat, m_layout, logger );
    }
    else {
        setupLogToConsole( m_level, m_consoleTitle, m_timeFormat, m_layout,
                           logger );
    }
}


const Level& Configurator::getLevel() const
{
    return m_level;
}

void Configurator::setLevel( const Level& level )
{
    m_level = level;
}

const Destination& Configurator::getDestination() const
{
    return m_dest;
}

void Configurator::setDestination( const Destination& dest )
{
    m_dest = dest;
}

const bp::file::Path& Configurator::getPath() const
{
    return m_path;
}

void Configurator::setPath( const bp::file::Path& path )
{
    m_path = path;
}

const std::string& Configurator::getLayout() const
{
    return m_layout;
}

void Configurator::setLayout( const std::string& layout )
{
    m_layout = layout;
}

const TimeFormat& Configurator::getTimeFormat() const
{
    return m_timeFormat;
}

void Configurator::setTimeFormat( const TimeFormat& timeFormat )
{
    m_timeFormat = timeFormat;
}

const FileMode& Configurator::getFileMode() const
{
    return m_fileMode;
}

void Configurator::setFileMode( const FileMode& fileMode )
{
    m_fileMode = fileMode;
}

int Configurator::getRolloverKB() const
{
    return m_rolloverKB;
}

void Configurator::setRolloverKB( int rolloverKB )
{
    m_rolloverKB = rolloverKB;
}

const std::string& Configurator::getConsoleTitle() const
{
    return m_consoleTitle;
}

void Configurator::setConsoleTitle( const std::string& title )
{
    m_consoleTitle = title;
}

const ServiceLogMode& Configurator::getServiceLogMode() const
{
    return m_serviceLogMode;
}

void Configurator::setServiceLogMode( const ServiceLogMode& mode )
{
    m_serviceLogMode = mode;
}




} // log
} // bp

