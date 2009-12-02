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

#include "bptypeutil.h"
#include <yajl/yajl_gen.h>
#include <assert.h>
#include <string.h>

using namespace bp;

/** 
* begin JSON serialization
*/

static yajl_gen_status 
toJsonRecurse(const Object* obj, 
              yajl_gen ghand)
{
    yajl_gen_status stat = yajl_gen_status_ok;
    
    assert(obj != NULL);
    
    const unsigned char* typeKey = (const unsigned char*) BROWSERPLUS_OBJECT_TYPE_KEY;
    int typeKeyLen = strlen((const char*) typeKey);
    const unsigned char* valueKey = (const unsigned char*) BROWSERPLUS_OBJECT_VALUE_KEY;
    int valueKeyLen = strlen((const char*) valueKey);
    
    stat = yajl_gen_map_open(ghand);
    yajl_gen_string(ghand, typeKey, typeKeyLen);
    
    switch (obj->type()) {
        case BPTMap: {
            yajl_gen_string(ghand, (const unsigned char*) "map",
                            strlen("map"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            yajl_gen_map_open(ghand);
            Map* m = (Map *) obj;
            Map::Iterator it(*m);
            const char* key = NULL;
            while ((key = it.nextKey()) != NULL) {
                yajl_gen_string(ghand, (const unsigned char *) key,
                                strlen(key));
                toJsonRecurse(m->value(key), ghand);
            }
            yajl_gen_map_close(ghand);
            break;
        }
        case BPTList: {
            yajl_gen_string(ghand, (const unsigned char*) "list",
                            strlen("list"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            stat = yajl_gen_array_open(ghand);
            List* l = (List*) obj;
            for (unsigned int i = 0; i < l->size(); i++) {
                toJsonRecurse(l->value(i), ghand);
            }
            yajl_gen_array_close(ghand);
            break;
        }
        case BPTNull: {
            yajl_gen_string(ghand, (const unsigned char*) "null", 
                            strlen("null"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            yajl_gen_null(ghand);
            break;
        }
        case BPTBoolean: {
            yajl_gen_string(ghand, (const unsigned char*) "boolean",
                            strlen("boolean"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            Bool* b = (Bool*) obj;
            yajl_gen_bool(ghand, b->value() ? 1 : 0);
            break;
        }
        case BPTString: {
            yajl_gen_string(ghand, (const unsigned char*) "string", 
                            strlen("string"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            String* s = (String*) obj;
            yajl_gen_string(ghand,
                            (const unsigned char*) s->value(),
                            (strlen(s->value())));
            break;
        }
        case BPTDouble: {
            yajl_gen_string(ghand, (const unsigned char*) "double", 
                            strlen("double"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            Double* d = (Double*) obj;
            yajl_gen_double(ghand, d->value());
            break;
        }
        case BPTInteger: {
            yajl_gen_string(ghand, (const unsigned char*) "integer",
                            strlen("integer"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            Integer* i = (Integer*) obj;
            yajl_gen_integer(ghand, static_cast<long>(i->value()));
            break;
        }
        case BPTCallBack: {
            yajl_gen_string(ghand, (const unsigned char*) "callback",
                            strlen("callback"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            CallBack* i = (CallBack*) obj;
            yajl_gen_integer(ghand, static_cast<long>(i->value()));
            break;
        }
        case BPTPath: {
            yajl_gen_string(ghand, (const unsigned char*) "path", 
                            strlen("path"));
            yajl_gen_string(ghand, valueKey, valueKeyLen);
            Path* s = (Path*) obj;
            yajl_gen_string(ghand,
                            (const unsigned char*) s->value(),
                            (strlen(s->value())));
            break;
        }
        case BPTAny: {
            // invalid
            break;
        }
    }
    stat = yajl_gen_map_close(ghand);
    return stat;
}


std::string 
Object::toJsonString(bool pretty) const
{
    yajl_gen_status s;
    yajl_gen_config cfg = { pretty ? 1 : 0, NULL };
    yajl_gen ghand = yajl_gen_alloc(&cfg, NULL);
    
    s = toJsonRecurse(this, ghand);
    
    std::string results;
    
    if (s == yajl_gen_status_ok) {
        const unsigned char* buf = NULL;
        unsigned int len = 0;
        
        (void) yajl_gen_get_buf(ghand, &buf, &len);
        results.append((const char*) buf, len);
    }
    
    yajl_gen_free(ghand);
    
    return results;
}

/** 
* end JSON serialization
*/
