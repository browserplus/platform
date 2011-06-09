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
 *  bpfile_UNIX.cpp
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>

#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>	/* _NSGetExecutablePath */
#else
#include <stdio.h>
#endif

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

using namespace std;
namespace bfs = boost::filesystem;

namespace bp { namespace file {

static bfs::path
readLink(const bfs::path& path)
{
    bfs::path rval;
    char buf[PATH_MAX+1];
    int r = ::readlink(path.c_str(), buf, sizeof(buf));
    if (r > 0) {
        int index = r < PATH_MAX ? r : PATH_MAX;
        buf[index] = '\0';
        rval = buf;
    }
#ifdef MACOSX
    // aliases appear as regular files
    if (rval.empty()) {
        FSRef ref;
        if (FSPathMakeRef((const UInt8*)path.c_str(),
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


#ifdef MACOSX
static string
stringRefToUTF8(CFStringRef cfStr)
{
    string rval;
    CFIndex cfLen = CFStringGetLength(cfStr);

    if (cfLen > 0) {
        char stackBuffer[2048], *dynamicBuf = NULL;
        char * buf = stackBuffer;
        if ((size_t) (cfLen*4) >= sizeof(stackBuffer)) {
            dynamicBuf = (char*) malloc(cfLen*4 + 1);
            buf = dynamicBuf;
        }
        CFStringGetCString(cfStr, buf, cfLen*4 + 1,
                           kCFStringEncodingUTF8);
        rval.append(buf);
        if (dynamicBuf) free(dynamicBuf);
    }

    return rval;
}


bfs::path
getTempDirectory()
{
    bfs::path tempDir;
    FSRef fref;
    OSErr err = FSFindFolder(kUserDomain, kTemporaryFolderType, 
                             kCreateFolder, &fref);
    if (err == noErr) {
        CFURLRef tmpUrl = CFURLCreateFromFSRef(kCFAllocatorSystemDefault,
                                               &fref);
        if (tmpUrl != NULL) {
            CFStringRef ctmpDir = CFURLCopyFileSystemPath(tmpUrl,
                                                          kCFURLPOSIXPathStyle);
            tempDir = stringRefToUTF8(ctmpDir);
            CFRelease(ctmpDir);
            CFRelease(tmpUrl);
        } else  {
            boost::system::error_code ec(errno, boost::system::system_category());
            throw bfs::filesystem_error("Can't get temp dir", bfs::path(),
                                        bfs::path(), ec);
        }
    }

    tempDir /= "YahooBrowserPlus";
    boost::filesystem::create_directories(tempDir);
    return tempDir;
}
#else
bfs::path
getTempDirectory()
{
    bfs::path tempDir("/tmp/YahooBrowserPlus");
    boost::filesystem::create_directories(tempDir);
    return tempDir;
}
#endif


bfs::path
absoluteProgramPath(const bfs::path& path)
{
    return absolutePath(path);
}


bool 
isSymlink(const bfs::path& path)
{
    try {
        return bfs::is_symlink(path);
    } catch(const bfs::filesystem_error& e) {
        BPLOG_DEBUG_STRM("bfs::is_symlink(" << path << ") failed.");
        BPLOG_INFO_STRM("bfs::is_symlink failed: " << e.what() <<
                        ", returning false.");
        return false;
    }
}


bool 
isLink(const bfs::path& path)
{
    if (isSymlink(path)) {
        return true;
    }

#ifdef MACOSX
    // aliases appear as regular files
    if (isRegularFile(path)) {
        FSRef ref;
        if (FSPathMakeRef((const UInt8*)path.c_str(), &ref, NULL) == noErr) {
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
createLink(const bfs::path& path,
           const bfs::path& target)
{
    try {
        bfs::create_symlink(target, path);
    } catch(const bfs::filesystem_error& e) {
        BPLOG_WARN_STRM("createLink(" << path << ", "
                        << target << ") failed: "
                        << e.what());
        return false;
    }
    return true;
}


bool
resolveLink(const bfs::path& path,
            bfs::path& target)
{
    bool rval = false;
    int cfd = -1;
    bfs::path linkVal = readLink(path);
    try {
        if (!linkVal.empty()) {
            bfs::path parent = path.parent_path();
            if (!parent.empty()) {
                cfd = ::open(".", O_RDONLY, 0);
                if (cfd < 0) {
                    throw string("unable to open .");
                }
                if (::chdir(parent.c_str()) < 0) {
                    throw string("unable to chdir to ") + parent.c_str();
                }
            }
            char buf[PATH_MAX+1];
            if (::realpath(path.c_str(), buf) == NULL) {
                throw string("realpath failed");
            }
            target = buf;
            rval = pathExists(target);
        }
    } catch (const string&) {
        rval = false;
    }

    if (!rval) {
        target.clear();
    }

    if (cfd >= 0) {
        ::fchdir(cfd);
        ::close(cfd);
    }
    return rval;
}


bool
touch(const bfs::path& path)
{
    if (pathExists(path)) {
        return (utimes(path.c_str(), NULL) == 0);
    }

    if (!isDirectory(path.parent_path())) {
        return false;
    }

    int fd = open(path.c_str(), O_EXCL | O_CREAT | O_WRONLY, 0644);
    if (fd < 0) return false;
    close(fd);

    return true;
}


bool
statFile(const bfs::path& p,
         FileInfo& fi)
{
    // init to zero
    memset(&fi, 0, sizeof(fi));

	if (p.empty()) return false;

    struct stat s;
    if (::stat(p.c_str(), &s) != 0) return false;

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
setFileProperties(const bfs::path& p,
                  const FileInfo& fi)
{
	if (p.empty()) return false;

    chmod(p.c_str(), fi.mode);

    // set file times
    try {
        bfs::last_write_time(p, fi.mtime);
    } catch(const bfs::filesystem_error&) {
        // empty
    }
    return true;
}

bfs::path
programPath()
{
    bfs::path rv("");
#ifdef MACOSX
    char pathbuf[PATH_MAX + 1];
    char real_executable[PATH_MAX + 1];
    memset(pathbuf, 0, sizeof(pathbuf));
    memset(real_executable, 0, sizeof(real_executable));
    uint32_t bufsize = sizeof(pathbuf);
    _NSGetExecutablePath(pathbuf, &bufsize);
    rv = pathbuf;
#else // MACOSX
    // NEEDSWORK.  No reliable implementation across Unices that I could find for now.
#endif // MACOSX
    return canonical(rv);
}


}}
