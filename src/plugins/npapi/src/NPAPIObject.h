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

/*
 *  NPAPIObject.h
 *
 *  Wraps an NPObject * in dave's cross browser abstraction
 *  
 *  Created by Lloyd Hilaiel on or around Thu Nov 15 20:35:21 PST 2007
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __NPAPIOBJECT_H__
#define __NPAPIOBJECT_H__

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include "nputils.h"

class NPAPIObject : public plugin::Object
{
  public:
    NPAPIObject(NPObject * obj) : m_obj(obj) { npu::retainObject(m_obj); }
    ~NPAPIObject() { npu::releaseObject(m_obj); }
    plugin::Object * clone() const { return new NPAPIObject(m_obj); }
    NPObject * object() const { return m_obj; }
  private:
    NPObject * m_obj;
};

#endif
