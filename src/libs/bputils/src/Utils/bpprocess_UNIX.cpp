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
 *  bpprocess_Darwin.cpp
 *
 *  Created by David Grigsby on 7/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpprocess.h"

#include <iostream>
#include <sstream> 

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#ifdef LINUX
#include <sys/wait.h>
#endif

using std::string;
using std::vector;
using namespace bp::file;


// Forward Declarations
static bool
forkAndExec(const Path & path,
            const Path & pwd,
            char *const argv[],
            bp::process::spawnStatus* pStat);


long
bp::process::currentPid()
{
    return getpid();
}


bool
bp::process::spawn(const Path& path,
                   const Path& wd,
                   spawnStatus* pStatus)
{
    // Setup argv.
    vector<char*> vArgs;
    
    // argv[0] = name used to invoke the program.
    vArgs.push_back(const_cast<char*>(path.externalUtf8().c_str()));
    
    // argv[argc] = 0.
    vArgs.push_back(0);
    
    return forkAndExec(path, wd, &vArgs[0], pStatus);
}


bool
bp::process::spawn(const Path& path,
                   const string& sTitle,
                   const Path& wd,
                   const vector<string>& vsArgs,
                   spawnStatus* pStatus)
{
    // Setup argv.
    vector<char*> vArgs;

    // argv[0] = name used to invoke the program.
    if (sTitle.empty() || true) {
        vArgs.push_back(const_cast<char*>(path.externalUtf8().c_str()));
    } else {
        vArgs.push_back(const_cast<char*>(sTitle.c_str()));
    }

    // Add caller's args to argv.
    for (vector<string>::const_iterator it = vsArgs.begin();
         it != vsArgs.end(); ++it)
    {
        vArgs.push_back(const_cast<char*>(it->c_str()));
    }
            
    // argv[argc] = 0.
    vArgs.push_back(0);

    return forkAndExec(path, wd, &vArgs[0], pStatus);
}


bool
forkAndExec(const Path & path,
            const Path & pwd,
            char *const argv[],
            bp::process::spawnStatus* pStat)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // if a pwd is provided, now is the time to chdir
        if (!pwd.empty() && 0 != chdir(pwd.external_directory_string().c_str())) {
            // TODO: we need a good way to return error!
            // a convention around return codes??
            std::cerr << "failed to chdir()" << std::endl;
            exit(1);
        }

        // We're the child.  Now execute the desired image.
        int nRet = execv(path.external_file_string().c_str(), argv);
        if (nRet == -1)
        {
            // TODO: perhaps notify parent of child execve failure.
            exit(1);
        }

        // return is basically irrelevant in this case.
        return true;
    }
    else if (pid == -1)
    {
        // We're the parent and fork failed.
        if (pStat)
        {
            pStat->errCode = errno;
            pStat->pid = 0;
        }

        return false;
    }
    else
    {
        // We're the parent and fork succeeded.
        if (pStat)
        {
            pStat->errCode = 0;
            pStat->pid = pid;
        }

        return true;
    }
}


bool
bp::process::wait(const bp::process::spawnStatus& stat,
                  bool block,
                  int& exitCode)
{
    pid_t pid = wait4(stat.pid, &exitCode, 
                      block == false ? WNOHANG : 0, 
                      NULL);
    if (pid != 0) {
        if (WIFEXITED(exitCode)) {
            // cast to char to handle codes < 0
            exitCode = (char) WEXITSTATUS(exitCode);
        } else if (WIFSIGNALED(exitCode)) {
            exitCode = WTERMSIG(exitCode);
        } else if (WIFSTOPPED(exitCode)) {
            exitCode = WSTOPSIG(exitCode);
        }
    }
    return pid != 0;
}


bool
bp::process::kill(const string& name,
                  bool forceful)
{
    // using spawn() doesn't work.  bummer
    // Must use login uid rather than $USER, which 
    // fails in the presence of non-ascii chars
    // Also, must specify -SIGNAL before -u
    std::stringstream oss;
    oss << "killall ";
    oss << (forceful ? " -SIGKILL " : " -SIGINT ");
    oss << " -u ";
    const char* lname = getlogin();
    struct passwd* pw = getpwnam(lname);
    oss << pw->pw_uid << " " << name;
    std::string cmd = oss.str();
    int i = system(cmd.c_str());
    return i == 0;
}
