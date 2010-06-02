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
 *  DropTargetBase.h
 *
 *  Common drop target functionality is handled by this class.
 *  Created by Gordon Durand on Thu June 26 2008
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
 
#ifndef __DROPTARGETBASE_H__
#define __DROPTARGETBASE_H__

#include <string>
#include <vector>
#include <set>

#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpfile.h"


class DropTargetBase
{
  public:
    
    typedef enum {
        Idle,
        Hovering
    } State;
    
    DropTargetBase();
    DropTargetBase(const std::string& name,
                   const std::set<std::string>& mimeTypes,
                   bool includeGestureInfo,
                   unsigned int limit);
    DropTargetBase(const std::string& name,
                   const std::string& version);
    DropTargetBase(const DropTargetBase& dtc);
    virtual ~DropTargetBase();

    virtual std::string name();
    virtual void enable(bool val);
    virtual bool isEnabled();
    virtual State state();
    
    virtual void enter(const std::vector<bp::file::Path>& dragItems);
    virtual void leave(bool freeDragItems);
    
    virtual bool canAcceptDrop();
    virtual bp::Object* dropItems(); // caller assumes ownership of return value

  protected:
    virtual bool directoryContainsMimeType(const bp::file::Path& path);

    typedef enum {
        Unknown,
        CannotAccept,
        CanAccept
    } DropState;
    std::string m_name;
    std::set<std::string> m_mimetypes;
    bool m_includeGestureInfo;
    std::vector<bp::file::Path> m_dragItems;
    std::vector<bp::file::Path> m_previousDragItems;
    State m_state;
    DropState m_dropState;
    bool m_enabled;
    unsigned int m_limit;
    std::string m_version;
};

#endif
