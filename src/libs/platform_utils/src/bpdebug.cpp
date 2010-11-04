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
 *  bpdebug.cpp
 *
 *  Created by David Grigsby on 5/3/09.
 *
 */

#include "bpdebug.h"
#include <string>
#include "bpconfig.h"
#include "BPUtils/BPLog.h"
#include "ProductPaths.h"


#ifdef WIN32
#include <Windows.h>
#elif defined(MACOSX)
#include <signal.h>
#endif

using namespace std;


namespace bp {
namespace debug {

std::list<std::string> s_forcedBreakpoints;


void attachDebuggerImpl()
{
#ifdef WIN32
    DebugBreak();
#elif defined(MACOSX)
    std::stringstream ss;
    pid_t p = getpid();
    ss << "Stopping process (attachDebugger() called) pid - " << p;
    std::cerr << ss.str() << std::endl;
    BPLOG_ERROR( ss.str() );    
    kill(p, SIGSTOP);
#else
#endif
}

void attachDebugger()
{
#ifndef NDEBUG    
attachDebuggerImpl();
#endif // NDEBUG
}

void breakpoint( const std::string& sName )
{
#ifndef NDEBUG
    // Load the bp config file.
    bp::config::ConfigReader config;
    bp::file::Path configPath = bp::paths::getConfigFilePath();
    if (config.load( configPath ))  {
        // Get "Breakpoints" member array.
        list<string> lsNames;
        if (config.getArrayOfStrings( "Breakpoints", lsNames )) {
            // Break if our name is present.
            if (find( lsNames.begin(), lsNames.end(), sName ) != lsNames.end()) {
                attachDebugger();
                return;
            }
        }
    }
    else {
        BPLOG_ERROR_STRM( "couldn't read config file at " << configPath );
    }
#endif // NDEBUG
    // Break if our name is present.
    if (find( s_forcedBreakpoints.begin(), s_forcedBreakpoints.end(), sName ) != s_forcedBreakpoints.end()) {
        attachDebuggerImpl();
        return;
    }
}

void setForcedBreakpoints( const std::list<std::string>& breakpoints )
{
    s_forcedBreakpoints = breakpoints;
}
   
} // namespace debug
} // namespace bp
