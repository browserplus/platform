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
 *  InterceptDropManager.h
 *  BrowserPlusPlugin
 *
 *  Created by Gordon Durand on 5/14/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __DROPMANAGER_H__
#define __DROPMANAGER_H__

#include <map>
#include <set>
#include <string>
#include <vector>
#include "common.h"
#include "PluginCommonLib/IDropManager.h"
#include "PluginCommonLib/DropTargetBase.h"
#include "BPUtils/bpfile.h"

class InterceptDropManager : public virtual IDropManager
{
public:
    // Well-known BrowserPlus methods which we invoke
    static const char* kGetTargetBounds;

    static IDropManager* allocate(NPP instance, 
                                  NPWindow* window,
                                  IDropListener* listener);
    virtual ~InterceptDropManager();

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
    virtual bool registerDropListener(IDropListener*) { 
        return true;
    }

    virtual void setWindow(NPWindow* window);

protected:
    // not publicly accessible, call allocate() to get the 
    // correct platform version
    InterceptDropManager(NPP instance, 
                         NPWindow* window,
                         IDropListener* listener);

    // not implemented, you may not copy.
    InterceptDropManager(const InterceptDropManager& dm);

    // derived classes may implement this if they care to know when
    // setwindow is called
    virtual void derivedSetWindow(NPWindow* window);

    // update all drop target bounds
    void updateBounds();

    // handle mouse entering/exiting the browser window
    void handleMouseEnter(const std::vector<bp::file::Path>& dragItems);
    void handleMouseExit(bool freeDragItems);

    // handle a mouse drag, where X,Y are browser relative coords,
    // minus any chrome.
    // returns true if the mouse is in a drop region which can
    // accept the drop, false otherwise
    bool handleMouseDrag(short int x, 
                         short int y);

    // handle a mouse drag, where X,Y are browser relative coords,
    // minus any chrome.
    //
    // returns true if the drop was handled, false if it should be
    // relayed to the browser.
    bool handleMouseDrop(short int x, 
                         short int y);
    
    // information about a particular target is stored in this
    // structure
    class DropTargetContext : public virtual DropTargetBase
    {
      public:
        DropTargetContext();
        DropTargetContext(const std::string& name,
                          const std::set<std::string>& mimeTypes,
                          bool includeGestureInfo,
                          unsigned int limit);
        DropTargetContext(const std::string& name,
                          const std::string& version);
        DropTargetContext(const DropTargetContext& dtc);
        virtual ~DropTargetContext() {}
        bool insideRegion(short int x, short int y);

        short int m_top;
        short int m_bottom;        
        short int m_left;
        short int m_right;
    };

    NPP m_instance;
    NPWindow* m_window;
    IDropListener* m_listener;

    std::map<std::string, DropTargetContext> m_targets;
    
    std::vector<bp::file::Path> m_dragItems;

    // a handle to the getXY function which attains the element's bounds
    NPObject* m_getXYFunction;
};

#endif
