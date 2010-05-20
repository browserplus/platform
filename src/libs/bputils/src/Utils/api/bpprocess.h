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
 *  bpprocess.h
 *
 *  Created by David Grigsby on 7/27/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _BPPROCESS_H_
#define _BPPROCESS_H_

#include <string>
#include <vector>
#include "bpfile.h"


// NOTE:  all pathnames are UTF8 encoded std::strings

namespace bp {
namespace process {

    /**
     * Holds results of spawn operation
     */
    struct spawnStatus
    {
        /** os-specific error code for the operation, 0 on success. */
        long errCode;

        /** pid process id of the spawned-process, 0 on failure. */
        long pid;
        
        /** process HANDLE on windows, not used on Mac */
        void* handle;
    };

    /**
     * Get pid for current process
     * \return pid for current process
     */
    long currentPid();

    /**
     * Spawn a process
     * \param  path Full path to executable file
     * \param  args Command-line arguments
     * \param  status Receives errCode/pid/handle info
     * \param  workingDirectory A path which should be the initial
     *           working directory of the spawned process.  If empty(),
     *           child inherits CWD.     
     * \param  title Alternate process title (e.g. for ps,
                 not currently implemented on any os).
     * \param  inheritWin32StdHandles Whether spawned process should
     *           inherit parents stdin/out/err handles.  (win32 only)
     * \return Success or failure
     */
    bool spawn(const bp::file::Path& path,
               const std::vector<std::string>& vsArgs =
                     std::vector<std::string>(),
               spawnStatus* status = NULL,
               const bp::file::Path& workingDirectory = bp::file::Path(),
               const std::string& sTitle = std::string(),
               bool inheritWin32StdHandles = false );
        
    /**
     * Wait for a process
     * \param  status - spawn status returned from spawn()
     * \param  block - if true, block until process completes
     * \param  exitCode - process' exit status.  processes
     *                    should use error exit codes < 0
     *                    so that codes > 0 can indicate signals
     * \return true if process has exited
     */
    bool wait(const spawnStatus& status,
              bool block, 
              int& exitCode);
              
    /**
     * Kill a process.
     * \param   name - process name
     * \param   forceful - if true the process will be forcefully stopped.
     * \return true on success
     */
    bool kill(const std::string& name, 
              bool forceful = false);

    /**
     * Get the command-line of current process as a vector of UTF-8 strings.
     */
#ifdef WIN32    
    std::vector<std::string> getCommandLineArgs();
#endif
    
}} // bp::process

#endif // _BPPROCESS_H_
