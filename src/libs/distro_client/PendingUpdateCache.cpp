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

/**
 * PendingUpdateCache - an abstraction around the disk based cache
 *                      of pending updates
 */

#include "PendingUpdateCache.h"
#include <stack>
#include "BPUtils/bpfile.h"
#include "BPUtils/ProductPaths.h"
#include "CoreletUnpacker.h"


std::list<bp::service::Summary>
PendingUpdateCache::cached()
{
    std::list<bp::service::Summary> currentCorelets;

    std::stack<bp::file::Path> dirStack;

    dirStack.push(bp::paths::getCoreletCacheDirectory());
    
    while (dirStack.size() > 0) {
        bp::file::Path path(dirStack.top());
        dirStack.pop();

        if (bp::file::isDirectory(path)) {
            try {
                bp::file::tDirIter end;
                for (bp::file::tDirIter it(path); it != end; ++it) {            
                    bp::file::Path p(it->path());
                    
                    // silently skip dot directories
                    bp::file::tString dot = bp::file::nativeFromUtf8(".");
                    if (p.filename().compare(0, 1, dot) == 0) continue;
                    
                    if (!bp::file::isDirectory(p)) continue;
                    
                    // check to see if this is a valid corelet
                    std::string error;
                    bp::service::Summary summary;
                    
                    if (!summary.detectCorelet(p, error)) {
                        dirStack.push(p);
                        continue;
                    }
                    
                    // cool, got a summary, add it to the set of currently
                    // available corelets.
                    currentCorelets.push_back(summary);
                }
            } catch (bp::file::tFileSystemError& e) {
                BPLOG_WARN_STRM("unable to iterate thru " << path
                                << ": " << e.what());
            }
        }
    }

    return currentCorelets;
}

bool
PendingUpdateCache::save(std::string name, std::string version,
                         const std::vector<unsigned char> & buf)
{
    // unpack & install, aborting everything if this fails. 
    CoreletUnpacker unpacker(
        buf, bp::paths::getCoreletCacheDirectory(), name, version);

    std::string errMsg;
                
    if (!unpacker.unpack(errMsg)) {
        BPLOG_ERROR_STRM("Error unpacking " << name << " corelet: "
                         << errMsg << ", ABORTING cache update");
        return false;
    }

    if (!unpacker.install(errMsg)) {
        BPLOG_ERROR_STRM("Error installing " << name 
                         << " corelet: "  << errMsg
                         << ", ABORTING cache update");
        return false;
    }

    return true;
}

bool
PendingUpdateCache::purge()
{
    return bp::file::remove(
        bp::paths::getCoreletCacheDirectory());
}

bool
PendingUpdateCache::install(std::string name, std::string version)
{
    bp::file::Path source = bp::paths::getCoreletCacheDirectory() / name / version;
    bp::file::Path dest = bp::paths::getCoreletDirectory()/ name /version;
    bp::file::Path shortDest = bp::paths::getCoreletDirectory() / name;

    // if source doesn't exist, fail
    if (!bp::file::isDirectory(source)) return false;

    // kill dest if it exists
    (void) bp::file::remove(dest);

    // create dest
    try {
        boost::filesystem::create_directories(shortDest);
    } catch(const bp::file::tFileSystemError&) {
        BPLOG_WARN_STRM("unable to create " << shortDest);
        return false;
    }
    
    // move it!
    return bp::file::move(source, dest);
}

bool
PendingUpdateCache::isCached(std::string name, std::string version)
{
    bp::service::Summary s;
    return getSummary(name, version, s);
}

bool 
PendingUpdateCache::getSummary(std::string name, std::string version,
                               bp::service::Summary & summary)
{
    std::list<bp::service::Summary> cached = PendingUpdateCache::cached();
    std::list<bp::service::Summary>::const_iterator it;

    for (it = cached.begin(); it != cached.end(); it++)
    {
        if (!it->name().compare(name) &&
            !it->version().compare(version))
        {
            summary = *it;
            return true;
        }
    }

    return false;
}
