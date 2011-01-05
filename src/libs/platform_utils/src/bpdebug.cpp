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
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <signal.h>
#endif

using namespace std;


#ifdef MACOSX
// from http://developer.apple.com/library/mac/#qa/qa2004/qa1361.html
// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
static bool 
AmIBeingDebugged(void)

{
    int                 junk;
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.

    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.

    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    assert(junk == 0);

    // We're being debugged if the P_TRACED flag is set.

    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}
#endif


namespace bp {
namespace debug {

static std::list<std::string>* s_forcedBreakpoints = NULL;

void attachDebuggerImpl()
{
#ifndef NDEBUG
#ifdef WIN32
    DebugBreak();
#elif defined(MACOSX)
    if (AmIBeingDebugged()) {
        __asm__ volatile ("int3");
    } else {  
        pid_t p = getpid();
        std::stringstream ss;
        ss << "Stopping process (attachDebugger() called) pid - " << p;
        std::cerr << ss.str() << std::endl;
        BPLOG_ERROR( ss.str() );    
        showAlert(ss.str());
        kill(p, SIGSTOP);
    }
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
