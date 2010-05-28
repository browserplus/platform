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


#include "V4ObjectConverter.h"
#include "BPUtils/bptypeutil.h"

bp::Object *
sapi_v4::v4ToBPObject(const struct sapi_v4::BPElement_t * elemPtr)
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
    return NULL;
}

void
sapi_v4::freeDynamicV4Element(struct sapi_v4::BPElement_t * elemPtr)
{
}
