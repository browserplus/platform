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


#ifndef __V4OBJECTCONVERTER_H__
#define __V4OBJECTCONVERTER_H__

#include "v4_bptypes.h"
#include "BPUtils/bptypeutil.h"

namespace sapi_v4
{
    /* convert from a v4 data structure to a bp::Object */
    bp::Object * v4ToBPObject(const sapi_v4::BPElement * elemPtr);

    /* convert from a modern BPElement to a v4 BPElement, returning dynamically
     * allocated memory that must be freed */
    sapi_v4::BPElement * v5ElementToV4(const ::BPElement * elemPtr);

	void freeDynamicV4Element(sapi_v4::BPElement * elemPtr);
};

#endif
