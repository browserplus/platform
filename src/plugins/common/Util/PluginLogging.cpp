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
 *  PluginLogging.cpp
 *
 *  Created by David Grigsby on 12/31/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "PluginLogging.h"
#include <iostream>
#include "BPUtils/bpconfig.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"

using namespace std;


namespace bp {
namespace plugin {


bool setupLogging( const bp::file::Path& logfilePath )
{
    // Setup a config file reader.
    bp::config::ConfigReader configReader;
    bp::file::Path configFilePath = bp::paths::getConfigFilePath();
    if (!configReader.load(configFilePath)) 
    {
        cerr << "couldn't read config file at: " << configFilePath << endl;
        return false;
    }

    // Clear out any existing appenders.
    bp::log::removeAllAppenders();
    
    // Setup the system-wide minimum log level.
    string sVal;
    if (configReader.getStringValue( "PluginLogLevel", sVal ))
    {
        bp::log::setLogLevel( sVal );
    }

    // Get log time format (if present) from the config file.
    string sTimeFormat;
    (void) configReader.getStringValue("LogTimeFormat",sTimeFormat);
    
    // Setup log to console.
    if (configReader.getStringValue( "PluginLogToConsole", sVal ))
    {
        bp::log::setupLogToConsole( sVal, "", sTimeFormat );
    }

    // Setup log to debugger.
    if (configReader.getStringValue( "PluginLogToDebugger", sVal ))
    {
        bp::log::setupLogToDebugger( sVal, sTimeFormat );
    }

    // Setup log to file.
    if (configReader.getStringValue( "PluginLogToFile", sVal ))
    {
        // Setup the logfile path.
        bp::file::Path path = bp::paths::getObfuscatedWritableDirectory()
                              / logfilePath;
        bp::log::setupLogToFile( path, sVal, bp::log::kTruncate, sTimeFormat );
    }

    return true;
}


} // plugin
} // bp

