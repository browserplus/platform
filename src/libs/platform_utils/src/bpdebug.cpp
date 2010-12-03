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

static std::list<std::string>* s_forcedBreakpoints = NULL;

void attachDebuggerImpl()
{
#ifndef NDEBUG
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
    boost::filesystem::path configPath = bp::paths::getConfigFilePath();
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

    // Break if our name is present.
    if (s_forcedBreakpoints != NULL &&
        find( s_forcedBreakpoints->begin(), s_forcedBreakpoints->end(), sName ) != s_forcedBreakpoints->end()) {
        attachDebuggerImpl();
        return;
    }
#endif // NDEBUG
}

void setForcedBreakpoints( const std::list<std::string>& breakpoints )
{
#ifndef NDEBUG
    if (s_forcedBreakpoints != NULL) {
        delete s_forcedBreakpoints;
        s_forcedBreakpoints = NULL;
    }
    s_forcedBreakpoints = new std::list<std::string>(breakpoints);
#endif // NDEBUG
}
   
} // namespace debug
} // namespace bp
