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

/*
 *  IDropManager.h
 *
 *  Drop Manager interface.
 *
 *  Created by David Grigsby on 11/11/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __IDROPMANAGER_H__
#define __IDROPMANAGER_H__

#include <set>

// Forward Decl
class IDropListener;


class IDropManager
{
public:
    virtual ~IDropManager() {};
    virtual bool addTarget(const std::string& sElement,
                           const std::set<std::string>& mimeTypes,
                           bool includeGestureInfo,
                           unsigned int limit) = 0;
    virtual bool removeTarget(const std::string& sElement) = 0;
    virtual bool enableTarget(const std::string& sElement,
                              bool bEnable) = 0;
    virtual bool registerDropListener(IDropListener* pListener) = 0;
};

#endif // __IDROPMANAGER_H__
