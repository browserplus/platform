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
 *  bpfile.h
 *
 *  Add a few convenience methods for dealing with boost::filesystem

 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _BPFILE_H_
#define _BPFILE_H_

// Make sure we get v3 of boost::filesystem, no deprecated methods
//
#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED

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


namespace bp {
    namespace file {

        extern const std::string kFileUrlPrefix;   // "file://"

        // mimetypes that we add
        extern const std::string kFolderMimeType;  // "application/x-folder"
        extern const std::string kLinkMimeType;    // "application/x-link"
        extern const std::string kBadLinkMimeType; // "application/x-badlink"

        /** Get a string representinng the native pathname.  Needed since 
         *  boost::filesystem::native() will leave / seperators on doze. 
         *  Not quite sure what "native" means to boost.  Sigh.
         *  \param p [IN] - pathname
         *  \returns - native string for path
         */
        boost::filesystem::path::string_type nativeString(const boost::filesystem::path& p);

        /** Get a utf8 string representinng the native pathname.  Needed since 
         *  boost::filesystem::native() will leave / seperators on doze. 
         *  Not quite sure what "native" means to boost.  Sigh.
         *  \param p [IN] - pathname
         *  \returns - native utf8 string for path
         */
        std::string nativeUtf8String(const boost::filesystem::path& p);

        // Regular and a recursive path visitors.
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
            virtual tResult visitNode(const boost::filesystem::path& p,
                                      const boost::filesystem::path& relativePath) = 0;
        };

        /** Non-recursive visit of nodes in a path.  If path is a directory,
         *  it will not be visited, but its children will.
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
        bool visit(const boost::filesystem::path& p,
                   IVisitor& v,
                   bool followLinks);

        /** Recursive visit of nodes in a path.  If path is a directory,
         *  both it and its children will be visited.  Link cycle detection
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
        bool recursiveVisit(const boost::filesystem::path& p,
                            IVisitor& v,
                            bool followLinks);

        /** Construct a path from a file:// url.  The url is expected
         *  to be urlencoded from utf8.
         *  \param url [IN] - url 
         *  \returns - a path for the file represented
         *             by the url.  If url is malformed,
         *             returned path is empty()
         */
        boost::filesystem::path pathFromURL(const std::string& url);

        /** return a file:// url for a path.  The url will be
         *  urlencoded utf8
         *  \param p [IN] - path
         *  \returns - a url for the file represented
         *             by the path.
         */
        std::string urlFromPath(const boost::filesystem::path& p);

        /** return canonical form a a path, resolves any "." and ".."
         *  \param p [IN] - path, need not exist
         *  \returns - canonical form of p
         */
        boost::filesystem::path canonical(const boost::filesystem::path& p);

        /** return a path relative to another path
         *  \param p [IN] - path for which to get relative name
         *  \param base [IN] - path that name will be relative to
         *  \returns - name of "p" relative to "base", throws bpfs::filesystem_error
         *             if "p" is not relative to "base"
         */
        boost::filesystem::path relativeTo(const boost::filesystem::path& p,
                                           const boost::filesystem::path& base);

        /** Return the absolute form of a path.
         *  Uses boost::filesystem::system_complete().
         *  Returns empty if that method throws an exception. 
         *  \param path [IN] - path
         *  \returns canonical form of path, empty on failure
         */
        boost::filesystem::path absolutePath(const boost::filesystem::path& path);
        
        /** Return the absolute form of an executable program name, which
         *  may include appending ".exe" on win32, etc.
         *  Uses boost::filesystem::system_complete().
         *  Returns empty if that method throws an exception. 
         *  \param path [IN] - path
         *  \returns canonical form of path, empty on failure
         */
        boost::filesystem::path absoluteProgramPath(const boost::filesystem::path& path);

        /** Return the full path of the current executing program.
         *  \returns full path of the current executing program
         */
        boost::filesystem::path programPath();

        /** Get a path within the specified dir that in not in use.
         *  \param tempDir [IN] - path to directory
         *  \param prefix [IN] - prefix to append to resulting path name
         *  \returns - path within tempDir (throws boost::filesystem_error on failure)
         */
        boost::filesystem::path getTempPath(const boost::filesystem::path& tempDir,
                                            const std::string& prefix); 

        /**  Get path to user's temporary directory.
         *   \returns   path to user's temporary directory (throws on failure)
         */
        boost::filesystem::path getTempDirectory();

        /** Delete a file or directory (directory delete is recursive)
         *  Deleting a non-existent path succeeds.  Attempts to forcibly
         *  remove read-only files/dirs.
         *  \param      path [IN] - directory path
         *  \returns    true upon success
         */
        bool safeRemove(const boost::filesystem::path& path);
        
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
        bool safeCopy(const boost::filesystem::path& fromPath,
					  const boost::filesystem::path& toPath,
					  bool followLinks = true);
        
        /** Move a file or dir to a new location.  Tries
         *  a copy/delete if rename() fails.
         *  \param      fromPath [IN] - source path
         *  \param      toPath [IN] - destination path
         *  \returns    true upon success
         */
        bool safeMove(const boost::filesystem::path& fromPath,
					  const boost::filesystem::path& toPath);
        
        /** Remove files which could not be deleted.  This happens
         *  on Windows XP when saveMove() tries to move something
         *  which is being virus scanned.  In that case, we fallback 
         *  to a copy/delete.  If the delete fails, we mark the path
         *  for delayed deletion.  Call delayDelete() at shutdown 
         *  to make one last attempt to remove the cruft.
         */
        void delayDelete();

        /** Returns file size (0 for directories).
         *  Uses boost::filesystem::file_size(), returning
         *  0 if that method throws an exception
         *  \param      path [IN] - sourc path
         *  \returns    file size
         */
        boost::uintmax_t size(const boost::filesystem::path& path);

        /** Whether the item specified by the path exists.
         *  Uses boost::filesystem::exists().
         *  Returns false if that method throws an exception. 
         *  \param		path [IN] - source path
         *  \returns	true if path item exists
         */
        bool pathExists(const boost::filesystem::path& path);

        /** Whether the item specified by the path is a directory.
         *  Uses boost::filesystem::is_directory().
         *  Returns false if that method throws an exception. 
         *  \param		path [IN] - source path
         *  \returns	true if path item is a directory
         */
        bool isDirectory(const boost::filesystem::path& path);

        /** Whether the item specified by the path is a regular file.
         *  Uses boost::filesystem::is_regular_file().
         *  Returns false if that method throws an exception. 
         *  \param      path [IN] - source path
         *  \returns    true if path item is a regular file
         */
        bool isRegularFile(const boost::filesystem::path& path);

        /** Whether the item specified by the path is an "other" type
         *  of file.  Uses boost::filesystem::is_other().
         *  Returns false if that method throws an exception.
         *  \param      path [IN] - source path
         *  \returns    true if path item is an "other" file
         */
        bool isOther(const boost::filesystem::path& path);

        /** Does a path refer to a Unix or NTFS symlink?
         *  \param		path [IN] - source path
         *  \returns	true if path is a link
         */
        bool isSymlink(const boost::filesystem::path& path);
        
        /** Does a path refer to a link?  In addition to Unix/NTFS
         *  symlinks, understands Mac aliases and Windows shortcuts.
         *  \param		path [IN] - source path
         *  \returns	true if path is a link
         */
        bool isLink(const boost::filesystem::path& path);
        
        /** Create a Unix symlink or Windows shortcut.
         *   \param     path [IN] - link path
         *   \param     target [IN] - link target, need not exist
         *   \returns   true on success
         */
        bool createLink(const boost::filesystem::path& path,
                        const boost::filesystem::path& target);

        /** Resolve a link to a valid path.  Understands Unix/NTFS symlinks,
         *  Mac aliases, and Windows shortcuts.  
         *  \param		path [IN] - link path
         *  \param      target [OUT] - valid path (empty on failure)
         *  \returns	true if target exists
         */
        bool resolveLink(const boost::filesystem::path& path,
                         boost::filesystem::path& target);

        /** Similar to unix touch(1) - create an empty file if
         *  the path doesn't exist (but the parent path does),
         *  or if the path does exist and points to a file, update
         *  the file's modtime
         */
        bool touch(const boost::filesystem::path& path);

        bool openReadableStream(std::ifstream& fstream,
                                const boost::filesystem::path& path,
                                int flags);
 
        bool openWritableStream(std::ofstream& fstream,
                                const boost::filesystem::path& path,
                                int flags);

        struct FileInfo 
        {
            FileInfo() : mode(0), mtime(0), ctime(0), atime(0), 
                         sizeInBytes(0), deviceId(0),
                         fileIdHigh(0), fileIdLow(0) {
            }
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
        bool statFile(const boost::filesystem::path& path,
                      FileInfo& fi);

        /** change file properties to match a FileInfo structure 
         */        
        bool setFileProperties(const boost::filesystem::path& path,
                               const FileInfo& fi);

        bool makeReadOnly(const boost::filesystem::path& path);

        // What are a path's mimetypes?  If there is
        // an "official" mimetype (xxx/vnd.yyy), it
        // will be first.
        std::vector<std::string> mimeTypes(const boost::filesystem::path& path);

        // is a path one of the specified mimetypes?
        bool isMimeType(const boost::filesystem::path& path,
                        const std::set<std::string>& filter);

        std::vector<std::string> extensionsFromMimeType(const std::string& mimeType);
    }
}

#endif
