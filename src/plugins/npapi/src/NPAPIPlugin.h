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

/**
 *
 */

#ifndef __NPAPIPLUGIN_H__
#define __NPAPIPLUGIN_H__

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include "PluginCommonLib/BPPlugin.h"
#include "PluginCommonLib/BPSession.h"

class NPAPIPlugin : public BPPlugin
{
  public:
    NPAPIPlugin(NPP npp);
    ~NPAPIPlugin();    

    void setWindow(NPWindow* window);
    NPObject* scriptableObject();

    BPSession & session() { return m_session; }

    virtual std::string getUserAgent() const;

    virtual void setConnected();

    virtual bp::BrowserInfo getBrowserInfo();

private:
    NPP m_npp;
    NPObject* m_scriptableObject;

    // functions required by BPPlugin
    virtual plugin::Variant* allocVariant() const;
    
    virtual void * getWindow() const;

    virtual void plugletsSetWindow();

    virtual bool callJsFunction( const plugin::Object* oFunc,
                                 plugin::Variant* args[], int nArgCount,
                                 plugin::Variant* pvtRet ) const;
    
    // Creates appropriate plugin-specific pluglets.
    // Callers own the returned ptrs.
    virtual std::list<Pluglet*> createPluglets( const std::string& sName ) const;
    
    virtual bool enumerateProperties( const plugin::Object* pObj,
                                      std::vector<std::string>& vsProps ) const;
    
    virtual bool evaluateJSON( const bp::Object* pObj,
                               plugin::Variant* pvtRet ) const;

    virtual void freeVariant( plugin::Variant* vt ) const;

    virtual bool getArrayElement( const plugin::Object* pObj,
                                  int nIdx,
                                  plugin::Variant* pvtElem ) const;
    
    // Get the length of the provided array object.
    virtual bool getArrayLength( const plugin::Object* pObj,
                                 int& nLength ) const;

    virtual bool getCurrentURL( std::string& sUrl ) const;

    // Pull the named property out of the object which the variant must hold.
    virtual bool getElementProperty( const plugin::Variant* pvtIn,
                                     const std::string& sPropName,
                                     plugin::Variant* pvtVal ) const;
    
    virtual bool getProperty( const plugin::Object* pObj,
                              const std::string& sPropName,
                              plugin::Variant* pvtVal ) const;

    virtual bool isJsArray( const plugin::Object* pObj ) const;
    
    virtual bool isJsFunction( const plugin::Object* pObj ) const;

    BPSession m_session;

    bp::BrowserInfo m_browserInfo;

    NPWindow* m_npWindow;

    bool m_connected;
};

#endif
