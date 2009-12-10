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

/**
 *
 */

// Disable "nameless struct/union" warnings that come from winnt.h that is 
// evidently included through some npapi header.
#ifdef WIN32
#pragma warning (disable: 4201)
// Disable 'this': used in base member initialize list warning
#pragma warning (disable: 4355)
#endif

#include "BPScriptableObject.h"
#include "DragAndDrop/DnDPlugletFactoryNPAPI.h"
#include "NPAPIObject.h"
#include "NPAPIPlugin.h"
#include "NPAPIVariant.h"
#include "nputils.h"
#include "PluginCommonLib/FileBrowsePlugletFactory.h"
#include "PluginCommonLib/LogPlugletFactory.h"
#include "WindowedPluglet.h"

#include <vector>
#include <list>

NPAPIPlugin::NPAPIPlugin(NPP npp)
    : BPPlugin(), m_npp(npp), m_scriptableObject(NULL), m_session(this),
      m_windowPtr(NULL)
{
}
    
NPAPIPlugin::~NPAPIPlugin()
{
}

void
NPAPIPlugin::setWindow(NPWindow* window)
{
    // iterate through all pluglets and inform the ones that need to
    // know
    if (m_scriptableObject) {
        std::list<Pluglet *> pluglets =
            m_session.getPlugletRegistry()->availablePluglets();
        std::list<Pluglet *>::iterator pluglet;
    
        for (pluglet = pluglets.begin(); pluglet != pluglets.end(); pluglet++)
        {
            WindowedPluglet * wpluglet =
                dynamic_cast<WindowedPluglet *>(*pluglet);
            if (wpluglet) wpluglet->setWindow(window);
        }
    }
    
    // get the os specific window reference
#ifdef WIN32
    // this is a HWND
    m_windowPtr = (void *) (window->window);
#else
    // this is a WindowRef (may be null)
    NP_CGContext* ctx = (NP_CGContext*)window->window;
    m_windowPtr = ctx->window;
#endif
}

void *
NPAPIPlugin::getWindow() const
{
    return m_windowPtr;
}


NPObject*
NPAPIPlugin::scriptableObject()
{
    if (!m_scriptableObject) {
        m_scriptableObject = BPScriptableObject::createObject(m_npp);
    }
    
    if (m_scriptableObject) {
        gBrowserFuncs.retainobject(m_scriptableObject);
    }
    
    return m_scriptableObject;
}

// functions required by BPPlugin
plugin::Variant*
NPAPIPlugin::allocVariant() const
{
    return new NPAPIVariant();
}


bool
NPAPIPlugin::callJsFunction( const plugin::Object* oFunc,
                             plugin::Variant* args[], int nArgCount,
                             plugin::Variant* pvtRet ) const
{
    std::vector<NPVariant> vArgs;
    for (int i=0; i<nArgCount; ++i)
    {
        vArgs.push_back( *(dynamic_cast<NPAPIVariant*>(args[i])->varPtr()) );
    }

    return npu::callFunction(m_npp,
                             ((NPAPIObject *) oFunc)->object(),
                             &(vArgs.front()), nArgCount,
                             ((NPAPIVariant *) pvtRet)->varPtr());
}

std::list<Pluglet*>
NPAPIPlugin::createPluglets( const std::string& sName ) const
{
    std::list<Pluglet*> rval;
    if (!sName.compare("DragAndDrop")) {
        DnDPlugletFactoryNPAPI factory;
        rval = factory.createPluglets(m_npp, (BPPlugin *) this);
    } else if (!sName.compare("FileBrowser")) {
        FileBrowsePlugletFactory factory;
        rval = factory.createPluglets((BPPlugin *) this);
    } else if (!sName.compare("Log")) {
        LogPlugletFactory factory;
        rval = factory.createPluglets((BPPlugin *) this);
    }
    
    return rval;
}

bool
NPAPIPlugin::enumerateProperties( const plugin::Object* pObj,
                                  std::vector<std::string>& vsProps ) const
{
    vsProps = npu::enumerateProperties(m_npp, ((NPAPIObject*) pObj)->object());
    return true;
}

    
bool
NPAPIPlugin::evaluateJSON( const bp::Object* pObj,
                           plugin::Variant* pvtRet ) const
{
    return npu::evaluateJSON(m_npp, *pObj,
                             ((NPAPIVariant*) pvtRet)->varPtr());
}


void
NPAPIPlugin::freeVariant( plugin::Variant* vt ) const
{
    if (vt == NULL) return;
    npu::releaseVariant(*(((NPAPIVariant *) vt)->varPtr()));
    delete vt;
}

bool
NPAPIPlugin::getArrayElement(const plugin::Object* pObj,
                             int nIdx,
                             plugin::Variant* pvtElem) const
{
    return npu::getArrayElement(m_npp,
                                ((NPAPIObject *) pObj)->object(),
                                nIdx,
                                ((NPAPIVariant*) pvtElem)->varPtr());
}
    
bool
NPAPIPlugin::getArrayLength(const plugin::Object* pObj,
                            int& nLen) const
{
    unsigned int uiLen;
    if (!npu::getArrayLength(m_npp,
                             ((NPAPIObject *) pObj)->object(),
                             &uiLen)) {
        return false;
    }

    nLen = static_cast<int>(uiLen);
    return true;
}

bool
NPAPIPlugin::getCurrentURL( std::string& sUrl ) const
{
    return npu::getCurrentURL(m_npp, sUrl);
}


bool
NPAPIPlugin::getElementProperty( const plugin::Variant* pvtIn,
                                 const std::string& sPropName,
                                 plugin::Variant* pvtVal ) const
{
    return npu::getElementProperty(m_npp, ((NPAPIVariant *) pvtIn)->varPtr(),
                                   sPropName.c_str(),
                                   ((NPAPIVariant *) pvtVal)->varPtr());
}

    
bool
NPAPIPlugin::getProperty( const plugin::Object* pObj,
                          const std::string& sPropName,
                          plugin::Variant* pvtVal ) const
{
    NPVariant v;
    v.type = NPVariantType_Object;
    v.value.objectValue = ((NPAPIObject *) pObj)->object();

    return npu::getElementProperty(m_npp, &v,
                                   sPropName.c_str(),
                                   ((NPAPIVariant *) pvtVal)->varPtr());
}

bool
NPAPIPlugin::isJsArray( const plugin::Object* pObj ) const
{
    if (pObj == NULL) return false;
    NPVariant v;
    v.type = NPVariantType_Object;
    v.value.objectValue = ((NPAPIObject *) pObj)->object();
    return npu::isArray(m_npp, &v);
}

bool
NPAPIPlugin::isJsFunction( const plugin::Object* pObj ) const
{
    if (pObj == NULL) return false;
    NPVariant v;
    v.type = NPVariantType_Object;
    v.value.objectValue = ((NPAPIObject *) pObj)->object();
    return npu::isFunction(m_npp, &v);
}

std::string
NPAPIPlugin::getUserAgent() const 
{
    const char * ua = NULL; 
    ua = gBrowserFuncs.uagent(m_npp);
    return std::string((ua ? ua : "unknown"));
}
