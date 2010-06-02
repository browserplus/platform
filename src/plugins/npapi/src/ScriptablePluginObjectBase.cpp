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
 *  ScriptablePluginObjectBase.cpp
 *  BrowserPlus
 *
 *  Created by Gordon Durand on 5/8/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "ScriptablePluginObjectBase.h"

#ifdef WIN32
// unused args warning
#pragma warning (disable : 4100)
#endif

void
ScriptablePluginObjectBase::invalidate()
{
}


bool
ScriptablePluginObjectBase::hasMethod(NPIdentifier name)
{
	return false;
}


bool
ScriptablePluginObjectBase::invoke(NPIdentifier name,
								   const NPVariant* args,
                                   uint32_t argCount,
								   NPVariant *result)
{
	return false;
}


bool
ScriptablePluginObjectBase::invokeDefault(const NPVariant* args,
                                          uint32_t argCount,
										  NPVariant* result)
{
	return false;
}


bool
ScriptablePluginObjectBase::hasProperty(NPIdentifier name)
{
	return false;
}


bool
ScriptablePluginObjectBase::getProperty(NPIdentifier name, 
										NPVariant* result)
{
	return false;
}


bool
ScriptablePluginObjectBase::setProperty(NPIdentifier name,
                                        const NPVariant *value)
{
	return false;
}


bool
ScriptablePluginObjectBase::removeProperty(NPIdentifier name)
{
	return false;
}


// static
void
ScriptablePluginObjectBase::sDeallocate(NPObject* npobj)
{
	// Call the virtual destructor.
	delete (ScriptablePluginObjectBase*)npobj;
}


// static
void
ScriptablePluginObjectBase::sInvalidate(NPObject* npobj)
{
	((ScriptablePluginObjectBase*)npobj)->invalidate();
}


// static
bool
ScriptablePluginObjectBase::sHasMethod(NPObject* npobj, 
									   NPIdentifier name)
{
	return ((ScriptablePluginObjectBase*)npobj)->hasMethod(name);
}


// static
bool
ScriptablePluginObjectBase::sInvoke(NPObject* npobj, 
									NPIdentifier name,
                                    const NPVariant* args,
									uint32_t argCount,
                                    NPVariant* result)
{
	return ((ScriptablePluginObjectBase*)npobj)->invoke(name, args, argCount,
														result);
}


// static
bool
ScriptablePluginObjectBase::sInvokeDefault(NPObject* npobj,
                                           const NPVariant* args,
                                           uint32_t argCount,
                                           NPVariant* result)
{
	return ((ScriptablePluginObjectBase*)npobj)->invokeDefault(args, argCount,
															   result);
}


// static
bool
ScriptablePluginObjectBase::sHasProperty(NPObject* npobj,
										 NPIdentifier name)
{
	return ((ScriptablePluginObjectBase*)npobj)->hasProperty(name);
}


// static
bool
ScriptablePluginObjectBase::sGetProperty(NPObject* npobj,
										 NPIdentifier name,
                                         NPVariant* result)
{
	return ((ScriptablePluginObjectBase*)npobj)->getProperty(name, result);
}


// static
bool
ScriptablePluginObjectBase::sSetProperty(NPObject* npobj, 
										 NPIdentifier name,
                                         const NPVariant* value)
{
	return ((ScriptablePluginObjectBase*)npobj)->setProperty(name, value);
}


// static
bool
ScriptablePluginObjectBase::sRemoveProperty(NPObject* npobj,
											NPIdentifier name)
{
	return ((ScriptablePluginObjectBase*)npobj)->removeProperty(name);
}

