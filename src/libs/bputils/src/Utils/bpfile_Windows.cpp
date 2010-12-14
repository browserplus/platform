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
 *  bpfile_Windows.cpp
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#include "shlobj.h"
#include <windows.h>
#include <ShellApi.h>
#include <shlobj.h>
#include <atlpath.h>
#include <io.h>
#include <sys/stat.h>

#include "api/bpfile.h"

#ifdef BP_PLATFORM_BUILD
#include "api/BPLog.h"
#include "api/bperrorutil.h"
#else
#define BPLOG_DEBUG(x)
#define BPLOG_INFO(x)
#define BPLOG_WARN(x)
#define BPLOG_ERROR(x)
#define BPLOG_DEBUG_STRM(x)
#define BPLOG_INFO_STRM(x)
#define BPLOG_WARN_STRM(x)
#define BPLOG_ERROR_STRM(x)
#endif

#pragma warning(disable:4101)

#include <windows.h>

using namespace std;
namespace bfs = boost::filesystem;

namespace bp { namespace file {


static bfs::path
readShortcut(const bfs::path& path)
{
    bfs::path rval;

    // shortcuts on windows don't work unless they have the .lnk suffix
    if (path.extension().string().compare(".lnk")) {
        return rval;
    }
    IShellLinkW* psl = NULL; 
	IPersistFile* ppf = NULL;

    do {
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
		hr = ppf->Load((wchar_t*) nativeString(path).c_str(), STGM_READ); 
		if (FAILED(hr)) break;

        // Resolve the link (msdn lies, return value is S_FALSE on failure,
        // not an error value)
        hr = psl->Resolve(NULL, HIWORD(300) | LOWORD(SLR_NO_UI));
        if (hr != S_OK) break;

		// Get the path to the link target.
		WCHAR szGotPath[MAX_PATH]; 
		hr = psl->GetPath(szGotPath, 
						  sizeof(szGotPath) / sizeof(szGotPath[0]), 
						  NULL, SLGP_UNCPRIORITY); 
		if (hr != S_OK) break;
        rval = szGotPath;
        break;
    } while(false);
		
	if (ppf) ppf->Release();
	if (psl) psl->Release();

	CoUninitialize();

	return rval;
}


bfs::path
canonicalPath(const bfs::path& path,
              const bfs::path& root)
{
    bfs::path rval = root;
    if (root.empty() 
        && PathIsRelativeW((wchar_t*) nativeString(path).c_str())) {
        wchar_t* curDir = _wgetcwd(NULL, 0);
        if (!curDir) return path;
        rval = curDir;
        free(curDir);
    } 
    if (rval.empty()) return path;

    rval /= path;
    rval = canonical(rval);
    return rval;
}


bfs::path
canonicalProgramPath(const bfs::path& path,
                     const bfs::path& root)
{
    bfs::path name = canonicalPath(path, root);
    (void) name.replace_extension(".exe");
    return name;
}


bfs::path
getTempDirectory()
{
    bfs::path tempDir;
    WCHAR buf[MAX_PATH] = {0};
    if (!GetTempPathW(MAX_PATH, buf)) {
        boost::system::error_code ec(GetLastError(),
                                     boost::system::system_category());
        throw bfs::filesystem_error("GetTempPathW fails", bfs::path(), bfs::path(), ec);
    }
    tempDir = buf;
    tempDir /= "YahooBrowserPlus";
    bfs::create_directories(tempDir);
    return tempDir;
}




bool
isSymlink(const bfs::path& p)
{
    bool rval = false;
    WIN32_FIND_DATAW findData;
    HANDLE h = FindFirstFileW(nativeString(p).c_str(), &findData);
    if (h) {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            && (findData.dwReserved0 == IO_REPARSE_TAG_SYMLINK)) {
            rval = true;
        }
        FindClose(h);
    }
    return rval;
}

    
bool 
isLink(const bfs::path& path)
{
    if (isSymlink(path)) {
        return true;
    }

    // shortcuts have .lnk suffix
    return path.extension().string() == ".lnk";
}


bool
createLink(const bfs::path& path,
           const bfs::path& target)
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
		hr = psl->SetPath(nativeString(target).c_str());
		if (FAILED(hr)) {
			throw string("unable to set shortcut");
		}

		// persist it
		hr = ppf->Save((LPCOLESTR) nativeString(path).c_str(), TRUE);
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


bool
resolveLink(const bfs::path& path,
            bfs::path& target)
{
    target.clear();
    if (isSymlink(path)) {
        HANDLE h = CreateFileW(nativeString(path).c_str(),
                               0, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                               NULL, OPEN_EXISTING, 
                               FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            return false;
        }
        HMODULE hm = LoadLibrary(_TEXT("kernel32.dll"));
        if (hm != NULL) {
            typedef DWORD (WINAPI *PGFPNBY)(HANDLE, LPTSTR, DWORD, DWORD);
            PGFPNBY fp = (PGFPNBY) GetProcAddress(hm, "GetFinalPathNameByHandleW");
            if (fp) {
                wchar_t buf[32768];
                ZeroMemory(buf, sizeof(buf));
                DWORD ret = (*fp)(h, buf, sizeof(buf), FILE_NAME_NORMALIZED);
                if (ret != 0) {
                    // returned pathname uses \\?\ syntax, strip that
                    target = &buf[4];
                }
            }
            FreeLibrary(hm);
        }
        CloseHandle(h);
    } else {
        // now try shortcuts
        target = readShortcut(path);
    }
    return !target.empty();
}


bfs::path
getTempPath(const bfs::path& tempDir,
            const string& prefix)
{
    bfs::path prefixPath(prefix);
	wchar_t outBuf[MAX_PATH];
	UINT x = GetTempFileNameW(nativeString(tempDir).c_str(),
                              nativeString(prefixPath).c_str(), 0, outBuf);
    if (!x) {
        boost::system::error_code ec(GetLastError(),
                                     boost::system::system_category());
        throw bfs::filesystem_error("GetTempFileNameW fails",
                                    tempDir, prefixPath, ec);
    }
    bfs::path path(outBuf);
    (void) safeRemove(path);
	return path;
}


bool
touch(const bfs::path& path)
{
    HANDLE file = NULL;
    file = CreateFileW(nativeString(path).c_str(), GENERIC_WRITE,
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
    
    success = SetFileTime(file, (LPFILETIME) NULL,
                          (LPFILETIME) NULL, &ft);    

    CloseHandle(file);

    return success;
}


bool
statFile(const bfs::path& p,
         FileInfo& fi)
{
    // init to zero
    ZeroMemory(&fi, sizeof(fi));

	if (p.empty()) return false;

    HANDLE h = NULL;
    try {
        // strip off trailing slash
        wstring nativePath = nativeString(p);
        if (nativePath[nativePath.size() - 1] == '\\') {
            nativePath.erase(nativePath.size() - 1);
        }
        struct _stat s;
        if (_wstat(nativePath.c_str(), &s) != 0) {
            throw string("stat(" + p.string() + ") failed");
        }

        // set times
        fi.mtime = s.st_mtime;      
        fi.ctime = s.st_ctime;
        fi.atime = s.st_atime;

        // set mode - on windows we'll default to 0644, if readonly is set,
        // we'll turn off 0200
        s.st_mode = 0644;
        DWORD attr = GetFileAttributesW(nativePath.c_str());
        if (attr & FILE_ATTRIBUTE_READONLY) s.st_mode &= ~0200;

        // set mode and size
        fi.mode = s.st_mode;
        fi.sizeInBytes = s.st_size;

        // Get device and file ids.  Note that we must use
        // FILE_FLAG_BACKUP_SEMANTICS to get a handle to a dir.
        // Intuitive.
        h = CreateFileW(nativePath.c_str(),
                        0, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, 
                        FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            throw string("CreateFileW(" + p.string() + ") failed");
        }
        BY_HANDLE_FILE_INFORMATION info;
        ZeroMemory(&info, sizeof(info));
        if (!GetFileInformationByHandle(h, &info)) {
            throw string("GetFileInformation(" + p.string() + ") failed");
        }
        fi.deviceId = info.dwVolumeSerialNumber;
        fi.fileIdHigh = info.nFileIndexHigh;
        fi.fileIdLow = info.nFileIndexLow;
        CloseHandle(h);
    
    } catch (const string& s) {
        BPLOG_WARN_STRM(s << ": " << bp::error::lastErrorString());
        if (h) CloseHandle(h);
        return false;
    }
    return true;
}


bool
setFileProperties(const bfs::path& p,
                  const FileInfo& fi)
{
    if (p.empty()) return false;

    DWORD attr = GetFileAttributesW(nativeString(p).c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) attr = 0;
    bool readOnly = ((fi.mode & 0200) == 0 && (fi.mode & 020) == 0 
                     && (fi.mode & 02) == 0);
    if (readOnly) {
        attr |= FILE_ATTRIBUTE_READONLY;
    } else {
        attr &= ~FILE_ATTRIBUTE_READONLY;
    }
    if (!SetFileAttributesW(nativeString(p).c_str(), attr)) {
        BPLOG_WARN_STRM("SetFileAttribute(" << p
                        << ", " << attr << ") failed: "
                        << bp::error::lastErrorString());
    }

    // set file times
    try {
        boost::filesystem::last_write_time(p, fi.mtime);
    } catch(const bfs::filesystem_error&) {
        // empty
    }
    return true;
}

bfs::path
programPath()
{
    bfs::path rv("");
    WCHAR szFilename[(MAX_PATH * 4) + 1];
    memset(szFilename, 0, sizeof(szFilename));
    if (0 != GetModuleFileNameW(NULL, szFilename, (MAX_PATH * 4)))
    {
        rv = szFilename;
    }
    return canonical(rv);
}


}}
