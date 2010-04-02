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
#include "bperrorutil.h"
#include <yajl/yajl_gen.h>
#include <string.h>

using namespace bp;

/** 
* begin JSON serialization
*/

#define CHECK_STATUS(_stat_) if ((_stat_) != yajl_gen_status_ok) break;

static yajl_gen_status 
toJsonRecurse(const Object* obj, 
              yajl_gen ghand)
{
    yajl_gen_status stat = yajl_gen_status_ok;
    
    BPASSERT(obj != NULL);
    
    switch (obj->type()) {
        case BPTMap: {
            stat = yajl_gen_map_open(ghand);
            CHECK_STATUS(stat);
            Map* m = (Map *) obj;
            Map::Iterator it(*m);
            const char* key = NULL;
            while ((key = it.nextKey()) != NULL) {
                stat = yajl_gen_string(ghand, (const unsigned char *) key,
                                       strlen(key));
                CHECK_STATUS(stat);
                stat = toJsonRecurse(m->value(key), ghand);
                CHECK_STATUS(stat);
            }
            CHECK_STATUS(stat);
            stat = yajl_gen_map_close(ghand);
            break;
        }
        case BPTList: {
            stat = yajl_gen_array_open(ghand);
            CHECK_STATUS(stat);
            List* l = (List*) obj;
            for (unsigned int i = 0; i < l->size(); i++) {
                stat = toJsonRecurse(l->value(i), ghand);
                CHECK_STATUS(stat);
            }

            stat = yajl_gen_array_close(ghand);
            CHECK_STATUS(stat);
            break;
        }
        case BPTNull: {
            stat = yajl_gen_null(ghand);
            break;
        }
        case BPTBoolean: {
            Bool* b = (Bool*) obj;
            stat = yajl_gen_bool(ghand, b->value() ? 1 : 0);
            break;
        }
        case BPTString: {
            String* s = (String*) obj;
            stat = yajl_gen_string(ghand,
                                   (const unsigned char*) s->value(),
                                   (strlen(s->value())));
            break;
        }
        case BPTDouble: {
            Double* d = (Double*) obj;
            stat = yajl_gen_double(ghand, d->value());
            break;
        }
        case BPTInteger: {
            Integer* i = (Integer*) obj;
            stat = yajl_gen_integer(ghand, static_cast<long>(i->value()));
            break;
        }
        case BPTCallBack: {
            CallBack* i = (CallBack*) obj;
            stat = yajl_gen_integer(ghand, static_cast<long>(i->value()));
            break;
        }
        case BPTPath: {
            Path* s = (Path*) obj;
            stat = yajl_gen_string(ghand,
                                   (const unsigned char*) s->value(),
                                   (strlen(s->value())));
            break;
        }
        case BPTAny: {
            // invalid
            break;
        }
    }

    return stat;
}


std::string 
Object::toPlainJsonString(bool pretty) const
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
