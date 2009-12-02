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
 * Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/************************************************************************
* BPHandleMapper.cpp
*
* Written by Gordon Durand, Tue Sep 4 2007
* (c) 2007, Yahoo! Inc, all rights reserved.
*/

#include "BPHandleMapper.h"
#include <iostream>
#include "BPUtils/bpfile.h"

std::map<bp::file::Path, BPHandle> s_pathMap;
std::map<BPHandle, bp::file::Path> s_handleMap;

BPHandle 
BPHandleMapper::pathToHandle(const bp::file::Path& path)
 {
    static int sHandle = ::rand();

    std::map<bp::file::Path, BPHandle>::iterator it = s_pathMap.find(path);
    if (it != s_pathMap.end()) {
        return it->second;
    }

    // no known handle, add one
    std::string safeName = bp::file::utf8FromNative(path.filename());
    long size = boost::filesystem::is_regular_file(path) ?
        (long) boost::filesystem::file_size(path) : 0;
    std::set<std::string> mimeTypes = bp::file::mimeTypes(path);
    BPHandle h("BPTPath", sHandle++, safeName, size, mimeTypes);
    s_pathMap.insert(std::make_pair(path, h));
    s_handleMap.insert(std::make_pair(h, path));
    return h;
}


bp::file::Path
BPHandleMapper::handleValue(const BPHandle& handle)
{
    bp::file::Path rval;
    std::map<BPHandle, bp::file::Path>::iterator it = s_handleMap.find(handle);
    if (it != s_handleMap.end()) {
        if (it->first.type().compare(handle.type()) == 0
            && it->first.name().compare(handle.name()) == 0) {
            rval = it->second;
        }
    } 
    return rval;
}


bp::Object* 
BPHandleMapper::insertHandles(const bp::Object* bpObj)
{
    using namespace bp;
    
    Object* rval = NULL;
    switch(bpObj->type()) {
        case BPTNull:
            rval = new Null();
            break;
        case BPTBoolean:
            rval = new Bool(dynamic_cast<const Bool*>(bpObj)->value());
            break;
        case BPTString:
            rval = new String(dynamic_cast<const String*>(bpObj)->value());
            break;
        case BPTInteger:
            rval = new Integer(dynamic_cast<const Integer*>(bpObj)->value());
            break;
        case BPTCallBack:
            rval = new Integer(dynamic_cast<const CallBack*>(bpObj)->value());
            break;
        case BPTDouble:
            rval = new Double(dynamic_cast<const Double*>(bpObj)->value());
            break;
        case BPTPath:
        {
            // Path must become a map containing id/name keys
            const Path* pObj = dynamic_cast<const Path*>(bpObj);
            bp::file::Path path = bp::file::pathFromURL(pObj->value());
            BPHandle handle = pathToHandle(path);
            Map* m = new Map;
            m->add(BROWSERPLUS_HANDLETYPE_KEY, new String(handle.type()));
            m->add(BROWSERPLUS_HANDLEID_KEY, new Integer(handle.id()));
            // TODO: eventually kill this guy once everyone is platform 2.0.7
            //      or greater
            m->add(DEPRECATED_BROWSERPLUS_HANDLENAME_KEY, new String(handle.name()));
            m->add(BROWSERPLUS_HANDLENAME_KEY, new String(handle.name()));
            m->add(BROWSERPLUS_HANDLESIZE_KEY, new Integer(handle.size()));
            std::set<std::string> mt = handle.mimeTypes();
            std::set<std::string>::const_iterator it;
            List* l = new List;
            for (it = mt.begin(); it != mt.end(); ++it) {
                l->append(new String(it->c_str()));
            }
            m->add(BROWSERPLUS_HANDLEMIMETYPE_KEY, l);
            rval = m;
            break;
        }
        case BPTList:
        {
            const List* me = dynamic_cast<const List*>(bpObj);
            List* l = new List;
            for (unsigned int i = 0; i < me->size(); i++) {
                l->append(insertHandles(me->value(i)));
            }
            rval = l;
            break;
        }
        case BPTMap:
        {
            const Map* me = dynamic_cast<const Map*>(bpObj);
            Map* m = new Map;
            Map::Iterator iter(*me);
            const char* key = NULL;
            while ((key = iter.nextKey()) != NULL) {
                const Object* obj = me->value(key);
                Object* newObj = insertHandles(obj);
                m->add(key, newObj);
            }
            rval = m;
            break;
        }
        case BPTAny:
            // invalid
            break;
    }
    return rval;
}


bp::Object*
BPHandleMapper::expandHandles(const bp::Object* bpObj)
{
    using namespace bp;
    
    Object* rval = NULL;
    switch(bpObj->type()) {
        case BPTNull:
        case BPTBoolean:
        case BPTString:
        case BPTInteger:
        case BPTDouble:
        case BPTCallBack:
            rval = bpObj->clone();
            break;
        case BPTList:
        {
            const List* me = dynamic_cast<const List*>(bpObj);
            List* l = new List;
            for (unsigned int i = 0; i < me->size(); i++) {
                l->append(expandHandles(me->value(i)));
            }
            rval = l;
            break;
        }
        case BPTMap:
        {
            // Handles are a map with 3 special keys.  Expand them.
            const Map* me = dynamic_cast<const Map*>(bpObj);
            const String* typeObj = 
                dynamic_cast<const String*>(me->value(BROWSERPLUS_HANDLETYPE_KEY));
            const Integer* idObj = 
                dynamic_cast<const Integer*>(me->value(BROWSERPLUS_HANDLEID_KEY));
            const String* nameObj = 
                dynamic_cast<const String*>(me->value(BROWSERPLUS_HANDLENAME_KEY));
            const Integer* sizeObj = 
                dynamic_cast<const Integer*>(me->value(BROWSERPLUS_HANDLESIZE_KEY));
            const List* mimeTypeObj = 
                dynamic_cast<const List*>(me->value(BROWSERPLUS_HANDLEMIMETYPE_KEY));
            if (typeObj && idObj && nameObj && sizeObj && mimeTypeObj) {
                std::set<std::string> mt;
                for (unsigned int i = 0; i < mimeTypeObj->size(); ++i) {
                    const String* s = 
                        dynamic_cast<const String*>(mimeTypeObj->value(i));
                    if (s) {
                        mt.insert(s->value());
                    }
                }
                BPHandle h(typeObj->value(), (int) idObj->value(),
                           nameObj->value(), (long) sizeObj->value(),
                           mt);
                bp::file::Path val = handleValue(h);
                if (!val.empty()) {
                    rval = new Path(val);
                }
            } else {
                // just a vanilla map, recurse into it
                Map* m = new Map;
                Map::Iterator iter(*me);
                const char* key = NULL;
                while ((key = iter.nextKey()) != NULL) {
                    const Object* obj = me->value(key);
                    Object* newObj = expandHandles(obj);
                    m->add(key, newObj);
                }
                rval = m;
            }
            break;
        }
        case BPTPath:
        case BPTAny:
            break;
    }
    return rval;
}
