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
#define BPLOG_DEBUG_STRM(x)
#define BPLOG_INFO_STRM(x)
#define BPLOG_WARN_STRM(x)
#define BPLOG_ERROR_STRM(x)
#endif

#pragma warning(disable:4101)

#include <windows.h>

using namespace std;

namespace bp { namespace file {


static Path
readShortcut(const Path& path)
{
    Path rval;

    // shortcuts on windows don't work unless they have the .lnk suffix
    if (path.extension().compare(L".lnk") != 0) {
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
		hr = ppf->Load((wchar_t*)path.external_file_string().c_str(), STGM_READ); 
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


string
utf8FromNative(const tString& native)
{
    string rval;
    // See how much space we need.
    // TODO: On Vista, it might be nice to specify WC_ERR_INVALID_CHARS
    int nChars = WideCharToMultiByte(CP_UTF8, 0, native.c_str(), -1,
                                     0, 0, 0, 0);

    // Do the conversion.
    char* paBuf = new char[nChars];
    int nRtn = WideCharToMultiByte(CP_UTF8, 0, native.c_str(), -1,
                                   paBuf, nChars, 0, 0);

    if (nRtn==0) {
        delete[] paBuf;
        boost::system::error_code ec(GetLastError(),
                                     boost::system::system_category);
        throw tFileSystemError("WideCharToMultiByte failed",
                               Path(native), Path(), ec);
    } else {
        rval = paBuf;
        delete[] paBuf;
    }
    return rval;
}


tString
nativeFromUtf8(const string& utf8)
{
    wstring rval;
    // See how much space we need.
    int nChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), 
                                     -1, 0, 0);

    // Do the conversion.
    wchar_t* pawBuf = new wchar_t[nChars];
    int nRtn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), 
                                   -1, pawBuf, nChars);
    if (nRtn==0) {
        delete[] pawBuf;
        boost::system::error_code ec(GetLastError(),
                                     boost::system::system_category);
        throw tFileSystemError("MultiByteToWideChar failed", Path(utf8), Path(), ec);
    } else {
        rval = pawBuf;
        delete[] pawBuf;
    }
    return rval;
}


Path
canonicalPath(const Path& path,
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
    rval = rval.canonical();
    return rval;
}


Path
canonicalProgramPath(const Path& path,
                     const Path& root)
{
    Path name = canonicalPath(path, root);
    (void) name.replace_extension(L".exe");
    return name;
}


Path
getTempDirectory()
{
    Path tempDir;
    WCHAR buf[MAX_PATH] = {0};
    if (!GetTempPathW(MAX_PATH, buf)) {
        boost::system::error_code ec(GetLastError(),
                                     boost::system::system_category);
        throw tFileSystemError("GetTempPathW fails", Path(), Path(), ec);
    }
    tempDir = buf;
    tempDir /= "YahooBrowserPlus";
    boost::filesystem::create_directories(tempDir);
    return tempDir;
}




bool
isSymlink(const Path& p)
{
    bool rval = false;
    WIN32_FIND_DATAW findData;
    HANDLE h = FindFirstFileW(p.external_file_string().c_str(), &findData);
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
isLink(const Path& path)
{
    if (isSymlink(path)) {
        return true;
    }

    // shortcuts have .lnk suffix
    return extension(path) == L".lnk";
}


bool
createLink(const Path& path,
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


bool
resolveLink(const Path& path,
            Path& target)
{
    target.clear();
    if (isSymlink(path)) {
        HANDLE h = CreateFileW(path.external_file_string().c_str(),
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


Path
getTempPath(const Path& tempDir,
            const string& prefix)
{
    wstring wprefix = nativeFromUtf8(prefix);
	wchar_t outBuf[MAX_PATH];
	UINT x = GetTempFileNameW(tempDir.external_file_string().c_str(),
                              wprefix.c_str(), 0, outBuf);
    if (!x) {
        boost::system::error_code ec(GetLastError(),
                                     boost::system::system_category);
        throw tFileSystemError("GetTempFileNameW fails",
                               tempDir, Path(prefix), ec);
    }
    Path path(outBuf);
    (void) remove(path);    
	return path;
}


bool
touch(const Path& path)
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
    
    success = SetFileTime(file, (LPFILETIME) NULL,
                          (LPFILETIME) NULL, &ft);    

    CloseHandle(file);

    return success;
}


bool
statFile(const Path& p,
         FileInfo& fi)
{
    // init to zero
    ZeroMemory(&fi, sizeof(fi));

	if (p.empty()) return false;

    HANDLE h = NULL;
    try {
        // strip off trailing slash
        tString nativePath = p.external_file_string();
        if (nativePath[nativePath.size() - 1] == '\\') {
            nativePath.erase(nativePath.size() - 1);
        }
        struct _stat s;
        if (_wstat(nativePath.c_str(), &s) != 0) {
            throw string("stat(" + p.utf8() + ") failed");
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
            throw string("CreateFileW(" + p.utf8() + ") failed");
        }
        BY_HANDLE_FILE_INFORMATION info;
        ZeroMemory(&info, sizeof(info));
        if (!GetFileInformationByHandle(h, &info)) {
            throw string("GetFileInformation(" + p.utf8() + ") failed");
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
setFileProperties(const Path& p,
                  const FileInfo& fi)
{
    if (p.empty()) return false;

    tString nativePath = p.external_file_string();

    DWORD attr = GetFileAttributesW(nativePath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) attr = 0;
    bool readOnly = ((fi.mode & 0200) == 0 && (fi.mode & 020) == 0 
                     && (fi.mode & 02) == 0);
    if (readOnly) {
        attr |= FILE_ATTRIBUTE_READONLY;
    } else {
        attr &= ~FILE_ATTRIBUTE_READONLY;
    }
    if (!SetFileAttributesW(nativePath.c_str(), attr)) {
        BPLOG_WARN_STRM("SetFileAttribute(" << p
                        << ", " << attr << ") failed: "
                        << bp::error::lastErrorString());
    }

    // set file times
    try {
        boost::filesystem::last_write_time(p, fi.mtime);
    } catch(const tFileSystemError&) {
        // empty
    }
    return true;
}

Path
programPath()
{
    Path rv("");
    WCHAR szFilename[(MAX_PATH * 4) + 1];
    memset(szFilename, 0, sizeof(szFilename));
    if (0 != GetModuleFileNameW(NULL, szFilename, (MAX_PATH * 4)))
    {
        rv = szFilename
    }
    return rv.canonical();
}


}}
