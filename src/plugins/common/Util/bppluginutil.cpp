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

/************************************************************************
* bppluginutil.cpp
*
* Written by Gordon Durand, Thu 20 Sept 2007
* (c) 2007, Yahoo! Inc, all rights reserved.
*/

#include "api/bppluginutil.h"
#include <assert.h>
#include "api/BPHandleMapper.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpconfig.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"


using namespace std;
using namespace std::tr1;


bool
bp::pluginutil::appendEnumerateResultsToList(const BPElement * corelets,
                                             bp::List &serviceList)
{
    bool rv = true;

    // lets start by building on a bp object that we can interact with
    // a bit more cleanly
    bp::Object * obj = bp::Object::build(corelets);
    assert(obj != NULL && obj->type() == BPTList);
    bp::List * l = (bp::List *) obj;        

    for (unsigned int i=0; i < l->size(); i++) {
        if (l->value(i)->type() == BPTMap) {
            bp::Map * m = (bp::Map *) l->value(i);
            
            bp::Map * outMap = new bp::Map;

            bp::Map::Iterator it(*m);

            const char * key;
            // some validation on the proto response.  verify that
            // each
            bool hasName = false;
            bool hasVersion = false;
            bool hasType = false;
            
            for (key = it.nextKey(); key != NULL; key = it.nextKey()) {
                if (!strcmp(key, "name")) hasName = true;
                else if (!strcmp(key, "version")) hasVersion = true;
                else if (!strcmp(key, "type")) hasType = true;
                else if (!strcmp(key, "doc")) { /* noop */ }
                else {
                    // presence of keys other than 'name' and 'version'
                    // indicate a protocol error
                    rv = false;
                    break;
                }
                if (m->value(key)->type() != BPTString) {
                    // non-string value for name or verison indicates
                    // protocol error
                    rv = false;
                    break;
                }
                bp::String * s = (bp::String *) m->value(key);
                outMap->add(key, new bp::String(s->value()));
            }
            if (!hasName || !hasVersion || !hasType) {
                // both name and version must be present
                rv = false;
                break;
            }

            serviceList.append(outMap);
        } else {
            // The protocol response must contain a list of maps.
            // non-map indicates protocol error
            rv = false;
            break;
        }
    }
        
    delete obj;

    return rv;
}

bool
bp::pluginutil::toBrowserSafeRep( const bp::Object* input,
                                  bp::Object*& output )
{
    if (input)
        output = BPHandleMapper::insertHandles(input);
    else
        output = new bp::Null;
    
    return true;
}


bp::Object* 
bp::pluginutil::applyFilters(const vector<bp::file::Path>& selection,
                             const set<string>& mimetypes,
                             unsigned int flags,
                             unsigned int limit)
{
    class MyVisitor : virtual public bp::file::IVisitor {
    public:
        MyVisitor(const set<string>& mimetypes,
                  int parentID,
                  size_t limit,
                  bp::List* list)
        : m_mimetypes(mimetypes), m_parentID( parentID),
          m_limit(limit), m_num(0), m_list(list) {
        }
        virtual ~MyVisitor() {
        }
        bp::file::IVisitor::tResult visitNode(const bp::file::Path& p,
                                              const bp::file::Path&) {
            if (m_num >= m_limit) {
                return eStop;
            }
            if (bp::file::isMimeType(p, m_mimetypes)) {
                if (m_parentID) {
                    bp::Map* itemMap = new bp::Map;
                    itemMap->add("handle", new bp::Path(p));
                    itemMap->add("parent", new bp::Integer(m_parentID));
                    m_list->append(itemMap);
                } else {
                    m_list->append(new bp::Path(p));
                }
                m_num++;
            }
            return eOk;
        }
        set<string> m_mimetypes;
        int m_parentID;
        size_t m_limit;
        size_t m_num;
        bp::List* m_list;
    };

    unsigned int num = 0;
    bp::List* l = NULL;
    bp::Map* m = NULL;
    bp::List* selList = NULL;
    bp::List* fileList = NULL;
    if (flags & kIncludeGestureInfo) {
        m = new bp::Map;
        selList = new bp::List;
        m->add("actualSelection", selList);
        fileList = new bp::List;
        m->add("files", fileList);
    } else {
        l = new bp::List;
    }
    
    for (unsigned int i = 0; (num < limit) && (i < selection.size()); ++i) {
        bp::file::Path item = selection[i];
        int parentID = 0;
        if (flags & kIncludeGestureInfo) {
            bp::Path* itemPath = new bp::Path(item);
            bp::file::Path path = bp::file::pathFromURL(itemPath->value());
            BPHandle h = BPHandleMapper::pathToHandle(path);
            parentID = h.id();
            selList->append(itemPath);
        }
        
        // visit selected item (and maybe it's kids)
        MyVisitor v(mimetypes, parentID, limit - num,
                    parentID ? fileList : l);
        if (flags & kRecurse) {
            (void) recursiveVisit(item, v, true);
        } else {
            v.visitNode(item, item);
        }
        num += v.m_num;
    }
    
    return (flags & kIncludeGestureInfo) ? dynamic_cast<bp::Object*>(m)
        : dynamic_cast<bp::Object*>(l);
}

std::string
bp::pluginutil::getBuildType()
{
    std::string buildType;
    bp::config::ConfigReader configReader;

    if (!configReader.load(bp::paths::getConfigFilePath()) ||
        !configReader.getStringValue("BuildType", buildType)) 
    {
        buildType = "unknown";
    }
    
    return buildType;
}

