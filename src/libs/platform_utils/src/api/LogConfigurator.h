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
 *  LogConfigurator.h
 *
 *  An object that configures the logging subsystem, typically
 *  incorporating settings from a config file.
 *  
 *  Created by David Grigsby on Thu April 15 2010.
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 */

#ifndef LOGCONFIGURATOR_H_
#define LOGCONFIGURATOR_H_

#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "ProductPaths.h"


namespace bp {
namespace log {


enum Destination
{
    kDestConsole,
    kDestFile,
    kDestWin32
};


enum ServiceLogMode
{
    kServiceLogCombined,
    kServiceLogSeparate
};


class Configurator
{
public:
    Configurator();

    /**
     * Load appropriate values from specified config file.
     */
    void loadConfigFile( const bp::file::Path& path =
                         bp::paths::getConfigFilePath() );

    /**
     * Configure the logging system for this process using our current
     * state.
     */
    void configure( Logger& logger=rootLogger() );

    /**
     * Accessors
     */
    const Level& getLevel() const;
    void setLevel( const Level& level );

    const Destination& getDestination() const;
    void setDestination( const Destination& dest );

    const bp::file::Path& getPath() const;
    void setPath( const bp::file::Path& path );

    const std::string& getLayout() const;
    void setLayout( const std::string& layout );

    const TimeFormat& getTimeFormat() const;
    void setTimeFormat( const TimeFormat& timeFormat );

    const FileMode& getFileMode() const;
    void setFileMode( const FileMode& fileMode );

    int getRolloverKB() const;
    void setRolloverKB( int rolloverKB );

    const std::string& getConsoleTitle() const;
    void setConsoleTitle( const std::string& title );

    const ServiceLogMode& getServiceLogMode() const;
    void setServiceLogMode( const ServiceLogMode& mode );
    
private:
    Level           m_level;
    Destination     m_dest;
    bp::file::Path  m_path;
    std::string     m_layout;
    TimeFormat      m_timeFormat;
    FileMode        m_fileMode;
    unsigned int    m_rolloverKB;
    std::string     m_consoleTitle;
    ServiceLogMode  m_serviceLogMode;
    
private:
    Configurator( const Configurator& );
    Configurator& operator=( const Configurator& );
};


}
}

#endif
