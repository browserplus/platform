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

// Disable "nameless struct/union" warnings that come from winnt.h that is 
// evidently included through some npapi header.

#include <iostream>
#include <sstream>

#include "common.h"
#include "NPAPIObject.h"
#include "nputils.h"
#include "PluginCommonLib/BPHandleMapper.h"


void
npu::releaseVariant(NPVariant& var)
{
    if (NPVARIANT_IS_OBJECT(var))
    {
        gBrowserFuncs.releaseobject(var.value.objectValue);
    }
    else
    {
        gBrowserFuncs.releasevariantvalue(&var);
    }
}

void
npu::retainVariant(NPVariant& var)
{
    if (NPVARIANT_IS_OBJECT(var))
    {
        gBrowserFuncs.retainobject(var.value.objectValue);
    }
}

bool
npu::getStringValue(const NPVariant& var, std::string &outStr)
{
    if(NPVARIANT_IS_STRING(var))
    {
        outStr.append(var.value.stringValue.utf8characters,
                      var.value.stringValue.utf8length);
        return true;
    }

    return false;
}

bool
npu::getWindow(NPP instance, NPVariant &outWindow)
{
	NPObject* windowObj = NULL;
	gBrowserFuncs.getvalue(instance, NPNVWindowNPObject, &windowObj);

    outWindow.type = NPVariantType_Object;
	outWindow.value.objectValue = windowObj;

	return true;
}

bool
npu::getArrayLength(NPP npp, const NPObject* array,
                    unsigned int *outInt)
{
    *outInt = 0;
    
    NPIdentifier propId = gBrowserFuncs.getstringidentifier("length");
	NPVariant arrlen;
    arrlen.type = NPVariantType_Void;
    
    if (!gBrowserFuncs.getproperty(npp, (NPObject *) array, propId, &arrlen)) {
        return false;        
    }

    bool rv = true;

    if (NPVARIANT_IS_INT32(arrlen)) {
        *outInt = (unsigned int) NPVARIANT_TO_INT32(arrlen);
    } else if (NPVARIANT_IS_DOUBLE(arrlen)) {
        *outInt = (unsigned int) NPVARIANT_TO_DOUBLE(arrlen);
    } else {
        rv = false;
    }

    releaseVariant(arrlen);
    
    return rv;
}

bool
npu::getArrayElement(NPP npp, const NPObject* array,
                     unsigned int i, NPVariant *outValue)
{
    unsigned int len;
    if (!getArrayLength(npp, array, &len)) return false;
    if (i >= len) return false;
    // On safari 4.0.3, snow leopard, getintidentifier doesn't work any more !?
    std::stringstream ss;
    ss << i;
    NPIdentifier id = gBrowserFuncs.getstringidentifier(ss.str().c_str());
    return gBrowserFuncs.getproperty(npp, (NPObject *) array, id, outValue);
}

bool
npu::getElementProperty(NPP instance, const NPVariant* parent,
                        const char* childId, NPVariant *outValue)
{
    NPIdentifier eid = gBrowserFuncs.getstringidentifier(childId);

    bool rv = false;

    if(parent->type == NPVariantType_Object)
    {
        rv = gBrowserFuncs.getproperty(instance,
                                       parent->value.objectValue, eid,
                                       outValue);
    }

    return rv;
}

bool
npu::getCurrentURL(NPP instance, std::string &outUrl)
{
    NPVariant window;
	NPVariant document;
	NPVariant docURL;

    docURL.type = NPVariantType_Void;
    
    if (!getWindow(instance, window) ||
        !getElementProperty(instance, &window, "document", &document))
    {
        return false;
    }

    // document.URL is the only thing that works in Safari
    // Firefox supports document.URL or document.location.href
    if (!getElementProperty(instance, &document, "URL", &docURL)) {
        NPVariant loc;
        if (getElementProperty(instance, &document, "location", &loc))
        {
            (void)getElementProperty(instance, &loc, "href", &docURL);
            releaseVariant(loc);
        }
    }

    bool rv = false;

    if (getStringValue(docURL, outUrl)) rv = true;

    releaseVariant(docURL);
    releaseVariant(document);
    releaseVariant(window);

    return rv;
}

bool
npu::callFunction(NPP instance, NPObject* obj, const NPVariant args[],
                  const int argCount, NPVariant* result)
{
    if (obj == NULL || result == NULL) return false;
    
    // (stolen from vargon by lth) So if you call invokeDefault() on
    // an object that's actually a function in Javascript, it will
    // invoke the function in Firefox, but not in Safari. However,
    // invoking the 'call' built-in method on the object appears to
    // work in both Firefox and Safari but requires that the first
    // argument passed in be a 'this' pointer to be used for the
    // function, so we'll pass in the object we're calling as 'this'
    result->type = NPVariantType_Void;

    NPIdentifier call =  gBrowserFuncs.getstringidentifier("call");
	
    NPVariant * vargs = (NPVariant *) calloc(argCount+1, sizeof(NPVariant));
    BPASSERT(vargs != NULL);
    vargs[0].type = NPVariantType_Object;
    vargs[0].value.objectValue = obj;
    memcpy((void *) (vargs + 1), (void *) args, sizeof(NPVariant) * argCount);

    bool invokeOK =
        gBrowserFuncs.invoke(instance, obj, call, vargs, argCount+1, result);

    free(vargs);

    return invokeOK;
}

bool
npu::evaluateScript(NPP npp, const std::string &script,
                    NPVariant * result)
{
    bool rval = false;
    NPObject* windowObject = NULL;
    NPError err = gBrowserFuncs.getvalue(npp,
                                         NPNVWindowNPObject,
                                         &windowObject);
    if (err == NPERR_NO_ERROR) {
        NPString eval;
        eval.utf8characters = script.c_str();
        eval.utf8length = script.length();
        rval = gBrowserFuncs.evaluate(npp, windowObject, &eval, result);
        gBrowserFuncs.releaseobject(windowObject);
    }
    return rval;
}

bool
npu::evaluateJSON(NPP npp, const bp::Object &obj,
                  NPVariant * result)
{
    std::string json = obj.toPlainJsonString();
    // For firefox on mac, evaluating an object yields null.  specifically:
    // "{foo: 'bar'}" -> null
    // "[{foo: 'bar'}][0]" -> object
    // don't ask me why. (lth)
    if (obj.type() == BPTMap) json = "[" + json + "][0]";
    
    bool rval = false;
    NPObject* windowObject = NULL;
    NPError err = gBrowserFuncs.getvalue(npp,
                                         NPNVWindowNPObject,
                                         &windowObject);
    if (err == NPERR_NO_ERROR) {
        NPString eval;
        eval.utf8characters = json.c_str();
        eval.utf8length = json.length();
        rval = gBrowserFuncs.evaluate(npp, windowObject, &eval, result);
        gBrowserFuncs.releaseobject(windowObject);
    }
    return rval;
}

bool
npu::generateErrorReturn(NPP npp, const char * error,
                         const char * verboseError, NPVariant * rv)
{
    if (error == NULL) error = "NPAPI.unknownError";
    
    bp::Map errorMap;
    errorMap.add("success", new bp::Bool(false));
    errorMap.add("error", new bp::String(error));
    if (verboseError) {
        errorMap.add("verboseError", new bp::String(verboseError));
    }

    return evaluateJSON(npp, errorMap, rv);
}


bool
npu::generateSuccessReturn(NPP npp, const bp::Object &value, NPVariant * rv)
{
    bp::Map successMap;
    successMap.add("success", new bp::Bool(true));

    // copy will be free'd when successMap goes out of scope.
    successMap.add("value", value.clone());

    return evaluateJSON(npp, successMap, rv);
}

std::vector<std::string>
npu::enumerateProperties(NPP npp, NPObject * obj)
{
    std::vector<std::string> props;
    uint32_t count;
    NPIdentifier* identifiers;

    if (gBrowserFuncs.enumerate(npp, obj, &identifiers, &count)) {
        for (uint32_t i = 0; i < count; i++) {
            NPUTF8* string = gBrowserFuncs.utf8fromidentifier(identifiers[i]);
            if (!string) continue;
            props.push_back(string);
            gBrowserFuncs.memfree(string);
        }
        gBrowserFuncs.memfree(identifiers);
    }

    return props;
}

bool
npu::isArray(NPP npp, const NPVariant * var)
{
    // can't be an array without being an object first
    if (!NPVARIANT_IS_OBJECT(*var)) return false;

    static std::string isArrayJSFunc(
        "function(x) {\n"
        "  return (x.constructor == Array);\n"
        "}\n");

    NPObject * function = createJSFunc(npp, isArrayJSFunc.c_str());

    if (!function)
    {
        // TODO: highly unexpected!  log?  throw?
        return false;
    }

    NPVariant result;
    result.type = NPVariantType_Void;

    if (!callFunction(npp, function, var, 1, &result) ||
        !NPVARIANT_IS_BOOLEAN(result))
    {
        // TODO: highly unexpected!  log?  throw?
        return false;
    }

    bool rv = NPVARIANT_TO_BOOLEAN(result);

    releaseVariant(result);
    releaseObject(function);
    
    return rv;
}

bool
npu::isFunction(NPP npp, const NPVariant * var)
{
    // can't be an function without being an object first
    if (!NPVARIANT_IS_OBJECT(*var)) return false;

    static std::string isFunctionJSFunc(
        "function(x) {\n"
        "  return (x.constructor == Function);\n"
        "}\n");

    NPObject * function = createJSFunc(npp, isFunctionJSFunc.c_str());

    if (!function)
    {
        // TODO: highly unexpected!  log?  throw?
        return false;
    }

    NPVariant result;
    result.type = NPVariantType_Void;

    if (!callFunction(npp, function, var, 1, &result) ||
        !NPVARIANT_IS_BOOLEAN(result))
    {
        // TODO: highly unexpected!  log?  throw?
        return false;
    }

    bool rv = NPVARIANT_TO_BOOLEAN(result);

    releaseObject(function);
    releaseVariant(result);
    
    return rv;
}

bool
npu::objectToJSON(const bp::Object * input, bp::Object * &output)
{
    // so first we gotta build up json
    if (input) output = BPHandleMapper::insertHandles(input);
    else output = new bp::Null;
    return true;
}

NPObject *
npu::createJSFunc(NPP npp, const char * funcBody)
{
    std::vector<std::string> props;

    NPVariant function;
    function.type = NPVariantType_Void;

    std::string funcBodyWrap("[");
    funcBodyWrap.append(funcBody);
    funcBodyWrap.append("][0]");    

    if (!evaluateScript(npp, funcBodyWrap.c_str(), &function) ||
        !NPVARIANT_IS_OBJECT(function))
    {
        std::cout << "failed to evaluate function: " << std::endl
                  << "----------------------------------------"
                  << funcBody
                  << "----------------------------------------"
                  << std::endl;
        return NULL;
    }
    
    return function.value.objectValue;
}

// decrement ref count on NPObject
void
npu::releaseObject(NPObject* obj)
{
    if (obj == NULL) return;
    NPVariant var;
    var.type = NPVariantType_Object;
    var.value.objectValue = obj;    
    releaseVariant(var);
}

// increment ref count on NPObject
void
npu::retainObject(NPObject* obj)
{
    if (obj == NULL) return;
    NPVariant var;
    var.type = NPVariantType_Object;
    var.value.objectValue = obj;    
    retainVariant(var);
}


