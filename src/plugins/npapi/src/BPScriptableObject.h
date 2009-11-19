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

/*
 *  BPScriptableObject.h
 *  BrowserPlusPlugin
 *
 *  Created by Gordon Durand on 5/2/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __BPSCRIPTABLEOBJECT_H__
#define __BPSCRIPTABLEOBJECT_H__

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include <string>
#include <list>
#include "common.h"
#include "ScriptablePluginObjectBase.h"
#include "BPProtocol/BPProtocol.h"
#include "NPAPIPlugin.h"

class ResultsObject
{
public:
    ResultsObject(NPObject * obj);
    ~ResultsObject();
    
    std::string m_results;
    NPObject * obj;
};

//
// BPScriptableObject implements the NPAPI structure (object layout is
// important) and implements functions that the browser calls.
// It's responsibility is to dispatch functions into the BPSession
// object.  There should be one BPScriptableObject allocated per active
// session.
//
class BPScriptableObject : public ScriptablePluginObjectBase
{
  public:
    BPScriptableObject(NPP instance);
    virtual ~BPScriptableObject();
    virtual bool hasMethod(NPIdentifier name);
    virtual bool invoke(NPIdentifier name, 
                        const NPVariant* args,
                        uint32_t argCount,
                        NPVariant* result);
    virtual bool hasProperty(NPIdentifier name);
    virtual bool getProperty(NPIdentifier name,
                             NPVariant* result);
    virtual bool setProperty(NPIdentifier name, 
                             const NPVariant* value);

    static NPObject * createObject(NPP npp);

  protected:
    std::string m_version;

    // The NPAPI Plugin instance associated with this scriptable object.
    NPAPIPlugin * m_plugin;
};

#endif // __BROWSERPLUS_H__
