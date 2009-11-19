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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  bpfile_Windows.cpp
 *
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "shlobj.h"
#include <windows.h>
#include <ShellApi.h>
#include <shlobj.h>
#include <atlpath.h>
#include <io.h>
#include <sys/stat.h>

#include "api/BPLog.h"
#include "api/bperrorutil.h"
#include "api/bpfile.h"
#include "api/bpstrutil.h"
#include "api/bpsync.h"

using namespace std;

// Capture common link junk for readLink and resolveLink
// They only differ in whether resolution is attempted.
static bp::file::Path
doReadLink(const bp::file::Path& path, 
           bool resolve)
{
    bp::file::Path rval;
    IShellLinkW* psl = NULL; 
	IPersistFile* ppf = NULL;

    for (;;) {  // don't worry, all paths break
		HRESULT hr = CoInitialize(NULL);
		if (FAILED(hr)) break;

		// Get a pointer to the IShellLink interface. 
		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
							  IID_IShellLinkW, (LPVOID*)&psl); 
		if (FAILED(hr)) break;

		// Get a pointer to the IPersistFile interface. 
		hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf); 
		if (FAILED(hr)) break;

		// Load the shortcut. 
		hr = ppf->Load((wchar_t*)path.external_file_string().c_str(), STGM_READ); 
		if (FAILED(hr)) break;

        if (resolve) {
    		// Resolve the link (msdn lies, return value is S_FALSE on failure,
            // not an error value)
		    hr = psl->Resolve(NULL, HIWORD(300) | LOWORD(SLR_NO_UI));
		    if (hr != S_OK) break;
        }

		// Get the path to the link target.
		WCHAR szGotPath[MAX_PATH]; 
		hr = psl->GetPath(szGotPath, 
						  sizeof(szGotPath) / sizeof(szGotPath[0]), 
						  NULL, SLGP_UNCPRIORITY); 
		if (hr != S_OK) break;
        rval = szGotPath;
        break;
    };
		
	if (ppf) ppf->Release();
	if (psl) psl->Release();

	CoUninitialize();

	return rval;
}


string
bp::file::utf8FromNative(const tString& native)
{
    return bp::strutil::wideToUtf8(native);
}


bp::file::tString
bp::file::nativeFromUtf8(const string& utf8)
{
    return bp::strutil::utf8ToWide(utf8);
}


bp::file::Path
bp::file::canonicalPath(const Path& path,
                        const Path& root)
{
    Path rval = root;
    if (root.empty() 
        && PathIsRelativeW((wchar_t*) path.external_file_string().c_str())) {
        wchar_t* curDir = _wgetcwd(NULL, 0);
        if (!curDir) return path;
        rval = curDir;
        free(curDir);
    } 
    if (rval.empty()) return path;

    rval /= path;

    return rval;
}


bp::file::Path
bp::file::canonicalProgramPath(const Path& path,
                               const Path& root)
{
    Path name = canonicalPath(path, root);
    (void) name.replace_extension(L".exe");
    return name;
}


bp::file::Path
bp::file::getTempDirectory()
{
    Path tempDir;
    WCHAR buf[MAX_PATH] = {0};
    if (!::GetTempPathW(MAX_PATH, buf)) {
        BP_THROW_FATAL("::GetTempPathW fails");
    }
    tempDir = buf;
    return tempDir;
}




bool 
bp::file::linkExists(const Path& path)
{
    Path target = doReadLink(path, false);
    return !target.empty();
}


bool
bp::file::createLink(const Path& path,
					 const Path& target)
{
	bool rval = false;
    IShellLinkW* psl = NULL; 
	IPersistFile* ppf = NULL;

	try {
		HRESULT hr = CoInitialize(NULL);
		if (FAILED(hr)) {
			throw string("CoInitialize() failed");
		}

		// Get a pointer to the IShellLink interface. 
		hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
							  IID_IShellLinkW, (LPVOID*)&psl); 
		if (FAILED(hr)) {
			throw string("unable to get IShellLinkW interface");
		}

		// Get a pointer to the IPersistFile interface. 
		hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf); 
		if (FAILED(hr)) {
			throw string("unable to get IPersistFile interface");
		}

		// set shortcut path
		hr = psl->SetPath(target.external_file_string().c_str());
		if (FAILED(hr)) {
			throw string("unable to set shortcut");
		}

		// persist it
		hr = ppf->Save((LPCOLESTR) path.external_file_string().c_str(), TRUE);
		if (FAILED(hr)) {
			throw string("unable to persist shortcut");
		}
		rval = true;

	} catch (const string& s) {
        BPLOG_WARN_STRM("createLink(" << path << ", " << target
                        << ") failed: " << s);
		rval = false;
	}
		
	if (ppf) ppf->Release();
	if (psl) psl->Release();

	CoUninitialize();

	return rval;
}


bp::file::Path
bp::file::readLink(const Path& path)
{
    return doReadLink(path, false);
}
	

bool
bp::file::resolveLink(const Path& path,
                      Path& target)
{
    target = doReadLink(path, true);
    return !target.empty();
}


bp::file::Path
bp::file::getTempPath(const Path& tempDir,
                      const string& prefix)
{
    wstring wprefix = nativeFromUtf8(prefix);
	wchar_t outBuf[MAX_PATH];
	UINT x = GetTempFileNameW(tempDir.external_file_string().c_str(),
                              wprefix.c_str(), 0, outBuf);
	if (!x) BP_THROW_FATAL("GetTempFileNameW fails");
    Path path(outBuf);
    (void) remove(path);    
	return path;
}


bool
bp::file::touch(const Path& path)
{
    HANDLE file = NULL;
    file = CreateFileW(path.external_file_string().c_str(), GENERIC_WRITE,
                       FILE_SHARE_DELETE | FILE_SHARE_READ |
                       FILE_SHARE_WRITE, NULL,
                       OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
                           
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Gets the current system time
    SYSTEMTIME st;
    FILETIME ft;
    bool success = false;

    GetSystemTime(&st);              
    SystemTimeToFileTime(&st, &ft);  
    
    success = SetFileTime(file, (LPFILETIME) NULL, (LPFILETIME) NULL, 
                          &ft);    

    CloseHandle(file);

    return success;
}
