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
 * Convert data between v4 and current representations for API backwards
 * compatibility
 */

#include "v4_bptypes.h"

#include "V4ObjectConverter.h"
#include "BPUtils/bptypeutil.h"

#include <string.h>

#ifdef WIN32
#define strdup _strdup
#endif

bp::Object *
sapi_v4::v4ToBPObject(const sapi_v4::BPElement * elemPtr)
{
    if (!elemPtr) return NULL;
    bp::Object * o = NULL;
    
    switch (elemPtr->type) {
        case BPTNull:
            o = new bp::Null;
            break;
        case BPTBoolean:
            o = new bp::Bool(elemPtr->value.booleanVal);
            break;
        case BPTInteger:
            o = new bp::Integer(elemPtr->value.integerVal);
            break;
        case BPTDouble:
            o = new bp::Double(elemPtr->value.doubleVal);
            break;
        case BPTString:
            o = new bp::String(elemPtr->value.stringVal);
            break;
        case BPTMap: {
            bp::Map * m = new bp::Map;
            for (unsigned int i = 0; i < elemPtr->value.mapVal.size; i++) {
                m->add(elemPtr->value.mapVal.elements[i].key,
					v4ToBPObject(elemPtr->value.mapVal.elements[i].value));
            }
            o = m;
            break;
        }
        case BPTList: {
            bp::List * l = new bp::List;
            for (unsigned int i = 0; i < elemPtr->value.listVal.size; i++)
            {
				l->append(v4ToBPObject(elemPtr->value.listVal.elements[i]));
            }
            o = l;
            break;
        }
        case BPTCallBack:
            o = new bp::CallBack(elemPtr->value.callbackVal);
            break;
        case BPTPath:
            // AHA: a v4 Path is a utf8 encoded url, here we convert. 
            o = new bp::Path(bp::file::pathFromURL(elemPtr->value.pathVal));
            break;
        case BPTAny:
            // noop!
            break;
    }

    return o;
}

sapi_v4::BPElement *
sapi_v4::v5ElementToV4(const ::BPElement * elemPtr)
{
    sapi_v4::BPElement * rv = NULL;

    switch (elemPtr->type) {
        case BPTNull:
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTNull;
            break;
        case BPTBoolean:
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTBoolean;
            rv->value.booleanVal = elemPtr->value.booleanVal;
            break;
        case BPTInteger:
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTInteger;
            rv->value.integerVal = elemPtr->value.integerVal;
            break;
        case BPTDouble:
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTDouble;
            rv->value.doubleVal = elemPtr->value.doubleVal;
            break;
        case BPTString:
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTString;
            if (elemPtr->value.stringVal) {
                rv->value.stringVal = strdup(elemPtr->value.stringVal);
            }
            break;
        case BPTMap: {
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTMap;
            rv->value.mapVal.size = elemPtr->value.mapVal.size;
            if (elemPtr->value.mapVal.size) {
                rv->value.mapVal.elements = (struct sapi_v4::BPMapElem_t *) calloc(elemPtr->value.mapVal.size, sizeof(struct sapi_v4::BPMapElem_t));

                for (unsigned int i = 0; i < elemPtr->value.mapVal.size; i++) {
                    rv->value.mapVal.elements[i].key = strdup(elemPtr->value.mapVal.elements[i].key);
                    rv->value.mapVal.elements[i].value =
                        sapi_v4::v5ElementToV4(elemPtr->value.mapVal.elements[i].value);
                }
            }
            break;
        }
        case BPTList: {
            rv = (sapi_v4::BPElement *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTList;
            rv->value.listVal.size = elemPtr->value.listVal.size;
            if (elemPtr->value.listVal.size) {
                rv->value.listVal.elements = (struct sapi_v4::BPElement_t **) calloc(elemPtr->value.listVal.size, sizeof(struct sapi_v4::BPElement_t *));

                for (unsigned int i = 0; i < elemPtr->value.mapVal.size; i++) {
                    rv->value.listVal.elements[i] = sapi_v4::v5ElementToV4(elemPtr->value.listVal.elements[i]);
                }
            }
            break;
        }
        case BPTCallBack:
            rv = (struct sapi_v4::BPElement_t *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTCallBack;
            rv->value.callbackVal = elemPtr->value.callbackVal;
            break;
        case BPTPath:
            // a v5 path is native, a v4 Path is a utf8 encoded url, here we convert. 
            rv = (struct sapi_v4::BPElement_t *) calloc(1, sizeof(struct sapi_v4::BPElement_t));
            rv->type = sapi_v4::BPTPath;
            if (elemPtr->value.pathVal) {
                rv->value.pathVal = strdup(bp::file::Path(elemPtr->value.pathVal).url().c_str());
            }
            break;
        case BPTAny:
            // noop!
            break;
    }
    
    return rv;
}

void
sapi_v4::freeDynamicV4Element(struct sapi_v4::BPElement_t * e)
{
    if (!e) return;

    switch (e->type) {
        case BPTNull:
        case BPTBoolean:
        case BPTInteger:
        case BPTDouble:
        case BPTCallBack:
            // noop
            break;
        case BPTString:
            if (e->value.stringVal) free(e->value.stringVal);
            break;
        case BPTPath:
            if (e->value.pathVal) free(e->value.pathVal);
            break;
        case BPTList: {
            if (e->value.listVal.size) {
                for (unsigned int i = 0; i < e->value.mapVal.size; i++) {
                    freeDynamicV4Element(e->value.listVal.elements[i]);
                }
                free(e->value.listVal.elements);
            }
            break;
        }
        case BPTMap: {
            if (e->value.mapVal.size) {
                for (unsigned int i = 0; i < e->value.mapVal.size; i++) {
                    free(e->value.mapVal.elements[i].key);
                    freeDynamicV4Element(e->value.mapVal.elements[i].value);
                }
                free(e->value.mapVal.elements);
            }
            break;
        }
        case BPTAny:
            // noop!
            break;
    }
    
    free(e);
}
