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
 *  bpfile.h
 *
 *  Provide a boost::filesystem::[w]path subclass with 
 *  a few convenience methods. Its primary purpose is to provide
 *  some typedefs to hide whether path or wpath is being used.
 *  On windows, we use wide paths.  Everybody else uses utf8.
 *
 *  As with boost, bp::file::Path only does pathname manipulation
 *  stuff.  Any actual contact with the filesystem is done by 
 *  other functions in bp::file namespace.

 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _BPFILE_H_
#define _BPFILE_H_

#include <fstream>
#include <sys/stat.h>
#ifdef WIN32
#include <sys/utime.h>
#include <time.h>
#else
#include <utime.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"


namespace bp {
    namespace file {

        extern const std::string kFileUrlPrefix;      // "file://"

        // mimetypes that we add
        extern const std::string kFolderMimeType;     // "application/x-folder"
        extern const std::string kLinkMimeType;       // "application/x-link"
        extern const std::string kBadLinkMimeType;    // "application/x-badlink"

        // a bunch of typedefs to deal with the fact that Windows uses wpath and
        // everyone else uses path
#ifdef WIN32
        typedef boost::filesystem::wpath tBase;
        typedef std::wstring tString;
        typedef wchar_t tChar;
        typedef boost::filesystem::wdirectory_iterator tDirIter;
        typedef boost::filesystem::wrecursive_directory_iterator tRecursiveDirIter;
        typedef boost::filesystem::basic_filesystem_error<boost::filesystem::wpath> tFileSystemError;
#else
        typedef boost::filesystem::path tBase;
        typedef std::string tString;
        typedef char tChar;
        typedef boost::filesystem::directory_iterator tDirIter;
        typedef boost::filesystem::recursive_directory_iterator tRecursiveDirIter;
        typedef boost::filesystem::basic_filesystem_error<boost::filesystem::path> tFileSystemError;
#endif

        // Our path abstraction built on boost::filesystem.  We just add a handful
        // of convenience methods.
        class Path : virtual public tBase
        {
        public:
            // echoing tBase constructors
            Path() : tBase() {}
            Path(const tString& s) : tBase(s) {}
            Path(const tChar* s) : tBase(s) {}
            template <class InputIterator>
                Path(InputIterator s, InputIterator last) : tBase(s, last) {}
            Path(const tBase& o) : tBase(o.string()) {}
  
#ifdef WIN32    
            // Windows provides some convenience stuff to take utf8 pathnames.
            // Path("foo") is easier that Path(nativeFromUtf8("foo"))
            Path(const std::string& utf8);
            inline Path& Path::operator=(const std::string& s) {
                *this = Path(s);
                return *this;
            }
            inline Path& Path::operator=(const char* s) {
                *this = Path(s);
                return *this;
            }
            inline Path& Path::operator/=(const Path& p) {
                *this = *this / p;
                return *this;
            }
            inline Path& Path::operator/=(const std::string& s) {
                *this = *this / Path(s);
                return *this;
            }
            inline Path& Path::operator/=(const char* s) {
                *this = *this / Path(s);
                return *this;
            }
#endif

            // reset to empty (documented but not implemented by bfs!)
            void clear() { *this = Path(); }

            // return a file:// url for this path
            std::string url() const;

            // convenience method for utf8FromNative(string())
            std::string utf8() const; 

            // convenience method for utf8FromNative(external_file_string())
            std::string externalUtf8() const; 

            // canonical form, resolves any "." and ".."
            Path canonical() const;

            // return a Path relative to another Path
            Path relativeTo(const Path& base) const; // throws bp::Exception
        };

        // Operations on Paths in addition to what boost::filesystem provides

        // ease dumping to streams
        template<typename _CharT, class _Traits> 
        std::basic_ostream<_CharT, _Traits>& 
        operator<<(std::basic_ostream<_CharT, _Traits>& os, const Path& p) {
            os << p.externalUtf8();
            return os;
        }

        // Regular and a recursive Path visitors.
        // The visit() method will visit each node of a directory,
        // calling the specified IVisitor's visit() method.
        // The recursive version just recurses.
        //
        class IVisitor {
        public:
            typedef enum {
                eOk,              // node processed
                eStop,            // terminate the entire visit
                eStopRecursion,   // don't recurse into this node
            } tResult;

            virtual ~IVisitor() {}

            /** Visit a node.
             *  \param p [IN] - real path of node
             *  \param relativePath [IN] - pseudo-path of node relative
             *                             to top node.  For shorcuts
             *                             and aliases, this will not be
             *                             valid relative path.
             *  \returns - a tResult which tells visit()/recursiveVisit()
             *             now to proceed
             */
            virtual tResult visitNode(const Path& p,
                                      const Path& relativePath) = 0;
        };

        /** Non-recursive visit of node in a path.
         *  \param p [IN] - path to visit
         *  \param v [IN] - visitor to apply to each node
         *  \param followLinks [IN] - should links be followed?  If false,
         *                            the link itself will be visited.
         *                            If true and a link is valid, the
         *                            link target will be visited.
         *                            If true and a link is broken,
         *                            the link itself will be visited.
         *  \returns - true if all nodes visited, false
         *             if "v" stopped the visit
         */
        bool visit(const Path& p,
                   IVisitor& v,
                   bool followLinks);

        /** Recursive visit of node in a path.  Links cycle detection
         *  is performed and cycles are not revisited.
         *  \param p [IN] - path to visit
         *  \param v [IN] - visitor to apply to each node
         *  \param followLinks [IN] - should links be followed?  If false,
         *                            the link itself will be visited.
         *                            If true and a link is valid, the
         *                            link target will be visited.
         *                            If true and a link is broken,
         *                            the link itself will be visited.
         *  \returns - true if all nodes visited, false
         *             if "v" stopped the visit
         */
        bool recursiveVisit(const Path& p,
                            IVisitor& v,
                            bool followLinks);

        /** Construct a Path from a file:// url.
         *  \param url [IN] - url 
         *  \returns - a Path for the file represented
         *             by the url.  If url is malformed,
         *             returned path is empty()
         */
        Path pathFromURL(const std::string& url);

        /** Return a utf8 string from a native string
         *  \param native [IN] - string in native (utf8 or wide) encoding
         *  \returns - utf8 equivalent
         */
        std::string utf8FromNative(const tString& native);

        /** Return a native string from a utf8 string
         *  \param utf8 [IN] - string in utf8 encoding
         *  \returns - native (utf8 or wide) equivalent
         */
        tString nativeFromUtf8(const std::string& utf8);

        /** Return the canonical form of a path
         *  \param path [IN] - path
         *  \param root [IN] - root of relative path evaluation.
         *                     Default (empty) means current dir.
         *  \returns canonical form of path, empty on failure
         */
        Path canonicalPath(const Path& path,
                           const Path& root = Path());
        
        /** Return the canonical form of an executable program name
         *  operations may include prepending root or current working
         *  directory, appending ".exe" on win32, etc.
         *  \param path [IN] - path
         *  \param root [IN] - root of relative path evaluation.
         *                     Default (empty) means current dir.
         *  \returns canonical form of path, empty on failure
         */
        Path canonicalProgramPath(const Path& path,
                                  const Path& root = Path());

        /** Get a path within the specified dir that in not in use.
         *  \param tempDir [IN] - path to directory
         *  \param prefix [IN] - prefix to append to resulting path name
         *  \returns - path within tempDir (empty on failure)
         */
        Path getTempPath(const Path& tempDir,
                         const std::string& prefix); 

        /**  Get path to user's temporary directory.
         *   \returns   path to user's temporary directory (empty on failure)
         */
        Path getTempDirectory();

        /** Delete a file or directory (directory delete is recursive)
         *  Deleting a non-existent path succeeds.  Attempts to forcibly
         *  remove read-only files/dirs.
         *  \param      path [IN] - directory path
         *  \returns    true upon success
         */
        bool remove(const Path& path);
        
        /** Copy a path to a new location.
         *  
         *  if the source path doesn't exist, the operation will fail.
         *
         *  if the source path is a file and...
         *    + the source path ends with '/' the operation will fail
         *    + the destination path is an existing file, the operation
         *      will fail.  overwriting doesn't occur implicitly.
         *    + the destination path is a directory, the source file will
         *      be copied to the destination directory with filename
         *      preserved.
         *    + the destination path has a non-existent basename who's
         *      parent is a directory, the source file will be copied to
         *      the new path and the original filename will be lost. 
         *
         *  if the source path is a directory...
         *    + the destination path is an existing file the operation
         *      will fail.
         *    + the destination path is an existing directory, the source
         *      path will be recursively copied into the destination
         *      directory, creating a new directory named with the basename
         *      of source
         *    + the destination path has a non-existent basename who's
         *      parent is a directory, the source dir will be copied to
         *      the new path and the original basename will be lost. 
         * 
         *  \param      fromPath [IN] - source path
         *  \param      toPath [IN] - destination path
         *  \param      followLinks [IN] - if true, links are chased
         *  \returns    true upon success
         */
        bool copy(const Path& fromPath,
                  const Path& toPath,
                  bool followLinks = true);
        
        /** Move a file or dir to a new location.  Tries
         *  a copy/delete if rename() fails.
         *  \param      fromPath [IN] - source path
         *  \param      toPath [IN] - destination path
         *  \returns    true upon success
         */
        bool move(const Path& fromPath, 
                  const Path& toPath);
        
        /** Remove files which could not be deleted.  This happens
         *  on Windows XP when move() tries to move something
         *  which is being virus scanned.  In that case, we fallback 
         *  to a copy/delete.  If the delete fails, we mark the path
         *  for delayed deletion.  Call delayDelete() at shutdown 
         *  to make one last attempt to remove the cruft.
         */
        void delayDelete();

        /** Does a path refer to a Unix or NTFS symlink?
         *  \param		path [IN] - source path
         *  \returns	true if path is a link
         */
        bool isSymlink(const Path& path);
        
        /** Does a path refer to a link?  In addition to Unix/NTFS
         *  symlinks, understands Mac aliases and Windows shortcuts.
         *  \param		path [IN] - source path
         *  \returns	true if path is a link
         */
        bool isLink(const Path& path);
        
        /** Create a Unix symlink or Windows shortcut.
         *   \param     path [IN] - link path
         *   \param     target [IN] - link target, need not exist
         *   \returns   true on success
         */
        bool createLink(const Path& path,
                        const Path& target);

        /** Resolve a link to a valid path.  Understands Unix/NTFS symlinks,
         *  Mac aliases, and Windows shortcuts.  
         *  \param		path [IN] - link path
         *  \param      target [OUT] - valid path (empty on failure)
         *  \returns	true if target exists
         */
        bool resolveLink(const Path& path,
                         Path& target);

        /** Similar to unix touch(1) - create an empty file if
         *  the path doesn't exist (but the parent path does),
         *  or if the path does exist and points to a file, update
         *  the file's modtime
         */
        bool touch(const Path& path);

        bool openReadableStream(std::ifstream& fstream,
                                const Path& path,
                                int flags);
 
        bool openWritableStream(std::ofstream& fstream,
                                const Path& path,
                                int flags);

        struct FileInfo 
        {
            // permissions - these are unix style permissions as
            // would be passed into chmod(3).  On windows we'll
            // perform a reasonable approximation.
            // (i.e. 0222 == readonly)
            unsigned int mode; 
            // last modification time
            std::time_t mtime;      
            // time of creation
            std::time_t ctime;
            // time of last access
            std::time_t atime;
            // size in bytes
            size_t sizeInBytes;
            // device identifier
            boost::uint32_t deviceId;
            // file identifier 
            boost::uint32_t fileIdHigh;
            boost::uint32_t fileIdLow;
        };

        /** get information about a file or directory on disk
         */
        bool statFile(const Path& path,
                      FileInfo& fi);

        /** change file properties to match a FileInfo structure 
         */        
        bool setFileProperties(const Path& path,
                               const FileInfo& fi);

        bool makeReadOnly(const Path& path);

        // what are a path's mimetypes?
        std::set<std::string> mimeTypes(const Path& path);

        // is a path one of the specified mimetypes?
        bool isMimeType(const Path& path,
                        const std::set<std::string>& filter);

        std::vector<std::string> extensionsFromMimeType(
            const std::string& mimeType);
    }
}

#ifdef WIN32
// More convenience for std::string and char*.  Note that these
// are not in the bp::file namespace, allowing them to be
// found by clients without requiring "using namespace bp::file"

inline bp::file::Path operator/(const bp::file::Path& lhs, const std::string& rhs) {
    return bp::file::Path(lhs) /= rhs;
}
inline bp::file::Path operator/(const bp::file::Path& lhs, const char* rhs) {
    return bp::file::Path(lhs) /= rhs;
}
inline bp::file::Path operator/(const std::string& lhs, const bp::file::Path& rhs) {
    return bp::file::Path(lhs) /= rhs;
}
inline bp::file::Path operator/(const char* lhs, const bp::file::Path& rhs) {
    return bp::file::Path(lhs) /= rhs;
}
// and now using a tBase, allows things like p.parent_path() / "foo"
inline bp::file::Path operator/(const bp::file::tBase& lhs, const std::string& rhs) {
    return bp::file::Path(lhs) /= rhs;
}
inline bp::file::Path operator/(const bp::file::tBase& lhs, const char* rhs) {
    return bp::file::Path(lhs) /= rhs;
}
inline bp::file::Path operator/(const std::string& lhs, const bp::file::tBase& rhs) {
     return bp::file::Path(lhs) /= rhs;
}
inline bp::file::Path operator/(const char* lhs, const bp::file::tBase& rhs) {
    return bp::file::Path(lhs) /= rhs;
}
#endif

#endif
