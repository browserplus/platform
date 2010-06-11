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

#include "bptypeutil.h"
#include "bperrorutil.h"
#include "bpfile.h"
#include <yajl/yajl_parse.h>
#include <iostream>
#include <stack>

struct ParseContext {
    std::stack<bp::Object *> nodeStack;
    std::stack<std::string> keyStack;
    unsigned int depth;
};

// push an element to the correct place
#define GOT_ELEMENT(pc, elem) {                                         \
  if ((pc)->nodeStack.size() == 0) {                                    \
      (pc)->nodeStack.push(elem);                                       \
  } else if ((pc)->nodeStack.top()->type() == BPTList) {                \
      dynamic_cast<bp::List *>((pc)->nodeStack.top())->append(elem);    \
  } else if ((pc)->nodeStack.top()->type() == BPTMap) {                 \
      dynamic_cast<bp::Map *>((pc)->nodeStack.top())->add(              \
          (pc)->keyStack.top().c_str(), (elem));                        \
      (pc)->keyStack.pop();                                             \
  }                                                                     \
}

static int
null_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new bp::Null);
    }
    return 1;
}

static int
boolean_cb(void * ctx, int boolVal)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new bp::Bool(boolVal));
    }
    return 1;
}

static int
integer_cb(void * ctx, long integerVal)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new bp::Integer(integerVal));
    }
    return 1;
}

static int
double_cb(void * ctx, double doubleVal)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new bp::Double(doubleVal));
    }
    return 1;
}

static int
string_cb(void * ctx, const unsigned char * stringVal,
                            unsigned int stringLen)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new bp::String((const char *) stringVal, stringLen));
    }
    return 1;
}

static int
start_map_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        pc->depth++;
        // map starts.  push a map onto the nodestack
        pc->nodeStack.push(new bp::Map);
    }
    return 1;
}

static int
map_key_cb(void * ctx, const unsigned char * key,
           unsigned int keyLen)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        std::string keyStr;
        keyStr.append((const char *) key, keyLen);
        pc->keyStack.push(keyStr);
    }
    return 1;
}

static int
end_map_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;    
    if (pc) {
        bp::Object * obj = pc->nodeStack.top();
        pc->nodeStack.pop();
        BPASSERT(obj->type() == BPTMap);
    
        // See if map describes one of our BP types
        // If so, replace map with instance of BP object.
        bp::Map * map = dynamic_cast<bp::Map*>(obj);
        if (pc->depth % 2) {
            obj = createBPObject(map);
            delete map;
        }
    
        GOT_ELEMENT(pc, obj);    

        pc->depth--;
    }
    return 1;
}

static int
start_array_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        // array starts.  push an empty array onto the nodestack
        pc->nodeStack.push(new bp::List);
        pc->depth++;
    }
    return 1;
}

static int
end_array_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;    
    if (pc) {
        bp::Object * obj = pc->nodeStack.top();
        pc->nodeStack.pop();
        BPASSERT(obj->type() == BPTList);
        GOT_ELEMENT(pc, obj);    
        pc->depth--;
    }
    return 1;
}

const static yajl_callbacks callbacks = {
    null_cb,
    boolean_cb,
    integer_cb,
    double_cb,
    NULL,
    string_cb,
    start_map_cb,
    map_key_cb,
    end_map_cb,
    start_array_cb,
    end_array_cb
};

bp::Object *
bp::Object::fromJsonString(std::string jsonText)
{
    yajl_parser_config cfg = { 1 };
    yajl_handle yh;
    yajl_status s;
    ParseContext pc;
    pc.depth = 0;
    yh = yajl_alloc(&callbacks, &cfg, NULL, (void *) &pc);
    s = yajl_parse(yh, (const unsigned char *) jsonText.c_str(),
                   jsonText.length());
    yajl_free(yh);

    if (s != yajl_status_ok) {
        while(pc.nodeStack.size()) {
            delete pc.nodeStack.top();
            pc.nodeStack.pop();
        }
        return NULL;
    }
    
    BPASSERT(pc.nodeStack.size() == 1);
    return pc.nodeStack.top();
}


bp::Object * 
bp::createBPObject(const Map * map)
{
    Object * rval = NULL;
    const Object * objType = map->value(BROWSERPLUS_OBJECT_TYPE_KEY);
    if (objType == NULL || objType->type() != BPTString) return NULL;
    const String * sObj = dynamic_cast<const String*>(objType);
    if (sObj == NULL) return NULL;
    std::string bpType = sObj->value();

    const Object * valObj = map->value(BROWSERPLUS_OBJECT_VALUE_KEY);
    if (valObj == NULL) return NULL;

    if (bpType.compare("path") == 0) {
        // Make a Path from the String value
        sObj = dynamic_cast<const String*>(valObj);
        if (sObj == NULL) return NULL;
        rval = new bp::Path(bp::file::Path(sObj->value()));
    } else if (bpType.compare("writablePath") == 0) {
        // Make a Path from the String value
        sObj = dynamic_cast<const String*>(valObj);
        if (sObj == NULL) return NULL;
        rval = new bp::WritablePath(bp::file::Path(sObj->value()));
    } else if (bpType.compare("callback") == 0) {
        // Make a Callback from the Integer value
        const Integer * iObj = dynamic_cast<const Integer*>(valObj);
        if (iObj == NULL) return NULL;
        rval = new bp::CallBack(iObj->value());
    } else {
        // These are all simple, value is of correct type
        rval = valObj->clone();
    }
    return rval;
}

