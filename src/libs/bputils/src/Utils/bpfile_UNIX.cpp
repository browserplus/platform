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
 *  bpfile_UNIX.cpp
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>

#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#else
#include <stdio.h>
#endif

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

using namespace std;
namespace bfs = boost::filesystem;

namespace bp { namespace file {

static Path
readLink(const Path& path)
{
    Path rval;
    char buf[PATH_MAX+1];
    int r = ::readlink(path.external_file_string().c_str(), buf, sizeof(buf));
    if (r > 0) {
        int index = r < PATH_MAX ? r : PATH_MAX;
        buf[index] = '\0';
        rval = buf;
    }
#ifdef MACOSX
    // aliases appear as regular files
    if (rval.empty()) {
        FSRef ref;
        if (FSPathMakeRef((const UInt8*)path.external_file_string().c_str(),
                          &ref, NULL) == noErr) {
            Boolean isFolder = false, wasAliased = false;
            (void) FSResolveAliasFile(&ref, true, &isFolder, &wasAliased);
            if (wasAliased) {
                if (FSRefMakePath(&ref, (UInt8*)buf, sizeof(buf)) == noErr) {
                    rval = buf;
                }
            }
        }
    }
#endif
	return rval;
}


#ifndef MACOSX
Path
getTempDirectory()
{
    return Path("/tmp");
}
#endif


tString
nativeFromUtf8(const string& s) 
{
    return s;
}


string
utf8FromNative(const tString& s) 
{
    return s;
}


Path
getTempPath(const Path& tempDir,
            const tString& prefix)
{
    Path rval;
    Path p = tempDir / Path(prefix + "XXXXXX");
    char* tmpl = new char[p.string().size() + 1];
    strcpy(tmpl, p.string().c_str());
    char* s = ::mktemp(tmpl);
    if (!s) {
        boost::system::error_code ec(errno, boost::system::system_category);
        throw tFileSystemError("::mktemp fails", tempDir, Path(prefix), ec);
    }
    rval = s;
    delete[] s;
    return rval;
}


Path
canonicalPath(const Path& path,
              const Path& root)
{
    string rval;
    int cfd = -1;
    try {
        if (!root.empty()) {
            cfd = ::open(".", O_RDONLY, 0);
            if (cfd < 0) {
                throw string("unable to open .");
            }
            if (::chdir(root.external_directory_string().c_str()) < 0) {
                throw string("unable to chdir to " + root.externalUtf8());
            }
        }
            
        char buf[PATH_MAX+1];
        if (::realpath(path.external_file_string().c_str(), buf) == NULL) {
            throw string("realpath failed on " + string(buf));
        }
        rval = buf;
    } catch(const string& s) {
        rval.clear();
    }
    if (cfd >= 0) {
        fchdir(cfd);
        ::close(cfd);
    }
    return rval;
}


Path
canonicalProgramPath(const Path& path,
                     const Path& root)
{
    // No action beyond canonicalName necessary on unix
    return canonicalPath(path, root);
}


bool 
isSymlink(const Path& path)
{
    try {
        return bfs::is_symlink(path);
    } catch(const tFileSystemError& e) {
        BPLOG_DEBUG_STRM("bfs::is_symlink(" << path << ") failed.");
        BPLOG_INFO_STRM("bfs::is_symlink failed: " << e.what() <<
                        ", returning false.");
        return false;
    }
}


bool 
isLink(const Path& path)
{
    if (isSymlink(path)) {
        return true;
    }

#ifdef MACOSX
    // aliases appear as regular files
    if (isRegularFile(path)) {
        FSRef ref;
        if (FSPathMakeRef((const UInt8*)path.external_file_string().c_str(),
                          &ref, NULL) == noErr) {
            Boolean isAlias = false, isFolder = false;
            if (FSIsAliasFile(&ref, &isAlias, &isFolder) == noErr) {
                return isAlias;
            }
        }
    }
#endif
    return false;	
}


bool
createLink(const Path& path,
           const Path& target)
{
    try {
        bfs::create_symlink(target, path);
    } catch(const bfs::basic_filesystem_error<bfs::path>& e) {
        BPLOG_WARN_STRM("createLink(" << path << ", "
                        << target << ") failed: "
                        << e.what());
        return false;
    }
    return true;
}


bool
resolveLink(const Path& path,
            Path& target)
{
    bool rval = false;
    Path rstr = readLink(path);
    if (!rstr.empty()) {
        rstr = canonicalPath(rstr, path.parent_path());
        rval = exists(rstr);
    }
    if (rval) {
        target = rstr;
    } else {
        target.clear();
    }
    return rval;
}


bool
touch(const Path& path)
{
    if (exists(path)) {
        return (utimes(path.external_file_string().c_str(), NULL) == 0);
    }

    if (!isDirectory(path.parent_path())) {
        return false;
    }

    int fd = open(path.external_file_string().c_str(),
                  O_EXCL | O_CREAT | O_WRONLY, 0644);
    if (fd < 0) return false;
    close(fd);

    return true;
}


bool
statFile(const Path& p,
         FileInfo& fi)
{
    // init to zero
    memset(&fi, 0, sizeof(fi));

	if (p.empty()) return false;

    struct stat s;
    tString nativePath = p.external_file_string();
    if (::stat(nativePath.c_str(), &s) != 0) return false;

    // set times
#ifdef MACOSX
    fi.mtime = s.st_mtimespec.tv_sec;      
    fi.ctime = s.st_ctimespec.tv_sec;
    fi.atime = s.st_atimespec.tv_sec;
#elif defined(LINUX)
    fi.mtime = s.st_mtime;      
    fi.ctime = s.st_ctime;
    fi.atime = s.st_atime;
#else
#error "unsupported platform"
#endif

    // set mode and size
    fi.mode = s.st_mode;
    fi.sizeInBytes = s.st_size;

    // set device and file ids
    fi.deviceId = s.st_rdev;
    fi.fileIdLow = s.st_ino;

    return true;
}


bool
setFileProperties(const Path& p,
                  const FileInfo& fi)
{
	if (p.empty()) return false;

    tString nativePath = p.external_file_string();

    chmod(nativePath.c_str(), fi.mode);

    // set file times
    try {
        bfs::last_write_time(p, fi.mtime);
    } catch(const tFileSystemError&) {
        // empty
    }
    return true;
}


}}
