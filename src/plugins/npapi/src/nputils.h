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

// various utilities for interacting with NPAPI/NPRUNTIME
// Adapted from code authord by AhitGandhi

#ifndef __NPUTILS_H__
#define __NPUTILS_H__

#include <string>
#include <vector>

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include "BPUtils/bptypeutil.h"
#include "PluginCommonLib/BPTransaction.h"
#include "PluginCommonLib/PluginObject.h"

namespace npu {

// get a string value form an npvariant
bool getStringValue(const NPVariant& var, std::string &outStr);

bool getWindow(NPP instance, NPVariant &outWindow);

bool getCurrentURL(NPP instance, std::string &outUrl);

void retainVariant(NPVariant& var);
void releaseVariant(NPVariant& var);
void releaseObject(NPObject* obj);
void retainObject(NPObject* obj);

bool getElementProperty(NPP instance, const NPVariant * parent,
                        const char* childId, NPVariant * outValue);

bool getArrayLength(NPP npp, const NPObject* array, unsigned int *outInt);

bool getArrayElement(NPP npp, const NPObject* parent,
                     unsigned int i, NPVariant *outValue);

bool callFunction(NPP instance, NPObject* obj, const NPVariant args[],
                  const int argCount, NPVariant * result);

// genrate success and generate error should be used to inflate return
// values from plugin to javascript.  They build objects conformant with
// our conventions, namely that objects are returned with a success key,
// dependent on the value of the success key, either error or value is
// present
bool generateSuccessReturn(NPP npp, const bp::Object &value,
                           NPVariant * rv);

bool generateErrorReturn(NPP npp, const char * error,
                         const char * verboseError, NPVariant * rv);

bool evaluateScript(NPP npp, const std::string &script,
                    NPVariant * result);

// Any time you're evaluating JSON, you should use this function.  On
// firefox OSX 2.0.7, evaluation of literal objects is sometimes broken.
// This function works around that problem.
bool evaluateJSON(NPP npp, const bp::Object &obj,
                  NPVariant * result);

// enumerate the properties of a javascript object
std::vector<std::string> enumerateProperties(NPP npp, NPObject * obj);

// determine if a NPVariant is an array
bool isArray(NPP npp, const NPVariant * var);

// determine if a NPVariant is a function
bool isFunction(NPP npp, const NPVariant * var);

// traverse an argument from javascript, perform callback caching and
// handle de-obfuscation, return a bp::Object representation.
// callbacks will be stored on the transaction object, and will be
// replaced with unique integer handles.
bool variantToObject(NPP npp, const NPVariant * input,
                     BPTransaction * transaction, bp::Object * &output);

// traverse a return value from BPCore.  perform handle obfuscation
bool objectToJSON(const bp::Object * input, bp::Object * &output);

// evaluate some javascript containing a function declaraion, return
// a NPObject which may be later  called using npu::callFunction
NPObject * createJSFunc(NPP npp, const char * funcBody);

};

#endif
