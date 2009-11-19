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

#include "PluginCommonLib/IDropListener.h"
#include "DragAndDrop/InterceptDropManager.h"
#include "nputils.h"
#include "BPUtils/BPLog.h"

using namespace std;

#ifdef WIN32
// Suppress "conditional expression is a constant" warning generated by 
// NPVariant macros.
#pragma warning (disable: 4127)

#define sscanf sscanf_s
#endif


static 
NPObject* compileGetXY(NPP npp)
{
    /**
     * Adapted from YUI
     *
     * Gets the current position of an element based on page coordinates.  
     * Element must be part of the DOM tree to have page coordinates 
     * (display:none or elements not appended return false).
     * @param {String/HTMLElement/Array} el Accepts a string to use as an ID, 
     * an actual DOM reference, or an Array of IDs and/or HTMLElements
     * @ return {Array} The XY position of the element(s)
     */
    static const char * funcBody = 
        "function(name) {\n"
        "  var el = document.getElementById(name);\n"
        "  if (!el) { return false; }\n" 
        // has to be part of document to have pageXY
        "  if (el.parentNode === null || el.style.display == 'none') {\n"
        "    return false;\n"
        "  }\n"
        "  var parentNode = null;\n"
        "  var pos = [el.offsetLeft, el.offsetTop];\n"
        "  parentNode = el.offsetParent;\n"
        "  if (parentNode != el) {\n"
        "    while (parentNode) {\n"
        "      pos[0] += parentNode.offsetLeft;\n"
        "      pos[1] += parentNode.offsetTop;\n"
        "      parentNode = parentNode.offsetParent;\n"
        "    }\n"
        "  }\n"
        "  if ((navigator.vendor && navigator.vendor.indexOf(\"Apple\") != -1)\n"
        "      && el.style.position == 'absolute') {\n"
        // safari doubles in some cases
        "    pos[0] -= document.body.offsetLeft;\n"
        "    pos[1] -= document.body.offsetTop;\n"
        "  } \n"
        
        "  if (el.parentNode) { \n"
        "    parentNode = el.parentNode;\n"
        "  } else { \n"
        "    parentNode = null;\n"
        "  }\n"

        "  while (parentNode && \n"
        "         parentNode.tagName.toUpperCase() != 'BODY' && \n"
        "         parentNode.tagName.toUpperCase() != 'HTML') {\n"
        // account for any scrolled ancestors
        "    pos[0] -= parentNode.scrollLeft;\n"
        "    pos[1] -= parentNode.scrollTop;\n"
        "    if (parentNode.parentNode) { \n"
        "      parentNode = parentNode.parentNode;\n"
        "    } else { \n"
        "      parentNode = null;\n"
        "    }\n"
        "  }\n"

        "  var str = parseInt(pos[1]) + ' ' "
        "            + parseInt(pos[0]) + ' ' " 
        "            + (parseInt(pos[1]) + parseInt(el.offsetHeight)) + ' '"
        "            + (parseInt(pos[0]) + parseInt(el.offsetWidth));"
        "  return str\n"
        "}\n";

    return npu::createJSFunc(npp, funcBody);
}


const char* InterceptDropManager::kGetTargetBounds = 
    "BrowserPlus._getDropTargetBounds";  // (name) appended

InterceptDropManager::DropTargetContext::DropTargetContext()
    : DropTargetBase(), 
      m_top(0), m_bottom(0), m_left(0), m_right(0)
{
}


InterceptDropManager::DropTargetContext::DropTargetContext(const string& name,
                                                           const set<string>& mimetypes,
                                                           bool includeGestureInfo,
                                                           unsigned int limit)
    : DropTargetBase(name, mimetypes, includeGestureInfo, limit), 
      m_top(0), m_bottom(0), m_left(0), m_right(0)
{
}


InterceptDropManager::DropTargetContext::DropTargetContext(const DropTargetContext& dtc)
    : DropTargetBase(dtc),
      m_top(dtc.m_top), m_bottom(dtc.m_bottom),
      m_left(dtc.m_left), m_right(dtc.m_right)
{
}


bool
InterceptDropManager::DropTargetContext::insideRegion(short int x,
                                                      short int y)
{
    return(x > m_left && x < m_right 
           && y > m_top && y < m_bottom);
}


InterceptDropManager::InterceptDropManager(NPP instance,
                                           NPWindow* window,
                                           IDropListener* listener)
    : m_instance(instance), m_window(window), m_listener(listener),
      m_getXYFunction(NULL)
{
    m_getXYFunction = compileGetXY(instance);
    if (m_getXYFunction == NULL) {
        BPLOG_FATAL("Couldn't compile getXY function!");
    }
}


InterceptDropManager::~InterceptDropManager()
{
    npu::releaseObject(m_getXYFunction);
}


bool
InterceptDropManager::addTarget(const string& name,
                                const set<string>& mimeTypes,
                                bool includeGestureInfo,
                                unsigned int limit)
{
    if (m_targets.find(name) != m_targets.end()) return false;
    DropTargetContext dtc(name, mimeTypes, includeGestureInfo, limit);
    m_targets[name] = dtc;
    return true;
}


bool
InterceptDropManager::removeTarget(const string& name)
{
    map<string, DropTargetContext>::iterator it;
    it = m_targets.find(name);;

    if (it == m_targets.end()) return false;
    m_targets.erase(it);
    return true;
}


bool
InterceptDropManager::enableTarget(const string& name,
                                   bool enable)
{
    if (m_targets.find(name) == m_targets.end()) return false;
    m_targets[name].enable(enable);
    return true;
}


void
InterceptDropManager::setWindow(NPWindow* window)
{
    m_window = window;
    updateBounds();
    derivedSetWindow(window);
}


void
InterceptDropManager::derivedSetWindow(NPWindow*)
{
}


void
InterceptDropManager::updateBounds()
{
    // iterate through all drop targets and update their bounds
    map<string, DropTargetContext>::iterator it;
    
    for (it = m_targets.begin(); it != m_targets.end(); it++) {
        // Ask browser for bounds of element associated with drop target
        NPVariant nameArg;
        STRINGZ_TO_NPVARIANT(it->first.c_str(), nameArg);
        NPVariant result;

        // if for whatever reason we cannot attain drop bounds for an
        // element, we'll reset it's bounds to zero.
        bool gotBounds = false;
        
        if (npu::callFunction(m_instance, m_getXYFunction, &nameArg, 1, &result)) {
            if (NPVARIANT_IS_STRING(result)) {
                NPString s = NPVARIANT_TO_STRING(result);
                string boundsStr;
                boundsStr.append(static_cast<const char*>(s.utf8characters),
                                 s.utf8length);
                
                int r = sscanf(boundsStr.c_str(), "%hd %hd %hd %hd",
                               &(it->second.m_top),
                               &(it->second.m_left), 
                               &(it->second.m_bottom),
                               &(it->second.m_right));

                if (r != 4) {
                    BPLOG_ERROR_STRM( "couldn't attain bounds for " <<
                                      it->first << " (malformed-string)");
                    // TODO: programatically return an error?  This is fatal
                    //       as we're invoking a javascript function that
                    //       we authored (in this source file)
                    return; 
                } else {
                    gotBounds = true;
                    BPLOG_INFO_STRM("bounds for " << it->first <<
                                    "  l/t/r/b " <<
                                    it->second.m_left << ", " << 
                                    it->second.m_top << " / " <<
                                    it->second.m_right << ", " <<
                                    it->second.m_bottom);
                }
            } else {
                BPLOG_ERROR_STRM("couldn't attain bounds for " << it->first <<
                                 " (non string)");
            }
            
            gBrowserFuncs.releasevariantvalue(&result);
        } else {
            BPLOG_ERROR_STRM("couldn't attain bounds for " << it->first);
        }

        if (!gotBounds) {
            it->second.m_top = it->second.m_bottom =
                it->second.m_left = it->second.m_right = 0;
        }
    }
}


// handle mouse entering/exiting the browser window
void
InterceptDropManager::handleMouseEnter(const vector<bp::file::Path>& dragItems)
{
    m_dragItems = dragItems;
    BPLOG_INFO_STRM(m_dragItems.size() << " drag items");
    updateBounds();
}


void
InterceptDropManager::handleMouseExit(bool freeDragItems)
{
    BPLOG_INFO_STRM((freeDragItems ? "free " : "keep ")
                    << m_dragItems.size() << " drag items");
    if (freeDragItems) {
        m_dragItems.clear();
    }
    
    // send hover off to any hovering drag targets and clear all dragItems
    map<string, DropTargetContext>::iterator it;
    for (it = m_targets.begin(); it != m_targets.end(); it++) {
        if (it->second.state() == DropTargetBase::Hovering) {
            m_listener->onHover(it->first, false);
        }
        it->second.leave(freeDragItems);
    }
}


bool
InterceptDropManager::handleMouseDrag(short int x,
                                      short int y)
{
    bool rval = false;
    
    // check for hover
    // iterate through all drop targets and check for hits
    map<string, DropTargetContext>::iterator it;
    
    for (it = m_targets.begin(); it != m_targets.end(); it++) {
        bool hit = it->second.insideRegion(x,y);
        if (hit) {
            if (it->second.state() == DropTargetBase::Idle) {
                it->second.enter(m_dragItems);
            }
            bool canAccept = it->second.canAcceptDrop();
            if (canAccept) rval = true;
            m_listener->onHover(it->first, canAccept);
        } else if (it->second.state() == DropTargetContext::Hovering) {
            it->second.leave(false);
            m_listener->onHover(it->first, false);
        }
    }

    return rval;
}


bool
InterceptDropManager::handleMouseDrop(short int x,
                                      short int y)
{
    bool rv = false;

    // did this drop occur in a target?
    // iterate through all drop targets and check for hits
    map<string, DropTargetContext>::iterator it;
    
    for (it = m_targets.begin(); it != m_targets.end(); it++) {
        if (it->second.insideRegion(x,y) && it->second.canAcceptDrop()) {
            bp::Object* dropList = it->second.dropItems();
            m_listener->onDrop(it->first, dropList);
            delete dropList;
            rv = true;
        }
    }

    return rv;
}

