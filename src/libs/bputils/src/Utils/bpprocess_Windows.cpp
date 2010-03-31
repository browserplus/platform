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
 *  bpprocess_Windows.cpp
 *
 *  Created by David Grigsby on 7/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include <sstream>
#include <Windows.h>
#include <ShellAPI.h>

#include "bpstrutil.h"
#include "bpprocess.h"
#include "bpthread.h"
#include "bpfile.h"
#include "BPLog.h"
#include "OS.h"

using std::ostream_iterator;
using std::string;
using std::stringstream;
using std::vector;

// Forward Declarations
static bool
invokeCreateProcess(const bp::file::Path& path,
                    const std::string& sTitle,
                    const bp::file::Path& workingDirectory,
                    const vector<string>& vsArgs,
                    bp::process::spawnStatus* pStat);


static std::wstring
escapeArgs(const std::vector<std::string>& args)
{
    wchar_t space(' ');
    wchar_t quote('"');
    wchar_t backslash('\\');
    std::wstring escaped;
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) {
            escaped.append(&space, 1);
        }
        std::wstring a = bp::strutil::utf8ToWide(args[i]);
        escaped.append(&quote, 1);
        for (size_t j = 0; j < a.length(); j++) {
            if (a[j] == quote) {
                escaped.append(&backslash, 1);
                escaped.append(&quote, 1);
            } else {
                escaped.append(&(a[j]), 1);
            }
        }
        escaped.append(&quote, 1);
    }
    return escaped;
}


long
bp::process::currentPid()
{
    return GetCurrentProcessId();
}


bool
bp::process::spawn(const bp::file::Path& path,
                   const bp::file::Path& workingDirectory,
                   spawnStatus* status)
{
    // TODO: set working directory
    vector<string> vsArgs;
    return invokeCreateProcess(path, std::string(), workingDirectory,
                               vsArgs, status);
}


bool
bp::process::spawn(const bp::file::Path& path,
                   const std::string& sTitle,
                   const bp::file::Path& workingDirectory,
                   const vector<string>& vsArgs,
                   spawnStatus* status)
{
    return invokeCreateProcess(path, sTitle, workingDirectory,
                               vsArgs, status);
}


// TODO
// * env handling
// * optional close/return handles?
// * figure out how to reliably change process name so that sTitle appears
//   in the "Activity Monitor" or "Task Manager" 
static bool 
invokeCreateProcess(const bp::file::Path& path,
                    const std::string& sTitle, 
                    const bp::file::Path& workingDirectory,
                    const vector<string>& vsArgs,
                    bp::process::spawnStatus* pStat)
{
	// get args into writable C buf needed by CreateProcessW()
    std::wstring wsArgs;
    wsArgs.append(L"\"" + path.external_file_string() + L"\" ");
    wsArgs.append(escapeArgs(vsArgs));
    wchar_t* wsBuf = _wcsdup(wsArgs.c_str());
    if (!wsBuf) {
        BPLOG_ERROR("memory allocation failure");
        return false;
    }

    // now execute path with args
    std::wstring dir = workingDirectory.external_directory_string();
    const wchar_t* pDir = dir.empty() ? NULL : dir.c_str();
    STARTUPINFO sinfo;
    ZeroMemory(&sinfo, sizeof(sinfo));
    sinfo.dwFlags = STARTF_USESHOWWINDOW;
    sinfo.wShowWindow = SW_HIDE;
    sinfo.cb = sizeof(sinfo);
    PROCESS_INFORMATION pinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    BOOL bRet = ::CreateProcessW(path.external_file_string().c_str(),
                                 wsBuf,
                                 NULL,              // process attributes
                                 NULL,              // thread attributes
                                 FALSE,             // inherit handles
                                 CREATE_NO_WINDOW,  // creation flags
                                 NULL,              // environment
                                 pDir,              // working directory
                                 &sinfo,            // starup info
                                 &pinfo);           // process information

    if (bRet) {
        if (pStat) {
            pStat->errCode = 0;
            pStat->pid = pinfo.dwProcessId;
            pStat->handle = pinfo.hProcess;
        }
    } else {
        if (pStat) {
            pStat->errCode = GetLastError();
            pStat->pid = 0;
            pStat->handle = 0;
        }
    }
    if (wsBuf) {
        free(wsBuf);
    }

    // Cannot close hProcess handle, we need it for wait
    if (pinfo.hThread) {
        CloseHandle(pinfo.hThread);
    }
    return (bRet!=0);
}


bool 
bp::process::wait(const spawnStatus& status,
                  bool block,
                  int& exitCode)
{
    bool rval = false;
    if (WaitForSingleObject(status.handle, 
                            block ? INFINITE : 0) == WAIT_OBJECT_0) {
        DWORD exitStatus = 0;
        (void) GetExitCodeProcess(status.handle, &exitStatus);
        CloseHandle(status.handle);
        exitCode = exitStatus;
        rval = true;
    }
    return rval;
}


bool
bp::process::kill(const string& name,
                  bool /*forceful*/)
{
    vector<string> killArgs;
    killArgs.push_back("/F");
    killArgs.push_back("/IM");
    killArgs.push_back(name);
    bp::process::spawnStatus status;
    return bp::process::spawn(bp::file::Path(L"taskkill.exe"),
                              std::string(), bp::file::Path(),
                              killArgs, &status);
}


vector<string>
bp::process::getCommandLineArgs()
{
    vector<string> vsRet;

    int nArgs;
    LPWSTR* waArgs = CommandLineToArgvW( GetCommandLineW(), &nArgs );
    if (waArgs) {
        for (int i=0; i<nArgs; ++i) {
            vsRet.push_back( bp::strutil::wideToUtf8( waArgs[i] ) );
        }

        LocalFree( waArgs );
    }

    return vsRet;
}
