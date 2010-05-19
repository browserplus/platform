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
 *  GenericJSObject.cpp
 *  A plugin allocated object that may be manipulated from both plugin
 *  code and javascript code
 *
 *  Created by Lloyd Hilaiel 5/17/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

// no "unused args", "conditional expression is a constant",
// or non-standard nameless union warnings
// (second warning from NPVariant macros, third from winnt.h)
#ifdef WIN32
#pragma warning (disable: 4100 4127 4201)
#endif

#include "GenericJSObject.h"
#include "BPUtils/BPLog.h"
#include "nputils.h"

static std::string
getStringFromIdentifier(NPIdentifier id)
{
    std::string str;
    NPUTF8 * utf8 = gBrowserFuncs.utf8fromidentifier(id);
    str.append(utf8);
    gBrowserFuncs.memfree(utf8);
    return str;
}

static NPObject*
allocateBPGenericObject(NPP npp, NPClass* theClass)
{
	return new BPGenericObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(BPGenericObject,
                                 allocateBPGenericObject);

BPGenericObject::BPGenericObject(NPP npp)
    : ScriptablePluginObjectBase(npp)
{
}

BPGenericObject::~BPGenericObject()
{
}

bool
BPGenericObject::hasProperty(NPIdentifier name)
{
    std::string key = getStringFromIdentifier(name);
    BPLOG_INFO_STRM("BPGenericObject::hasProperty: " << key);

    std::map<std::string, std::string>::iterator it;
    it = m_properties.find(key);

    return (it != m_properties.end());
}

bool
BPGenericObject::getProperty(NPIdentifier name,
							 NPVariant* result)
{
    std::string key = getStringFromIdentifier(name);
    BPLOG_INFO_STRM("BPGenericObject::getProperty: " << key);

    std::map<std::string, std::string>::iterator it;
    it = m_properties.find(key);
    if (it != m_properties.end())
    {
        std::string val = it->second;
        char * mem = (char *) gBrowserFuncs.memalloc(val.length()+1);
        memcpy((void *) mem, (const void *) val.c_str(), val.length()+1);
        STRINGZ_TO_NPVARIANT(mem, *result);
        return true;
    }

    return false;    
}

void
BPGenericObject::setProperty(const char * key, const char * value)
{
    BPLOG_DEBUG("BPGenericObject::setProperty");
    m_properties[std::string(key)] = std::string(value);
}

bool
BPGenericObject::defineFunction(const std::string & name,
                                IJSCallableFunctionHost * implementor)
{
    if (name.empty() || implementor == NULL) return false;
    (void) clearFunction(name);
    m_methods[name] = implementor;
    return true;
}

bool
BPGenericObject::clearFunction(const std::string & name)
{
    std::map<std::string, IJSCallableFunctionHost *>::iterator it;
    it = m_methods.find(name);
    if (it == m_methods.end()) return false;
    m_methods.erase(it);
    return true;
}

bool
BPGenericObject::hasMethod(NPIdentifier name)
{
    return (m_methods.end() != m_methods.find(getStringFromIdentifier(name)));
}

bool
BPGenericObject::invoke(NPIdentifier nameIdent,
						const NPVariant* argsVariant,
						uint32_t argCount,
						NPVariant* result)
{
    // function name
    std::string funcName = getStringFromIdentifier(nameIdent);

    std::map<std::string, IJSCallableFunctionHost *>::iterator it;
    it = m_methods.find(funcName);
    if (it == m_methods.end() || it->second == NULL) return false;
    
    // now prepare arguments.
    bp::Object * rv = it->second->invoke(funcName, argsVariant, argCount);

    // TODO: now translate return value
    // (in terms of file handes and callbacks??)

    if (rv) delete rv;

    return true;
}

NPObject*
BPGenericObject::getObject(NPP npp)
{
    NPObject * obj = gBrowserFuncs.createobject(
        npp, GET_NPOBJECT_CLASS(BPGenericObject));
    return obj;
}
