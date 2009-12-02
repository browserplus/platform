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
 *  InterceptDropManager_Darwin.cpp
 *  BrowserPlusPlugin
 *
 *  Created by Gordon Durand on 5/14/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "BPScriptableObject.h"
#include "InterceptDropManager.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"
#include "nputils.h"
#include "common.h"
#include "PluginCommonLib/IDropListener.h"

#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>

using namespace std;

class CarbonDropManager : public virtual InterceptDropManager
{
   friend class CarbonDragDispatcher;
    
  public:
    CarbonDropManager(NPP instance,
                      NPWindow* window,
                      IDropListener* dl);
    ~CarbonDropManager();
    
    bool installHandlers();
    void removeHandlers();

  protected:
    void derivedSetWindow(NPWindow* window);
        
  private:
    static const int kMaxPathLength = 255;
    static OSErr specToPath(const FSSpec* spec,
                            UInt8* buf);

    bool getCurrentCoords(WindowRef theWindow,
                          DragReference theDragRef,
                          short int& x,
                          short int& y);
    vector<bp::file::Path> getDragItems(WindowRef theWindow,
                                        void* refcon,
                                        DragReference theDragRef);

    static pascal OSErr dragHandler(DragTrackingMessage message,
                                    WindowRef theWindow,
                                    void* refCon,
                                    DragReference theDragRef);

    static pascal OSErr receiveHandler(WindowRef theWindow,
                                       void* refcon,
                                       DragReference theDragRef);

    WindowRef m_windowRef;

    // 1 per target, active on drag
    unsigned int m_numDropWindows;
    WindowRef* m_dropWindows;
    void drawDropWindows();
    void releaseDropWindows();

    // windows that need to be reaped
    list<WindowRef> m_deadWindows;
};


// We can get multiple CarbonDropManagers associated with the
// same WindowRef.  To handle this, there is a CarbonDragDispatcher
// which handles the drag enter/exit for a window and tells
// each of the associated CarbonDropManagers to setup/destroy its
// temporal drop windows
//
class CarbonDragDispatcher
{
public:
    static CarbonDragDispatcher* get(WindowRef window) {
        map<WindowRef, CarbonDragDispatcher*>::iterator it;
        it = s_dispatchers.find(window);
        if (it == s_dispatchers.end()) {
            s_dispatchers[window] = new CarbonDragDispatcher(window);
            BPLOG_INFO_STRM("create CarbonDragDispatcher"
                            << BP_HEX_MANIP << s_dispatchers[window]
                            << " for window " << BP_HEX_MANIP << window);
        }
        return s_dispatchers[window];
    }

    void addDropManager(CarbonDropManager* dm) {
        BPLOG_INFO_STRM("insert CarbonDropManager " 
                        << BP_HEX_MANIP << dm 
                        << " into CarbonDragDispatcher" 
                        << BP_HEX_MANIP << this);
        m_dropManagers.insert(dm);
    }
    void removeDropManager(CarbonDropManager* dm) {
        BPLOG_INFO_STRM("remove CarbonDropManager " 
                        << BP_HEX_MANIP << dm 
                        << " from CarbonDragDispatcher" 
                        << BP_HEX_MANIP << this);
        m_dropManagers.erase(dm);
        if (m_dropManagers.empty()) {
            BPLOG_INFO_STRM("CarbonDragDispatcher " 
                            << BP_HEX_MANIP << this
                            << " deletes self");
            s_dispatchers.erase(dm->m_windowRef);
            delete this;
        }
    }
    
private:
    CarbonDragDispatcher(WindowRef window) : m_windowRef(window) {
        // verify machine features
        long response;
        if (Gestalt(gestaltDragMgrAttr, &response) == noErr) {  
            OSErr err = InstallTrackingHandler(bootStrapDragHandler, 
                                               m_windowRef, this);
            BPLOG_INFO_STRM("CarbonDragDispatcher " 
                            << BP_HEX_MANIP << this
                            << " installed tracking handler, err = " << err);
        } else {
            BPLOG_ERROR_STRM("CarbonDragDispatcher " 
                             << BP_HEX_MANIP << this
                             << " no drag manager gestalt");
        }
    }
    virtual ~CarbonDragDispatcher() {
        RemoveTrackingHandler(bootStrapDragHandler, m_windowRef);
    }
    static pascal OSErr bootStrapDragHandler(DragTrackingMessage message,
                                             WindowRef theWindow,
                                             void* refCon,
                                             DragReference theDragRef);
    WindowRef m_windowRef;
    set<CarbonDropManager*> m_dropManagers;
    static map<WindowRef, CarbonDragDispatcher*> s_dispatchers;
};

map<WindowRef, CarbonDragDispatcher*> CarbonDragDispatcher::s_dispatchers;

pascal OSErr
CarbonDragDispatcher::bootStrapDragHandler(DragTrackingMessage message,
                                           WindowRef theWindow,
                                           void* refCon,
                                           DragReference theDragRef)
{
    CarbonDragDispatcher* disp = (CarbonDragDispatcher*) refCon;    
    
    switch (message) {
        case kDragTrackingEnterWindow: {
            BPLOG_INFO_STRM("CarbonDragDispatcher "
                            << BP_HEX_MANIP << disp
                            << " gets kDragTrackingEnterWindow");
            set<CarbonDropManager*>::iterator it;
            for (it = disp->m_dropManagers.begin(); 
                 it != disp->m_dropManagers.end(); ++it) {
                CarbonDropManager* dm = *it;
                vector<bp::file::Path> dragItems = dm->getDragItems(theWindow,
                                                                    refCon,
                                                                    theDragRef);
                dm->handleMouseEnter(dragItems);
                dm->drawDropWindows();
                BPLOG_INFO_STRM("drawDropWindows for CarbonDropManager "
                                << BP_HEX_MANIP << dm);
            }
            break;
        }
        case kDragTrackingLeaveWindow: {
            // here we want to free the windows only if the
            // user is dragging _from_ the browser window off to the
            // desktop.  If the user is dragging from the browser
            // window into a drop target, we want to leave the drop
            // windows
            BPLOG_INFO_STRM("CarbonDragDispatcher "
                            << BP_HEX_MANIP << disp
                            << " gets kDragTrackingLeaveWindow");
            bool leaving = true;
            set<CarbonDropManager*>::iterator it;
            for (it = disp->m_dropManagers.begin(); 
                 it != disp->m_dropManagers.end(); ++it) {
                short x = 0, y = 0;
                CarbonDropManager* dm = *it;
                if (dm->getCurrentCoords(theWindow, theDragRef, x, y)
                    && dm->handleMouseDrag(x, y)) {
                    leaving = false;
                    break;
                }
            }
            if (leaving) {
                BPLOG_INFO("left main window");
                for (it = disp->m_dropManagers.begin(); 
                     it != disp->m_dropManagers.end(); ++it) {
                    CarbonDropManager* dm = *it;
                    dm->handleMouseExit(true);
                    dm->releaseDropWindows();
                    BPLOG_INFO_STRM("release DropWindows for CarbonDropManager "
                                    << BP_HEX_MANIP << dm);
                }
            } else {
                BPLOG_INFO("left main window, entered drop zone");
            }
            break;
        }
    }
    return noErr; // there's no point in confusing Drag Manager or its caller
}


IDropManager*
InterceptDropManager::allocate(NPP instance,
                               NPWindow* window,
                               IDropListener* dl)
{
    return new CarbonDropManager(instance, window, dl);
}


CarbonDropManager::CarbonDropManager(NPP instance,
                                     NPWindow* window,
                                     IDropListener* listener)
    : InterceptDropManager(instance, window, listener),
      m_numDropWindows(0), m_dropWindows(NULL)
{
    const NP_CGContext* ctx = (NP_CGContext*)window->window;
    m_windowRef = ctx->window;

    CarbonDragDispatcher* disp = CarbonDragDispatcher::get(m_windowRef);
    assert(disp != NULL);
    disp->addDropManager(this);
}


CarbonDropManager::~CarbonDropManager()
{
    releaseDropWindows();
    
    CarbonDragDispatcher* disp = CarbonDragDispatcher::get(m_windowRef);
    assert(disp != NULL);
    disp->removeDropManager(this);

    // now clean up all overlay drop windows allocated during interaction
    // with this page
    BPLOG_INFO_STRM("Cleaning up " << m_deadWindows.size() <<
                    " temporal drop windows");
    list<WindowRef>::iterator it;
    for (it = m_deadWindows.begin(); it != m_deadWindows.end(); it++) {
        DisposeWindow(*it);
    }
}


OSErr
CarbonDropManager::specToPath(const FSSpec* spec,
                              UInt8* buf)
{
    OSErr err;
    FSRef theRef;

    err = FSpMakeFSRef(spec, &theRef);
    if (err != noErr) {
        return err;
    }
    err = FSRefMakePath(&theRef, buf, kMaxPathLength);
    return err;
}


void
CarbonDropManager::derivedSetWindow(NPWindow* window)
{
    // if we've currently got drop windows rendered, we should
    // re-render them, because the window has moved.
    if (m_dropWindows) drawDropWindows();
}


bool
CarbonDropManager::getCurrentCoords(WindowRef theWindow,
                                    DragReference theDragRef,
                                    short int& x,
                                    short int& y)
{
    NP_CGContext* npgc = (NP_CGContext*)m_window->window;
    if (!npgc) {
        return false;
    }
    
    // we cannot drag to our own window
    OSErr err;
    DragAttributes dragAttrs;
    if ((err = GetDragAttributes(theDragRef, &dragAttrs)) != noErr) {
        return false;
    }
    if ((dragAttrs & kDragInsideSenderWindow) != 0) {
        return false;
    }

    // Get mouse position in global coordinates
    Point deviceMouse;
    GetDragMouse(theDragRef, &deviceMouse, 0L);
    
    // Get window bounds
    Rect bounds;
    GetWindowBounds(theWindow, kWindowStructureRgn, &bounds);

    BPLOG_DEBUG_STRM("window (l/t/r/b): " << bounds.left
                     << "," << bounds.top
                     << " / " << bounds.right
                     << "," << bounds.bottom);

    BPLOG_DEBUG_STRM("mouse (h/v): " << deviceMouse.h
                     << "," << deviceMouse.v);

    // Now if the device mouse falls outside of the window bounds,
    // this is an invalid drag.
    if (deviceMouse.h < bounds.left || deviceMouse.h > bounds.right || 
        deviceMouse.v < bounds.top || deviceMouse.v > bounds.bottom) {
        BPLOG_DEBUG("mouse outside of window");
        return false;
    }

    // Convert to local coordinates
    Point localMouse = deviceMouse;
    localMouse.v -= bounds.top;
    localMouse.h -= bounds.left;
    BPLOG_DEBUG_STRM("localMouse (v/h): " << localMouse.v
                     <<  ", " << localMouse.h);
    
    // Now account for chrome
    x = localMouse.h + m_window->x;
    y = localMouse.v - m_window->y;

    return true;
}


pascal OSErr
CarbonDropManager::receiveHandler(WindowRef theWindow,
                                  void* refcon,
                                  DragReference theDragRef)
{
    CarbonDropManager* dm = (CarbonDropManager*) refcon;
    BPLOG_INFO_STRM("CarbonDropManager "
                    << BP_HEX_MANIP << dm
                    << " receiveHandler");

    short int x = 0, y = 0;
    if (!dm->getCurrentCoords(dm->m_windowRef, theDragRef, x, y)) {
        return userCanceledErr;
    }

    dm->handleMouseDrop(x, y);
    dm->releaseDropWindows();
    dm->handleMouseExit(true);

    return noErr;
}


vector<bp::file::Path>
CarbonDropManager::getDragItems(WindowRef theWindow,
                                void* refcon,
                                DragReference theDragRef)
{
    vector<bp::file::Path> results;
    OSErr err = noErr;
    // how many items
    UInt16 itemCount;
    if ((err = CountDragItems(theDragRef, &itemCount)) != noErr) {
        return results;
    }
    if (itemCount < 1) {
        return results;
    }
    
    short int x = 0, y = 0;
    if (!getCurrentCoords(m_windowRef, theDragRef, x, y)) {
        return results;
    }
    
    for (int i = 1; i <= itemCount; i++) {
        UInt8 path[kMaxPathLength];
        ItemReference theItem;
        err = GetDragItemReferenceNumber(theDragRef, i, &theItem);
        if (err != noErr) {
            return results;
        }
        
        // try to get a  HFSFlavor
        HFSFlavor targetFile;
        Size theSize = sizeof(HFSFlavor);
        err = GetFlavorData(theDragRef, theItem, kDragFlavorTypeHFS,
                            &targetFile, &theSize, 0);
        if (err == noErr) {
            err = specToPath(&targetFile.fileSpec, path);
            if (err == noErr) {
                bp::file::Path p(bp::file::nativeFromUtf8((const char*)path));
                results.push_back(p);
            }
        } else {
            // try to get a  promised HFSFlavor
            PromiseHFSFlavor targetPromise;
            theSize = sizeof(PromiseHFSFlavor);
            err = GetFlavorData(theDragRef, theItem, kDragFlavorTypePromiseHFS,
                                &targetPromise, &theSize, 0);
            if (err == noErr) {
                err = specToPath(&targetFile.fileSpec, path);
                if (err == noErr) {
                    bp::file::Path p(bp::file::nativeFromUtf8((const char*)path));
                    results.push_back(p);
                }
            }
        }
    }
    return results;
}


static WindowGroupRef bplusGroup;

void
CarbonDropManager::drawDropWindows()
{
    releaseDropWindows();
    assert(m_dropWindows == NULL);

    m_numDropWindows = m_targets.size();
    m_dropWindows = (WindowRef*) calloc(m_numDropWindows, sizeof(WindowRef));
    
    map<string, DropTargetContext>::iterator target;

    int i = 0;

    // at drag start time we allocate a window group.  This window group
    // tells the window manager that our drop overlay windows and the
    // browser window should be considered to be one layer.  that is,
    // nothing from another app should be able to slip between a browser
    // and it's temporal drop target windows.  (YIB-1615269).
    if (!bplusGroup) {
        CreateWindowGroup(kWindowGroupAttrMoveTogether |
                          kWindowGroupAttrLayerTogether,
                          &bplusGroup);
        BPLOG_INFO_STRM("Created window group: " << bplusGroup);
        SetWindowGroupName(
            bplusGroup,
            CFSTR("com.yahoo.BrowserPlus.docWindowAndOverlayGroup"));
        WindowGroupRef og = GetWindowGroup(m_windowRef);
        SetWindowGroupParent(bplusGroup, og);
        SetWindowGroup(m_windowRef, bplusGroup);
    }
    
    // get bounds of browser window
    Rect bWindow;
    GetWindowBounds(m_windowRef, kWindowStructureRgn, &bWindow);
    BPLOG_DEBUG_STRM("window (l/t/r/b): "
                     << bWindow.left << " "
                     << bWindow.top << " "
                     << bWindow.right << " "
                     << bWindow.bottom);

    // and account for scrolling
    bWindow.top += m_window->y;
    bWindow.left += m_window->x;


    
    for (i = 0, target = m_targets.begin();
         target != m_targets.end();
         i++, target++) {
        Rect r;
        r.top = target->second.m_top;
        r.bottom =  target->second.m_bottom;
        r.left =  target->second.m_left;
        r.right =  target->second.m_right;
        BPLOG_DEBUG_STRM("target inside browser page (l/t/r/b): "
                         << r.left << " "
                         << r.top << " "
                         << r.right << " "
                         << r.bottom);

        // now position target within browser window
        r.top += bWindow.top;
        r.bottom += bWindow.top;
        r.left += bWindow.left;
        r.right += bWindow.left;

        BPLOG_DEBUG_STRM("create drop overlay at (l/t/r/b): "
                         << r.left << " "
                         << r.top << " "
                         << r.right << " "
                         << r.bottom);

        OSErr x = CreateNewWindow(
                        kPlainWindowClass,
                        kWindowNoShadowAttribute |kWindowOpaqueForEventsAttribute |
                        kWindowNoActivatesAttribute | kWindowDoesNotCycleAttribute,
                        &r,
                        &(m_dropWindows[i]));

        // make window transparent
        // for debugging, a value of 0.5 can be handy
        SetWindowAlpha(m_dropWindows[i], 0.0);
        
        x = InstallReceiveHandler(receiveHandler, m_dropWindows[i], this);
        x = InstallTrackingHandler(dragHandler, m_dropWindows[i], this);

        // By adding the new drop window to the "bplusGroup" which is a
        // window group with the "layer together" property, we gaurantee
        // that never will a window come between the browser and the drop
        // targets.  (YIB-1615269).
        SetWindowGroup(m_dropWindows[i], bplusGroup);
        SendBehind(m_windowRef, m_dropWindows[i]);
        ShowWindow(m_dropWindows[i]);
    }

    // debug output
//     {
//         int x = 0;
//         WindowRef wr;

//         bp::debug::print("All Windows:");
//         for (wr = GetFrontWindowOfClass(kAllWindowClasses, false);
//              wr != NULL;
//              wr = GetNextWindowOfClass(wr, kAllWindowClasses, false))
//         {
//             WindowGroupRef grp = GetWindowGroup(wr);
//             WindowGroupAttributes attrs =
//                 GetWindowGroupAttributes(grp, &attrs);
//             bp::debug::print("%d: w:%p g:%p atr:0x%x", ++x, wr, grp, attrs);
//         }

//         bp::debug::print("All Windows Groups:\n");
//         DebugPrintAllWindowGroups();
//         bp::debug::print("My window reference: %p", m_windowRef);
//         bp::debug::print("BPlus group:\n");
//         DebugPrintWindowGroup(bplusGroup);
//     }
}


void
CarbonDropManager::releaseDropWindows()
{
    if (m_dropWindows != NULL) {
        WindowGroupRef docGrp = GetWindowGroupOfClass(kDocumentWindowClass);
        SetWindowGroup(m_windowRef, docGrp);

        // can this thing be re-entrant?  Can HideWindow cause
        // releaseDropWindows to be called again?!
        // seen some hard to reproduce cases where this appears to
        // explain the data.  Shouldn't see them anymore.
        WindowRef* dropWindows = m_dropWindows;
        m_dropWindows = NULL;

        for (unsigned int i = 0; i < m_numDropWindows; i++) {
            RemoveReceiveHandler(receiveHandler, dropWindows[i]);
            RemoveTrackingHandler(dragHandler, dropWindows[i]);
            HideWindow(dropWindows[i]);
            // (lth) Workaround:  On leopard/safari we cannot
            //       immediately dispose of this window, or we will get a
            //       subsequent crash in SetGWorld.  Cause unknown.
            //       (YIB-1609997)
            // DisposeWindow(dropWindows[i]);
            m_deadWindows.push_back(dropWindows[i]);
        }
        free(dropWindows);
        ReleaseWindowGroup(bplusGroup);
        bplusGroup = NULL;
    }
}


pascal OSErr
CarbonDropManager::dragHandler(DragTrackingMessage message,
                               WindowRef theWindow,
                               void* refCon,
                               DragReference theDragRef)
{
    CarbonDropManager* dm = (CarbonDropManager*) refCon;    

    switch (message) {
        case kDragTrackingEnterWindow: {
            short x1 = 0, y1 = 0;
            if (dm->getCurrentCoords(dm->m_windowRef, theDragRef, x1, y1)) {
                BPLOG_INFO_STRM("CarbonDropManager "
                                << BP_HEX_MANIP << dm
                                << " enter drop window: " 
                                << x1 << "," << y1);
            }
        }
            
        case kDragTrackingInWindow: {
            short x = 0, y = 0;
            if (dm->getCurrentCoords(dm->m_windowRef, theDragRef, x, y)) {
                // if this drag is not inside the browser window,
                // then it's time to clean up our drop targets.  
                BPLOG_DEBUG_STRM("handleMouseDrag(" << x << ", " << y << ")");
                dm->handleMouseDrag(x,y);
            } else {
                // oops.  Our drop windows are out of place!
                BPLOG_DEBUG_STRM("handleMouseExit(true)");
                dm->releaseDropWindows();
                dm->handleMouseExit(true);
            }
            break;
        }
            
        case kDragTrackingLeaveWindow: 
            // now set all states of targets to Idle
            BPLOG_INFO_STRM("CarbonDropManager "
                            << BP_HEX_MANIP << dm
                            << " leave drop window");
            dm->handleMouseExit(false);
            break;
    }
    return noErr; // there's no point in confusing Drag Manager or its caller
}



