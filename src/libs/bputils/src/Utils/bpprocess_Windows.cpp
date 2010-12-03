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


// TODO
// * env handling
// * optional close/return handles?
// * figure out how to reliably change process name so that sTitle appears
//   in the "Activity Monitor" or "Task Manager" 
bool
bp::process::spawn(const boost::filesystem::path& path,
                   const vector<string>& vsArgs,
                   spawnStatus* status,
                   const boost::filesystem::path& workingDirectory,
                   const std::string& sTitle,
                   bool inheritWin32StdHandles)
{
	// get args into writable C buf needed by CreateProcessW()
    std::wstring wsArgs;
    wsArgs.append(L"\"" + path.native() + L"\" ");
    wsArgs.append(escapeArgs(vsArgs));
    wchar_t* wsBuf = _wcsdup(wsArgs.c_str());
    if (!wsBuf) {
        BPLOG_ERROR("memory allocation failure");
        return false;
    }

    // Setup CreateProcess.
    BOOL inheritHandles = inheritWin32StdHandles ? TRUE : FALSE;
    std::wstring dir = workingDirectory.native();
    const wchar_t* pDir = dir.empty() ? NULL : dir.c_str();
    STARTUPINFO sinfo;
    ZeroMemory(&sinfo, sizeof(sinfo));
    sinfo.dwFlags = STARTF_USESHOWWINDOW;
    if (inheritWin32StdHandles) {
        sinfo.dwFlags |= STARTF_USESTDHANDLES;
        sinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }
    sinfo.wShowWindow = SW_HIDE;
    sinfo.cb = sizeof(sinfo);
    PROCESS_INFORMATION pinfo;
    ZeroMemory(&pinfo, sizeof(pinfo));
    
    BOOL bRet = ::CreateProcessW(path.native().c_str(),
                                 wsBuf,
                                 NULL,              // process attributes
                                 NULL,              // thread attributes
                                 inheritHandles,    // inherit handles
                                 CREATE_NO_WINDOW,  // creation flags
                                 NULL,              // environment
                                 pDir,              // working directory
                                 &sinfo,            // starup info
                                 &pinfo);           // process information

    if (bRet) {
        if (status) {
            status->errCode = 0;
            status->pid = pinfo.dwProcessId;
            status->handle = pinfo.hProcess;
        }
    } else {
        if (status) {
            status->errCode = GetLastError();
            status->pid = 0;
            status->handle = 0;
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
    return bp::process::spawn(boost::filesystem::path(L"taskkill.exe"), killArgs,
                              &status);
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
