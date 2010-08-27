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
 *  BPPlugin.h
 *
 *  Declares the interface IPlugin implemented by plugins and other
 *  basic plugin-related items.
 *  This is a good place to declare methods that are plugin-specific,
 *  but must be accessed by plugin common code.
 *  
 *  Created by David Grigsby on 10/18/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */
#ifndef __BPPLUGIN_H__
#define __BPPLUGIN_H__

#include <string>
#include <vector>
#include <list>
#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpbrowserinfo.h"


// Forward decls
class Pluglet;

namespace plugin {
class Object;
class Variant;
}



class BPPlugin
{
public:
    virtual ~BPPlugin() { }
    
    virtual plugin::Variant* allocVariant() const = 0;

    virtual bool callJsFunction( const plugin::Object* oFunc,
                                 plugin::Variant* args[], int nArgCount,
                                 plugin::Variant* pvtRet ) const = 0;
    
    // Creates appropriate plugin-specific pluglets.
    // Callers own the returned ptrs.
    virtual std::list<Pluglet*> createPluglets( const std::string& sName ) const = 0;

    virtual bool enumerateProperties( const plugin::Object* pObj,
                                      std::vector<std::string>& vsProps ) const = 0;
    
    virtual bool evaluateJSON( const bp::Object* pObj,
                               plugin::Variant* pvtRet ) const = 0;

    virtual void freeVariant( plugin::Variant* vt ) const = 0;

    // Set pvtElem's plugin-specific data to point to the requested
    // element of the provided array object.
    virtual bool getArrayElement( const plugin::Object* pObj,
                                  int nIdx,
                                  plugin::Variant* pvtElem ) const = 0;

    // Get the length of the provided array object.
    virtual bool getArrayLength( const plugin::Object* pObj,
                                 int& nLength ) const = 0;

    // sUrl - the page/frame to which the plugin is attached
    //        string encoding varies by browser:
    //        IE: UTF-8 encoded
    //        XP FF: IDNA ascii-encoded
    virtual bool getCurrentURL( std::string& sUrl ) const = 0;

    // TODO: this method could probably be eliminated.
    //       Clients would call plugin::Variant::getObjectValue, then
    //       getProperty.
    // Pull the named property out of the object which the variant must hold.
    virtual bool getElementProperty( const plugin::Variant* pvtIn,
                                     const std::string& sPropName,
                                     plugin::Variant* pvtVal ) const = 0;
    
    virtual bool getProperty( const plugin::Object* pObj,
                              const std::string& sPropName,
                              plugin::Variant* pvtVal ) const = 0;

    // get the OS specific pointer to the browser window, HWND or WindowRef 
    // The caller does *not* own the returned pointer.
    virtual void* getWindow() const = 0;
    
    virtual bool isJsArray( const plugin::Object* pObj ) const = 0;
    
    virtual bool isJsFunction( const plugin::Object* pObj ) const = 0;

    virtual std::string getUserAgent() const = 0;

    // tell the plugin that it's connection to the daemon is complete
    virtual void setConnected() = 0;

    virtual bp::BrowserInfo getBrowserInfo() = 0;
};



#endif // __BPPLUGIN_H__
