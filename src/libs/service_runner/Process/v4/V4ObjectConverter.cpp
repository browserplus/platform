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

bp::Object *
sapi_v4::v4ToBPObject(const struct sapi_v4::BPElement_t * elemPtr)
{
    return NULL;
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
