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
 *  ScriptablePluginObjectBase.h
 *  BrowserPlus
 *
 *  Created by Gordon Durand on 5/8/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __SCRIPTABLEPLUGINOBJECTBASE_H__
#define __SCRIPTABLEPLUGINOBJECTBASE_H__

#include "common.h"

// Helper class that can be used to map calls to the NPObject hooks
// into virtual methods on instances of classes that derive from this
// class.
class ScriptablePluginObjectBase : public NPObject
{
public:
	ScriptablePluginObjectBase(NPP npp) : m_npp(npp) {
	}

	virtual ~ScriptablePluginObjectBase() {
	}

	// Virtual NPObject hooks called through this base class. Override
	// as you see fit.
	virtual void invalidate();
	virtual bool hasMethod(NPIdentifier name);
	virtual bool invoke(NPIdentifier name, 
						const NPVariant* args,
						uint32_t argCount,
						NPVariant* result);
	virtual bool invokeDefault(const NPVariant* args, 
							   uint32_t argCount,
							   NPVariant* result);
	virtual bool hasProperty(NPIdentifier name);
	virtual bool getProperty(NPIdentifier name,
							 NPVariant* result);
	virtual bool setProperty(NPIdentifier name, 
							 const NPVariant* value);
	virtual bool removeProperty(NPIdentifier name);
	
	static void sDeallocate(NPObject* npobj);
	static void sInvalidate(NPObject* npobj);
	static bool sHasMethod(NPObject* npobj,
						   NPIdentifier name);
	static bool sInvoke(NPObject* npobj,
						NPIdentifier name,
						const NPVariant* args,
						uint32_t argCount,
						NPVariant* result);
	static bool sInvokeDefault(NPObject* npobj, 
							   const NPVariant* args,
							   uint32_t argCount, 
							   NPVariant* result);
	static bool sHasProperty(NPObject* npobj,
							 NPIdentifier name);
	static bool sGetProperty(NPObject* npobj,
							 NPIdentifier name,
							 NPVariant* result);
	static bool sSetProperty(NPObject* npobj, 
							 NPIdentifier name,
							 const NPVariant* value);
	static bool sRemoveProperty(NPObject* npobj, 
								NPIdentifier name);
	
protected:
	NPP m_npp;
};

#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class, ctor)                        \
static NPClass s##_class##_NPClass = {                                        \
	NP_CLASS_STRUCT_VERSION,                                                  \
	ctor,                                                                     \
	ScriptablePluginObjectBase::sDeallocate,                                  \
	ScriptablePluginObjectBase::sInvalidate,                                  \
	ScriptablePluginObjectBase::sHasMethod,                                   \
	ScriptablePluginObjectBase::sInvoke,                                      \
	ScriptablePluginObjectBase::sInvokeDefault,                               \
	ScriptablePluginObjectBase::sHasProperty,                                 \
	ScriptablePluginObjectBase::sGetProperty,                                 \
	ScriptablePluginObjectBase::sSetProperty,                                 \
	ScriptablePluginObjectBase::sRemoveProperty                               \
}

#define GET_NPOBJECT_CLASS(_class) &s##_class##_NPClass

#endif // __SCRIPTABLEPLUGINOBJECTBASE_H__

