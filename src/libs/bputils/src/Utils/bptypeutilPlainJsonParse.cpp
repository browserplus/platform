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
#include <iostream>
#include <stack>
#include <yajl/yajl_parse.h>
#include "bperrorutil.h"

using namespace bp;

struct ParseContext {
    std::stack<Object *> nodeStack;
    std::stack<std::string> keyStack;
};

// push an element to the correct place
#define GOT_ELEMENT(pc, elem) {                                         \
  if ((pc)->nodeStack.size() == 0) {                                    \
        (pc)->nodeStack.push(elem);                                     \
  } else if ((pc)->nodeStack.top()->type() == BPTList) {                \
        List * l = dynamic_cast<List *>((pc)->nodeStack.top());         \
        BPASSERT(l);                                                    \
        l->append(elem);                                                \
  } else if ((pc)->nodeStack.top()->type() == BPTMap) {                 \
        Map * m = dynamic_cast<Map *>((pc)->nodeStack.top());           \
        BPASSERT(m);                                                    \
        m->add((pc)->keyStack.top().c_str(), (elem));                   \
       (pc)->keyStack.pop();                                             \
  }                                                                     \
}

static int
null_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new Null);
    }
    return 1;
}

static int
boolean_cb(void * ctx, int boolVal)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new Bool(boolVal));
    }
    return 1;
}

static int
integer_cb(void * ctx, long integerVal)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new Integer(integerVal));
    }
    return 1;
}

static int
double_cb(void * ctx, double doubleVal)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new Double(doubleVal));
    }
    return 1;
}

static int
string_cb(void * ctx, const unsigned char * stringVal,
                            unsigned int stringLen)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        GOT_ELEMENT(pc, new String((const char *) stringVal, stringLen));
    }
    return 1;
}

static int
start_map_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        // map starts.  push a map onto the nodestack
        pc->nodeStack.push(new Map);
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
        Object * obj = pc->nodeStack.top();
        pc->nodeStack.pop();
        BPASSERT(obj->type() == BPTMap);
        GOT_ELEMENT(pc, obj);    
    }
    return 1;
}

static int
start_array_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;
    if (pc) {
        // array starts.  push an empty array onto the nodestack
        pc->nodeStack.push(new List);
    }
    return 1;
}

static int
end_array_cb(void * ctx)
{
    ParseContext * pc = (ParseContext *) ctx;    
    if (pc) {
        Object * obj = pc->nodeStack.top();
        pc->nodeStack.pop();
        BPASSERT(obj->type() == BPTList);
        GOT_ELEMENT(pc, obj);    
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

Object *
Object::fromPlainJsonString(const std::string & jsonTextIn, std::string * error)
{
    yajl_parser_config cfg = { 1 };
    yajl_handle yh;
    yajl_status s;
    ParseContext pc;
    yh = yajl_alloc(&callbacks, &cfg, NULL, (void *) &pc);
    // Add a bit of whitespace to the end which
    // allows correct parsing of integers (YIB-1956152)
    std::string jsonText = jsonTextIn + " ";
    s = yajl_parse(yh, (const unsigned char *) jsonText.c_str(),
                   jsonText.length());

    if (s != yajl_status_ok) {
        while(pc.nodeStack.size()) {
            delete pc.nodeStack.top();
            pc.nodeStack.pop();
        }
        if (error != NULL) {
            unsigned char * errString =
                yajl_get_error(yh, 1,
                               (const unsigned char *) jsonText.c_str(),
                               jsonText.length());

            error->append((const char *) errString);
            yajl_free_error(yh, errString);
        }
        yajl_free(yh);
        return NULL;
    }

    yajl_free(yh);
    
    BPASSERT(pc.nodeStack.size() == 1);

    return pc.nodeStack.top();
}
