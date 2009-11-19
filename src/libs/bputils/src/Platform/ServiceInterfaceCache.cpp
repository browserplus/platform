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

/**
 *  ServiceInterfaceCache - wrappers around lower level bputils facilities
 *                          to give a high level API for caching serialized
 *                          corelet descriptions.
 *                          
 * (c) 2008 Yahoo!
 */

#include "BPUtils/bptime.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/ProductPaths.h"

#include "api/ServiceInterfaceCache.h"

static bp::file::Path buildPath(const std::string & name,
                                const std::string & version)
{
    bp::file::Path cachePath = bp::paths::getCoreletInterfaceCachePath();
    cachePath /= bp::file::Path(name + "_" + version + ".json");
    return cachePath;
}

bool
bp::serviceInterfaceCache::isNewerThan(const std::string & name,
                                       const std::string & version,
                                       const BPTime &t)
{
    if (name.empty() || version.empty()) return false;

    bp::file::Path path = buildPath(name, version);    
    return (boost::filesystem::exists(path)
            && bp::file::modTime(path).compare(t) >= 0);
}

  
bp::Object *
bp::serviceInterfaceCache::get(const std::string & name,
                               const std::string & version)
{
    if (name.empty() || version.empty()) return NULL;

    bp::file::Path path = buildPath(name, version);
    bp::Object * obj = NULL;

    if (boost::filesystem::exists(path)) {
        // read cache
        std::string jsonRep;
        if (bp::strutil::loadFromFile(path, jsonRep)) {
            obj = bp::Object::fromPlainJsonString(jsonRep);
        }
    }
    
    return obj;
}

bool
bp::serviceInterfaceCache::set(const std::string & name,
                               const std::string & version,
                               const bp::Object * obj)
{
    if (name.empty() || version.empty() || obj == NULL) return false;
    bp::file::Path path = buildPath(name, version);
    std::string jsonRep = obj->toPlainJsonString();
    return bp::strutil::storeToFile(path, jsonRep);
}

bool
bp::serviceInterfaceCache::purge(const std::string & name,
                                 const std::string & version)
{
    if (name.empty() || version.empty()) return false;

    bp::file::Path path = buildPath(name, version);

    return bp::file::remove( path );
}

