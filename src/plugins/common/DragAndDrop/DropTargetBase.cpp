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
 *  DropTargetBase.cpp
 *
 *  Common drop target functionality is handled by this class.
 *  Created by Gordon Durand on Thu June 26 2008
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */

#include "DropTargetBase.h"
#include "BPHandleMapper.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "PluginCommonLib/bppluginutil.h"

using namespace std;
using namespace bp::file;

DropTargetBase::DropTargetBase()
    : m_name(), m_mimetypes(),
      m_includeGestureInfo(false),
      m_dragItems(), m_previousDragItems(),
      m_state(DropTargetBase::Idle),
      m_dropState(DropTargetBase::Unknown),
      m_enabled(true),
// win32 file systems tend to be much slower than osx, on win32 we'll
// have a limit of 1k files dropped (YIB-2077636)
#ifdef WIN32
      m_limit(1000)
#else
      m_limit(10000)
#endif
{
}
  
  
DropTargetBase::DropTargetBase(const string& name,
                               const set<string>& mimeTypes,
                               bool includeGestureInfo,
                               unsigned int limit)
    : m_name(name),
      m_mimetypes(mimeTypes), 
      m_includeGestureInfo(includeGestureInfo),
      m_dragItems(), 
      m_previousDragItems(),
      m_state(DropTargetBase::Idle),
      m_dropState(DropTargetBase::Unknown),
      m_enabled(true),
      m_limit(limit)
{
}
    

DropTargetBase::DropTargetBase(const DropTargetBase& dtb)
    : m_name(dtb.m_name),
      m_mimetypes(dtb.m_mimetypes), 
      m_includeGestureInfo(dtb.m_includeGestureInfo), 
      m_dragItems(),
      m_previousDragItems(),
      m_state(dtb.m_state),
      m_dropState(Unknown),
      m_enabled(dtb.m_enabled),
      m_limit(dtb.m_limit)
{
}


DropTargetBase::~DropTargetBase()
{
}


void 
DropTargetBase::enable(bool val)
{
    BPLOG_INFO_STRM((val ? "enable " : "disable ") << m_name); 
    m_enabled = val;
}


bool 
DropTargetBase::isEnabled()
{
    return m_enabled;
}


DropTargetBase::State
DropTargetBase::state()
{
    return m_state;
}
    

void 
DropTargetBase::enter(const vector<Path>& dragItems)
{
    BPLOG_INFO_STRM("enter target " << m_name 
                    << ", dropState = " << m_dropState);
    
    // Have drag items changed?  if so, reset m_dropState
    // Allows optimization for subclasses which don't know
    // when a drag has entered/exited browser (IE) and thus
    // don't know when to call leave(true)
    if (dragItems != m_previousDragItems) {
        BPLOG_INFO_STRM("dragItems changed");
        m_dragItems = dragItems;
        m_previousDragItems = dragItems;
        m_dropState = Unknown;
    }
    m_state = Hovering;
}


void 
DropTargetBase::leave(bool freeDragItems)
{
    BPLOG_INFO_STRM("leave target " << m_name 
                    << ", free = " << freeDragItems);
    if (freeDragItems) {
        m_dragItems.clear();
        m_previousDragItems.clear();
        m_dropState = Unknown;
    }
    m_state = Idle;
}
    

bool 
DropTargetBase::canAcceptDrop()
{
    if (!m_enabled) {
        return false;
    }

    if (m_dropState == Unknown) {
        if (m_mimetypes.empty()) {
            m_dropState = CanAccept;
        } else {
            m_dropState = CannotAccept;
            for (unsigned int i = 0; i < m_dragItems.size(); ++i) {
                Path path = m_dragItems[i];
                if (boost::filesystem::is_directory(path)) {
                    if (directoryContainsMimeType(path)) {
                        m_dropState = CanAccept;
                        break;
                    }
                } else {
                    if (isLink(path)) {
                        if (!resolveLink(path, path)) {
                            continue;
                        }
                    }
                    if (isMimeType(path, m_mimetypes)) {
                        m_dropState = CanAccept;
                        break;
                    }
                }
            }
        }
    }
    BPLOG_INFO_STRM(m_name << " can " 
                    << ((m_dropState == CanAccept) ? "" : "not") 
                    << " accept drop");
    return m_dropState == CanAccept;
}


bp::Object*
DropTargetBase::dropItems()
{
    // resolve links
    vector<Path>::iterator it = m_dragItems.begin();
    while (it != m_dragItems.end()) {
        Path path(*it);
        if (isLink(path)) {
            if (!resolveLink(path, path)) {
                it = m_dragItems.erase(it);
                continue;
            } else {
                *it = path;
            }
        }
        ++it;
    }
        
    unsigned int flags = bp::pluginutil::kRecurse;
    if (m_includeGestureInfo) flags |= bp::pluginutil::kIncludeGestureInfo;
    return bp::pluginutil::applyFilters(m_dragItems, m_mimetypes,
                                        flags, m_limit);
}

std::string
DropTargetBase::name()
{
    return m_name;
}


bool 
DropTargetBase::directoryContainsMimeType(const Path& path)
{    
    class MyVisitor : public bp::file::IVisitor {
    public:
        MyVisitor(const set<string>& mimeTypes,
                  unsigned int limit)
            : m_mimeTypes(mimeTypes), m_limit(limit),
              m_numChecked(0), m_found(false) {
        }
        virtual ~MyVisitor() {}
        virtual tResult visitNode(const bp::file::Path& p,
                                  const bp::file::Path&) {
            if (m_numChecked >= m_limit) {
                return eStop;
            }
            m_numChecked++;
            Path resolved = p;
            if (isLink(p)) {
                if (!resolveLink(p, resolved)) {
                    return eOk;
                }
            }
            if (boost::filesystem::is_directory(resolved)) {
                if (m_mimeTypes.count(kFolderMimeType)) {
                    m_found = true;
                    return eStop;
                }
            } else if (boost::filesystem::is_regular_file(resolved)) {
                if (isMimeType(resolved, m_mimeTypes)) {
                    m_found = true;
                    return eStop;
                }
            }
            return eOk;
        }
        set<string> m_mimeTypes;
        size_t m_limit;
        size_t m_numChecked;
        bool m_found;
    };
    
    MyVisitor v(m_mimetypes, m_limit);
    (void) recursiveVisit(path, v, true);
    return v.m_found;
}


