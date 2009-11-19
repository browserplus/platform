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
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <sys/time.h>

#include <iostream>

#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#else
#include <stdio.h>
#endif

#include "api/bpfile.h"
#include "api/bpstrutil.h"
#include "api/BPLog.h"

using namespace std;
namespace bfs = boost::filesystem;


#ifndef MACOSX
bp::file::Path
bp::file::getTempDirectory()
{
    return Path("/tmp");
}
#endif


bp::file::tString
bp::file::nativeFromUtf8(const string& s) 
{
    return s;
}


string
bp::file::utf8FromNative(const tString& s) 
{
    return s;
}


bp::file::Path
bp::file::getTempPath(const Path& tempDir,
                      const tString& prefix)
{
    Path rval;
    Path p = tempDir / Path(prefix + "XXXXXX");
    char* tmpl = new char[p.string().size() + 1];
    strcpy(tmpl, p.string().c_str());
    char* s = ::mktemp(tmpl);
    if (!s) BP_THROW_FATAL("::mktemp fails");
    rval = s;
    delete[] s;
    return rval;
}


bp::file::Path
bp::file::canonicalPath(const Path& path,
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
            
        char buf[PATH_MAX];
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


bp::file::Path
bp::file::canonicalProgramPath(const Path& path,
                               const Path& root)
{
    // No action beyond canonicalName necessary on unix
    return canonicalPath(path, root);
}


bool 
bp::file::linkExists(const Path& path)
{
    if (bfs::is_symlink(path)) {
        return true;
    }

#ifdef MACOSX
    // aliases appear as regular files
    if (bfs::is_regular(path)) {
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
bp::file::createLink(const Path& path,
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


bp::file::Path
bp::file::readLink(const Path& path)
{
	Path rval;
    char buf[PATH_MAX];
    int r = ::readlink(path.external_file_string().c_str(), buf, sizeof(buf));
    if (r > 0) {
        buf[r] = '\0';
        rval = buf;
    }
#ifdef MACOSX
    // aliases appear as regular files
    if (rval.empty()) {
        FSRef ref;
        if (FSPathMakeRef((const UInt8*)path.external_file_string().c_str(),
                          &ref, NULL) == noErr) {
            Boolean isFolder = false, wasAliased = false;
            (void) FSResolveAliasFile(&ref, true, &isFolder,
                                      &wasAliased);
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


bool
bp::file::resolveLink(const Path& path,
                      Path& target)
{
    bool rval = false;
    Path rstr = readLink(path);
    if (!rstr.empty()) {
        rstr = canonicalPath(rstr, path.parent_path());
        rval = bfs::exists(rstr);
    }
    if (rval) {
        target = rstr;
    } else {
        target.clear();
    }
    return rval;
}


bool
bp::file::touch(const Path& path)
{
    if (bfs::exists(path)) {
        return (utimes(path.external_file_string().c_str(), NULL) == 0);
    }

    if (!bfs::is_directory(path.parent_path())) {
        return false;
    }

    int fd = open(path.external_file_string().c_str(),
                  O_EXCL | O_CREAT | O_WRONLY);
    if (fd < 0) return false;
    close(fd);

    return true;
}
