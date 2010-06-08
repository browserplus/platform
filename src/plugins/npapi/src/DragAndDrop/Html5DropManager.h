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
 *  Html5DropManager.h
 *  BrowserPlusPlugin
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __HTML5DROPMANAGER_H__
#define __HTML5DROPMANAGER_H__

#include "common.h"
#include "PluginCommonLib/IDropManager.h"
#include "PluginCommonLib/DropTargetBase.h"
#include "GenericJSObject.h"

class Html5DropManager : public virtual IDropManager
{
public:
    static IDropManager* allocate(NPP instance, 
                                  NPWindow* window,
                                  IDropListener* listener,
                                  const std::string& platform,
                                  const std::string& browser);
    virtual ~Html5DropManager();

    // IDropManager interface
    virtual bool addTarget(const std::string& name,
                           const std::set<std::string>& mimeTypes,
                           bool includeGestureInfo,
                           unsigned int limit);
    virtual bool addTarget(const std::string& name,
                           const std::string& version);
    virtual bool removeTarget(const std::string& name);
    virtual bool enableTarget(const std::string& name,
                              bool enable);
    virtual bool registerDropListener(IDropListener*);

protected:
    // not publicly accesible, you must call allocate to get
    // the correct platform version of the DropManager
    Html5DropManager(NPP instance, 
                     NPWindow* window, 
                     IDropListener* listener,
                     const std::string& platform,
                     const std::string& browser);

    // not implemented, you may not copy.
    Html5DropManager(const Html5DropManager& dm);

    NPP m_npp;

    // a javascript function to add a drop target.
    NPObject * m_addDropTargetFunc;

    // a javascript function to remove a drop target.
    NPObject * m_removeDropTargetFunc;

    // the listener that gets called when interesting events occur
    IDropListener* m_listener;

    std::string m_platform;
    std::string m_browser;

    // how a drop target is represented
    class DropTargetContext : public virtual DropTargetBase,
                              public virtual IJSCallableFunctionHost // the hook used for javascript to call us back
    {
      public:
        DropTargetContext(NPP m_npp,
                          const std::string& name,
                          const std::set<std::string>& mimeTypes,
                          bool includeGestureInfo,
                          unsigned int limit,
                          Html5DropManager * theMan);

        DropTargetContext(NPP m_npp,
                          const std::string& name,
                          const std::string& version,
                          Html5DropManager * theMan);

        virtual ~DropTargetContext();

        // a scriptable callback object suportting 3 functions is allocated during context construction
        NPObject * callbackObject() { return m_go; }

      private:
        // incoming calls from javascript use this side door
        virtual bp::Object * invoke(const std::string & funcName,
                                    const NPVariant* args,
                                    uint32_t argCount);

        DropTargetContext();
        DropTargetContext(const DropTargetContext& dtc);
        NPP m_npp;
        BPGenericObject * m_go;
        Html5DropManager * m_theMan;
    };

    // all currently active drop targets
    std::map<std::string, DropTargetContext *> m_targets;
};

#endif
