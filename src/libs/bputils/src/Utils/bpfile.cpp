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
 *  bpfile.cpp
 *
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */

#include <set>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "api/bpfile.h"
#include "api/BPLog.h"
#include "api/bpstrutil.h"
#include "api/bpurl.h"
#include "BPUtils/bpmimetype.h"

#ifdef WIN32
// boost::algorithm::is_any_of causes vs to whine
#pragma warning(disable:4996 4512)
#endif

#include "boost/filesystem/fstream.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#ifdef WIN32
#include <windows.h>
// deal with Windows naming...
#define tStat struct _stat
#define stat(x, y) _wstat(x, y)
#define chmod(x, y) _wchmod(x, y)
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#else
#define tStat struct stat
#endif

using namespace std;
using namespace std::tr1;
namespace bfs = boost::filesystem;

namespace bp { namespace file {

#define FILE_URL_PREFIX "file://"

static set<Path> s_delayDelete;

#ifdef WIN32
Path::Path(const std::string& utf8) : tBase(bp::file::nativeFromUtf8(utf8)) 
{
}
#endif


string 
Path::utf8() const
{
    return utf8FromNative(string());
}


string 
Path::externalUtf8() const
{
    return utf8FromNative(external_file_string());
}


string 
Path::url() const
{
    // Split out edges.  Note that if "s" started
    // with "/", first edge will be empty.
    std::string s = utf8();
    if (s.empty()) {
        return std::string("");
    };
    vector<std::string> edges;
    boost::algorithm::split(edges, s, boost::algorithm::is_any_of("/"));
    std::string rval(FILE_URL_PREFIX);
    unsigned int start = 0;
    if (edges[0].empty()) {
        start++;
    }
#ifdef WIN32
    // Windows hosts appear differently in pathname (//host/path)
    // and must appear in final url as file://host/path.  Also
    // want drive names, which appear as C:/foo, to appear in
    // url as file:///C:/foo
    if (edges[start].length() == 0) {
        // have a host, e.g. //host/path
        rval.append(edges[++start]);
        start++;
    } else if (edges[start].length() == 2 && edges[start][1] == ':') {
        // have a drive, e.g. c:/foo
        rval.append("/" + edges[start]);
        start++;
    }
#endif

    // add remaining edges
    for (unsigned int i = start; i < edges.size(); i++) {
        if (edges[i].length() > 0) {
            rval.append("/");
            rval.append(bp::url::urlEncode(edges[i]));
        }
    }
    return rval;
}


Path 
Path::canonical() const
{
    tString dot = nativeFromUtf8(".");
    tString dotdot = nativeFromUtf8("..");
    Path rval;
    for (iterator iter(begin()); iter != end(); ++iter) {
        if (iter->compare(dot) == 0) {
            continue;
        }
        if (iter->compare(dotdot) == 0) {
            rval = rval.parent_path();
            continue;
        }
        rval = rval / *iter;
    }

    // make sure we preserve a trailing /
    tString slash = nativeFromUtf8("/");
    if (string().rfind(slash) == string().length()-1) {
        tString s = rval.string();
        s += slash;
        rval = s;
    }
    return rval;
}



Path
Path::relativeTo(const Path& base) const
{
    std::string baseStr = base.utf8();
    std::string ourStr = utf8();
    if (baseStr.rfind("/") != baseStr.length()-1) {
        baseStr += "/";
    }
    if (ourStr.find(baseStr) != 0) {
        BP_THROW(externalUtf8() + " is not relative to "
                 + base.externalUtf8());
    }
    tString relStr = nativeFromUtf8(ourStr.substr(baseStr.length(), string::npos));
    return Path(relStr);
}


//-------------------------- convenience functions


bool
visit(const Path& p,
      IVisitor& v)
{
    if (v.visitNode(p) == IVisitor::eStop) {
        return false;
    }
    if (bfs::is_directory(p)) {
        tDirIter end;
        for (tDirIter iter(p); iter != end; ++iter) {
            Path node(iter->path());
            if (v.visitNode(node) == IVisitor::eStop) {
                return false;
            }
        }
    }
    return true;
}


bool
recursiveVisit(const Path& p,
               IVisitor& v)
{
    if (v.visitNode(p) == IVisitor::eStop) {
        return false;
    }
    if (bfs::is_directory(p)) {
        tRecursiveDirIter end;
        for (tRecursiveDirIter iter(p); iter != end; ++iter) {
            Path node(iter->path());
            switch (v.visitNode(node)) {
            case IVisitor::eOk:
                break;
            case IVisitor::eStop:
                return false;
            case IVisitor::eStopRecursion:
                iter.no_push();
                break;
            }
        }
    }
    return true;
}


Path 
pathFromURL(const string& url)
{
    Path rval;
    
    // file url format is file://host/path

    // check for file://
    if (url.substr(0, strlen(FILE_URL_PREFIX)) != FILE_URL_PREFIX) {
        return rval;
    }

    // Rip off file:// and get remaining edges.
    // Note that if "s" started with "/", 
    // first edge will be empty.  Also, Windows 
    // handles hosts with //host/path.  No uniform
    // way to do hosts on other platforms, so bail.
    string s = url.substr(strlen(FILE_URL_PREFIX));
    vector<string> edges;
    boost::algorithm::split(edges, s, boost::algorithm::is_any_of("/"));
    unsigned int start = 0;
    if (edges[0].empty()) {
        start++;
    }

    string firstEdge = bp::url::urlDecode(edges[start]);
    if (s[0] == '/') {
#ifdef WIN32
        // no host, check for drive
        if (firstEdge.length() == 2 && firstEdge[1] == ':') {
            rval = Path(firstEdge + "/");
        } else {
            rval = Path("/" + firstEdge);
        }
#else
        rval = Path("/" + firstEdge);
#endif
        start++;
    } else {
        // got a host.  everybody skips localhost and 127.0.0.1,
        // doze groks //host/path
        if (!firstEdge.compare("localhost") || !firstEdge.compare("127.0.0.1")) {
            rval = Path("/");
        } else {
#ifdef WIN32
            rval = Path("//" + firstEdge);
#else
            return rval;
#endif
        }
        start++;
    }

    // add remaining edges
    for (size_t i = start; i < edges.size(); i++) {
        rval /= Path(bp::url::urlDecode(edges[i]));
    }

    return rval;
}


static bool
unsetReadOnly(const Path& path)
{
    bool rval = false;
    FileInfo fi;
    if (statFile(path, fi)) {
        if ((fi.mode & 0200) == 0) {
            fi.mode |= 0200;
            if (setFileProperties(path, fi)) {
                rval = true;
            } else {
                BPLOG_DEBUG_STRM("setFileProperties(" << path
                                 << ") failed: " 
                                 << bp::error::lastErrorString());
            }
        }
    } else {
        BPLOG_DEBUG_STRM("statFile(" << path
                         << ") failed: " 
                         << bp::error::lastErrorString());
    }
    return rval;
}


BPTime
modTime(const Path& path)
{
    Path p = path;
    if (linkExists(path)) {
        if (!resolveLink(path, p)) {
            return 0L;
        }
    }
    
    time_t mt = 0;
    try {
        mt = bfs::last_write_time(path);
    } catch(const tFileSystemError& e) {
        BPLOG_WARN_STRM("modTime(" << path << 
                        ") failed: " << e.what());
        mt = 0;
    }
    return BPTime(mt);
}


bool
statFile(const Path& p,
         FileInfo& fi)
{
	if (p.empty()) return false;

    // init to zero
    fi.mode = 0;
    fi.mtime.set(0);      
    fi.ctime.set(0);
    fi.atime.set(0);
    fi.sizeInBytes = 0;

    tStat s;

    tString nativePath = p.external_file_string();
#ifdef WIN32
	// strip off trailing slash
	if (nativePath[nativePath.size() - 1] == '\\') {
		nativePath.erase(nativePath.size() - 1);
	}
    if (::stat(nativePath.c_str(), &s) != 0) return false;

	// set times
    fi.mtime.set(s.st_mtime);      
    fi.ctime.set(s.st_ctime);
    fi.atime.set(s.st_atime);

    // set mode - on windows we'll default to 0644, if readonly is set,
    // we'll turn off 0200
    s.st_mode = 0644;
    DWORD attr = GetFileAttributesW(nativePath.c_str());
    if (attr & FILE_ATTRIBUTE_READONLY) s.st_mode &= ~0200;
#else
    if (::stat(nativePath.c_str(), &s) != 0) return false;

    // set times
#ifdef MACOSX
    fi.mtime.set(s.st_mtimespec.tv_sec);      
    fi.ctime.set(s.st_ctimespec.tv_sec);
    fi.atime.set(s.st_atimespec.tv_sec);
#elif defined(LINUX)
    fi.mtime.set(s.st_mtime);      
    fi.ctime.set(s.st_ctime);
    fi.atime.set(s.st_atime);
#else
#error "unsupported platform"
#endif

#endif

    // set mode and size
    fi.mode = s.st_mode;
    fi.sizeInBytes = s.st_size;

    return true;
}


bool
setFileProperties(const Path& p,
                  const FileInfo& fi)
{
	if (p.empty()) return false;

    tString nativePath = p.external_file_string();

#ifdef WIN32
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
        BPLOG_WARN_STRM("::SetFileAttribute(" << p
                        << ", " << attr << ") failed: "
                        << bp::error::lastErrorString());
    }

#else
    chmod(nativePath.c_str(), fi.mode);
#endif

    // set file times
    try {
        bfs::last_write_time(p, fi.mtime.get());
    } catch(const tFileSystemError&) {
        // empty
    }
    return true;
}


bool 
remove(const Path& path)
{

    // easy, path doesn't exist
    if (path.empty() || !bfs::exists(path)) {
        return true; 
    }

    bool rval = false;
    try {
        bfs::remove_all(path);
        rval = true;
    } catch(const tFileSystemError&) {
        // bfs::remove_all can fail if anything is read-only.
        // Thus, we'll recursively remove the attribute and try again.
        BPLOG_DEBUG_STRM("bfs::remove_all(" << path
                         << ") failed, removing read-only attributes "
                         << "and trying again");
        FileInfo fi;
        if (bfs::is_directory(path)) {
            tRecursiveDirIter end;
            for (tRecursiveDirIter it(path); it != end; ++it) {
                Path p(it->path());
                (void) unsetReadOnly(p);
            }
        } else {
            (void) unsetReadOnly(path);
        }
        // Ability to delete p depends upons permissions on 
        // p's parent.  Try to give parent write permission,
        // then restore old permission when done.
        FileInfo parentInfo;
        Path parent = path.parent_path();
        bool setParentInfo = statFile(parent, parentInfo)
                             && unsetReadOnly(parent);
        try {
            bfs::remove_all(path);
            rval = true;
        } catch(const tFileSystemError& e) {
            BPLOG_WARN_STRM("bfs::remove_all(" << path
                            << ") failed: " << e.what());
        }
        if (setParentInfo) {
            setFileProperties(parent, parentInfo);
        }
    }
    return rval;
}


bool
move(const Path& from,
     const Path& to)
{
    try {
        bfs::rename(from, to);
    } catch(const tFileSystemError& e) {
        BPLOG_WARN_STRM("move(" << from
                        << ", " << to << ") failed: " << e.what()
                        << ", trying copy/delete");
        if (!copy(from, to)) {
            BPLOG_WARN_STRM("copy failed");
            return false;
        }
        if (!remove(from)) {
            BPLOG_WARN_STRM("delete after copy failed: " << from
                            << ", adding for delayed delete");
            s_delayDelete.insert(from);
        }
    }
    return true;
}


void
delayDelete()
{
    set<Path>::iterator it;
    for (it = s_delayDelete.begin(); it != s_delayDelete.end(); ++it) {
        Path p(*it);
        BPLOG_DEBUG_STRM("attempt to delete " << p);
        (void) remove(p);
    }
}


static void
copyDir(const Path& from,
        const Path& to)
{
    tString fromStr = from.string();
    tRecursiveDirIter end;
    for (tRecursiveDirIter it(from); it != end; ++it) {
        Path relPath = Path(it->path()).relativeTo(from);
        Path target = to / relPath;
        if (bfs::is_directory(it->path())) {
            bfs::create_directories(target);
        } else {
            bfs::copy_file(it->path(), target);
        }
    }
}


bool 
copy(const Path& src,
     const Path& dst,
     bool followLinks)
{
    try {
        // fail on bad args
        if (src.empty() || dst.empty()) {
            return false;
        }

        // fail if source doesn't exist
        if (!bfs::exists(src)) {
            return false;
        }
    
        // fail if destination file exists (no implicit overwrite)
        if (bfs::exists(dst) && !bfs::is_directory(dst)) {
            return false;
        }

        // chase links if asked, broken links cause failure
        Path from = src;
        if (followLinks && linkExists(src)) {
            if (!resolveLink(src, from)) {
                return false;
            } 
        }
        Path to = dst;
        if (followLinks && linkExists(dst)) {
            if (!resolveLink(dst, to)) {
                return false;
            } 
        }

        // trailing / on dirs ok, but not on files
        string fromStr = from.utf8();
        if (fromStr.rfind("/") == fromStr.length()-1) {
            if (bfs::is_directory(from)) {
                from = fromStr.substr(0, fromStr.length()-1);
            } else {
                return false;
            }
        }
        string toStr = to.utf8();
        if (toStr.rfind("/") == toStr.length()-1) {
            if (bfs::exists(to) && !bfs::is_directory(to)) {
                return false;
            }
            to = toStr.substr(0, toStr.length()-1);
        }

        if (bfs::is_directory(from)) {
            // source is a directory
            Path target = to;
            if (bfs::is_directory(to)) {
                // copy into dest, creating new dir with
                // basename of source
                target = to / from.filename();
                bfs::create_directory(target);
            } else if (bfs::is_directory(to.parent_path())) {
                // copy source to new dir, losing original basename
                bfs::create_directory(target);
            } else {
                // dest isn't a dir, nor is it's parent. fail
                return false;
            }
            copyDir(from, target);
        } else {
            // source is file
            if (src.utf8().rfind("/") == src.utf8().length() - 1) {
                // no trailing / allowed in src
                return false;
            }
            Path target = to;
            if (bfs::is_directory(to)) {
                // dest is directory, copy preserving filename
                target = to / from.filename();
            } else if (!bfs::is_directory(to.parent_path())) {
                // dest isn't a dir, nor is it's parent. fail
                return false;
            }
            bfs::copy_file(from, target);
        }
    } catch(tFileSystemError& e) {
        BPLOG_ERROR_STRM("copy(" << src << ", " << dst
                         << " failed: " << e.what());
        return false;
    }
    return true;
}


bool
openReadableStream(ifstream& fstream,
                   const Path& path,
                   int flags)
{
    if (fstream.is_open()) {
        BPLOG_WARN_STRM("openReadableStream, stream already open");
        return false;
    }
    tString native = path.external_file_string();
#ifdef WIN32
    fstream.open(native.c_str(), ios::in | flags);
#else
    fstream.open(native.c_str(), ios::in | (_Ios_Openmode) flags);
#endif
    if (!fstream.is_open()) {
        BPLOG_WARN_STRM("openReadableStream, stream open failed for " << path);
        return false;
    }
    return true;
}


bool
openWritableStream(std::ofstream& fstream,
                   const Path& path,
                   int flags)
{
    if (fstream.is_open()) {
        BPLOG_WARN_STRM("openWritableStream, stream already open");
        return false;
    }
    tString native = path.external_file_string();
#ifdef WIN32
    fstream.open(native.c_str(), std::ios::out | flags);
#else
    fstream.open(native.c_str(), std::ios::out | (_Ios_Openmode) flags);
#endif

	// set user read/write permission if needed
    tStat sb;
    if (::stat(native.c_str(), &sb) != 0) {
        BPLOG_WARN_STRM("openWritableStream, unable to stat " << path);
        return false;
    }
    if ((sb.st_mode & (S_IRUSR|S_IWUSR)) != (S_IRUSR|S_IWUSR)) {
        if (::chmod(native.c_str(), S_IRUSR | S_IWUSR) != 0) {
            BPLOG_WARN_STRM("openWritableStream, unable to chmod " << path);
            return false;
        }
    }
    if (!fstream.is_open()) {
        BPLOG_WARN_STRM("openWritableStream, stream open failed for " << path);
        return false;
    }
    return true;
}

bool
makeReadOnly(const Path& path)
{
    // toggle bits to make this thing read only
    FileInfo fi;
    if (!statFile(path, fi)) {
        return false;
    }
    
    // turn off write bits (making the file _read_only_)
    fi.mode &= ~(0222);

    if (!setFileProperties(path, fi)) {
        return false;
    }
    
    return true;
}

}}

