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
 * HTMLScriptObject.h - an abstraction around a scriptable object,
 *                      which may expose functions into javascript,
 *                      handling type conversion and dispatch.
 */

#include "HTMLTransactionContext.h"
#include "api/HTMLScriptObject.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include <Carbon/Carbon.h>
#include <WebKit/CarbonUtils.h>
#include <WebKit/HIWebView.h>

#include <iostream>
#include <sstream>
#include <string>

namespace bp { namespace html {
class ScriptFunctionHostProxyClass 
{
  public:
    static void release(ScriptableFunctionHost * host, unsigned int id)
    {
        host->release(id);
    }
    static unsigned int addTransaction(ScriptableFunctionHost * host,
                                       bp::html::BPHTMLTransactionContext * t)
    {
        return host->addTransaction(t);
    }
};
}}

using namespace bp::html;

// a recursive utility function to map from Foundation types to
// bp::Object
static bp::Object * mapFromFoundation(WebScriptObject * tlwso,
                                      NSObject * obj,
                                      BPHTMLTransactionContext & tc)
{
    bp::Object * robj = NULL;
    
    if (obj == NULL || [obj class] == [WebUndefined class]) {
        robj = new bp::Null;
    }
    else if ([obj isKindOfClass: [NSString class]]) {
        robj = new bp::String([(NSString *) obj UTF8String]);
    }
    else if ([obj isKindOfClass: [NSNumber class]])
    {
        NSNumber * numnum = (NSNumber *) obj;
        if (![[obj className] compare: @"NSCFBoolean"]) {
            robj = new bp::Bool([numnum boolValue]);
        } else {
            long long llv = [numnum longLongValue];
            double dv = [numnum doubleValue];
            if (dv != llv) {
                robj = new bp::Double(dv);
            } else {
                robj = new bp::Integer(llv);
            }
        }
    }
    else if ([obj isKindOfClass: [WebScriptObject class]]) {    
        bool isArray = false;
        {
            NSArray * args = [NSArray arrayWithObject: obj];
            NSNumber * n = [tlwso callWebScriptMethod: @"_bp_isArrayFunc"
                                  withArguments: args];
            isArray = [n boolValue];
        }

        bool isFunction = false;
        {
            NSArray * args = [NSArray arrayWithObject: obj];
            NSNumber * n = [tlwso callWebScriptMethod: @"_bp_isFunctionFunc"
                                  withArguments: args];
            isFunction = [n boolValue];
        }

        WebScriptObject * wso = (WebScriptObject *) obj;

        if (isArray) {
            bp::List * l = new bp::List;

            unsigned ix;
            for (ix = 0; true; ix++) 
            {
                id x = [wso webScriptValueAtIndex: ix];
                if ([x class] == [WebUndefined class]) break;
                bp::Object * child = mapFromFoundation(tlwso, x, tc);
                if (child != NULL) l->append(child);
            }
            robj = l;
        } else if (isFunction) {
            JSFunctionWrapper f((void *) tlwso, (void *) wso);
            robj = new bp::CallBack(f.id());
            // add the function to the transaction context
            tc.m_callbacks[f.id()] = f;
        } else {
            bp::Map * m = new bp::Map;

            // enumerate the keys
            NSArray * args = [NSArray arrayWithObject: wso];
            WebScriptObject * keys =
                [tlwso callWebScriptMethod: @"_bp_enumerateKeys"
                       withArguments: args];
            unsigned ix;
            for (ix = 0; true; ix++) 
            {
                id x = [keys webScriptValueAtIndex: ix];
                if ([x class] == [WebUndefined class]) break;
                bp::Object * child =
                    mapFromFoundation(tlwso, [wso valueForKey: x], tc);
                if (child != NULL) m->add([x UTF8String], child);
            
            }
            robj = m;
        }
        
    }
    
    return robj;
}

// a recursive utility function to map from bp::Object to
// Foundation types
static NSObject * mapToFoundation(WebScriptObject * tlwso,
                                  const bp::Object * obj)
{
    NSObject * robj = NULL;

    if (obj != NULL) {
        std::string jsonRep = "[";
        jsonRep += obj->toPlainJsonString(true);
        jsonRep += "][0];";
        NSString * str = [NSString stringWithUTF8String: jsonRep.c_str()];
        WebScriptObject * wso = [tlwso evaluateWebScript: str];
//        [wso retain];

        robj = wso;
    }

    return robj;
}

@interface AppleWebKitScriptableObject : NSObject {
@public
    std::map<std::string, ScriptableFunctionHost *> * functions;
    WebScriptObject * m_wso;
};

- (id)   invokeUndefinedMethodFromWebScript:(NSString *)name
                              withArguments:(NSArray *)args;

- (void) attachToWebScriptObject: (WebScriptObject *) wso;
@end;

@implementation AppleWebKitScriptableObject

- (void) attachToWebScriptObject: (WebScriptObject *) wso
{
    // define a utility function in the js env for enumerating keys
    std::string enumFunc(
        "function _bp_enumerateKeys(x) {\n"
        "  var i = 0;\n"
        "  var arr = new Array;\n"
        "  for (var y in x) {\n"
        "    if (!x.hasOwnProperty || x.hasOwnProperty(y))\n"
        "      arr[i++] = y;\n"
        "  }\n"
        "  return arr;\n"
        "}\n");

    [wso evaluateWebScript:
             [NSString stringWithUTF8String: enumFunc.c_str()]];

    // and another function to determine if an object is an array
    static std::string isArrayJSFunc(
        "function _bp_isArrayFunc(x) {\n"
        "  return (x.constructor == Array);\n"
        "}\n");

    [wso evaluateWebScript:
             [NSString stringWithUTF8String: isArrayJSFunc.c_str()]];

    // a function to test if an object is a function
    static std::string isFunctionJSFunc(
        "function _bp_isFunctionFunc(x) {\n"
        "  return (x.constructor == Function);\n"
        "}\n");

    [wso evaluateWebScript:
             [NSString stringWithUTF8String: isFunctionJSFunc.c_str()]];

    // and another function to allocate a object in javascript
    static std::string proxyToJS(
        "function _bp_proxyToJS(x) {\n"
        "  return x;\n"
        "}\n");

    [wso evaluateWebScript:
             [NSString stringWithUTF8String: proxyToJS.c_str()]];

    m_wso = wso;
}

- (id)invokeUndefinedMethodFromWebScript:(NSString *) name
                           withArguments:(NSArray *) nsArgs
{
    unsigned int i;
    bp::Object * rv = NULL;
    std::vector<bp::Object *> args;
    std::vector<const bp::Object *> constArgs;

    // allocate a new transaction context
    BPHTMLTransactionContext * tc = new BPHTMLTransactionContext;

    // map arguments into bp::Objects
    for (i=0; i<[nsArgs count]; i++) {
        bp::Object * o = mapFromFoundation(m_wso, [nsArgs objectAtIndex: i],
                                           *tc);
        if (o != NULL) {
            args.push_back(o);
            constArgs.push_back(o);
        }
    }

    std::map<std::string, ScriptableFunctionHost *>::iterator it;
    it = functions->find([name UTF8String]);
    if (it != functions->end()) {
        // now add our transaction context to the ScriptableFunctionHost
        unsigned int tid = 
            ScriptFunctionHostProxyClass::addTransaction(it->second, tc);

		// handle allowed std::string exceptions
		try {
            rv = it->second->invoke([name UTF8String], tid, constArgs);
		} catch(std::string s) {
            NSString * exc = [[NSString alloc] initWithUTF8String: s.c_str()];
            [exc autorelease];
            [m_wso setException: exc];
		}

        // and release our transaction context.  If the function wished
        // to preserve it for callback invocation, it will have already
        // called ScriptableFunctionHost::retain()
        ScriptFunctionHostProxyClass::release(it->second, tid);
    } else {
        delete tc;
    }

    // free arguments
    while (args.size() > 0) {
        delete args.back();
        args.pop_back();
    }
    
    if (it == functions->end()) {
        // throw exception, no such function
		std::stringstream ss;
        ss << "No such function: '" << [name UTF8String] << "'";
        NSString * exc = [[NSString alloc]
                             initWithUTF8String: ss.str().c_str()];
        [exc autorelease];
        [m_wso setException: exc];
    }

    // handle return value
    NSObject * nsrv = nil;
    
    if (rv) {
        nsrv = mapToFoundation(m_wso, rv);
        delete rv;
    }

    return nsrv;
}
@end

bp::html::ScriptableObject::~ScriptableObject()

{
    AppleWebKitScriptableObject * mso =
        (AppleWebKitScriptableObject *) m_osSpecific;
    if (mso) [mso release];
}

void *
bp::html::ScriptableObject::scriptableObject(void * topLevelScriptableObj)
{
    // allocate a scriptable object
    AppleWebKitScriptableObject * mso =
        [[AppleWebKitScriptableObject alloc] init];
    mso->functions = & m_functions;

    [mso attachToWebScriptObject: (WebScriptObject *) topLevelScriptableObj];

    m_osSpecific = (void *) mso;
    
    return m_osSpecific;
}


// function wrapper implementation.  Things could perhaps be broken
// into multiple files.
bp::Object *
JSFunctionWrapper::invoke(std::vector<const bp::Object *> args)
{
    bp::Object * rv = NULL;
    
    if (m_osCallback) {
        NSMutableArray * argsArr = [[NSMutableArray alloc] init];
        [argsArr addObject: (WebScriptObject *) m_osCallback];

        for (unsigned int i = 0; i < args.size(); i++) {
            NSObject * e =
                mapToFoundation((WebScriptObject *) m_osContext, args[i]);
            [argsArr addObject: e];
        }
        
        WebScriptObject * wso = (WebScriptObject *) m_osCallback;
        NSObject * obj = [wso callWebScriptMethod: @"call"
                                    withArguments: argsArr];

        [argsArr release];

        BPHTMLTransactionContext tc;
        rv = mapFromFoundation((WebScriptObject *) m_osContext,
                               obj, tc);
    }
    return rv;
}

void
JSFunctionWrapper::retain(void * o)
{
    if (o) [(WebScriptObject *) o retain];
}

void
JSFunctionWrapper::release(void * o)
{
    if (o) [(WebScriptObject *) o release];
}

