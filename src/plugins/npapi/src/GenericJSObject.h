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
 *  GenericJSObject.h
 *  A plugin allocated object that may be manipulated from both plugin
 *  code and javascript code
 *
 *  Created by Lloyd Hilaiel 5/17/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __GENERICJSOBJECT_H__
#define __GENERICJSOBJECT_H__

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include <string>
#include <map>
#include "common.h"
#include "ScriptablePluginObjectBase.h"

#include "BPUtils/bptypeutil.h"

// A listener may implement callable functions on the javascript object
class IJSCallableFunctionHost 
{
  public:
    // return value is *dynamically* allocated by callee, caller free'd
    // Note, we *might* want to convert these arguments at some point automatically
    // inside the abstraction.  Some considerations, however...  JavaScript callbacks
    // and BrowserPlus custom types (files) require additional context, some place
    // to perform the mapping (file handle->path or callback id->NSObject ref), and this stuff 
    // is browser specific.  For now, we'll just pass around raw npapi types
    virtual bp::Object * invoke(const std::string & funcName,
                                const NPVariant* args,
                                uint32_t argCount) = 0;
    virtual ~IJSCallableFunctionHost() { }
};

// A generic object that may be manipulated from both javascript and
// c++.  A useful mechanism to return objects back to javascript.
//
// currently, only properties are supported
class BPGenericObject : public ScriptablePluginObjectBase
{
public:
	virtual ~BPGenericObject();

    // C++ interface
    void setProperty(const char * key, const char * value);
    bool defineFunction(const std::string & name,
                        IJSCallableFunctionHost * implementor);
    bool clearFunction(const std::string & name);

    static NPObject* getObject(NPP npp);

    // do not use
	BPGenericObject(NPP npp);
protected:

    // implemented functions from base class to support JavaScript
    // scripting
	virtual bool hasProperty(NPIdentifier name);
	virtual bool getProperty(NPIdentifier name,
							 NPVariant* result);
	virtual bool hasMethod(NPIdentifier name);
	virtual bool invoke(NPIdentifier name, 
						const NPVariant* args,
						uint32_t argCount,
						NPVariant* result);

    std::map<std::string, std::string> m_properties;
    std::map<std::string, IJSCallableFunctionHost *> m_methods;
};

#endif
