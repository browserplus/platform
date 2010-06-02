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
 *  IDropListener.h
 *
 *  Drop Listener interface.
 *
 *  Created by David Grigsby on 11/11/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __IDROPLISTENER_H__
#define __IDROPLISTENER_H__

#include <string>
#include <vector>

namespace bp {
    class Object;
}

class IDropListener
{
public:
    virtual ~IDropListener() {};
    virtual void onHover(const std::string& id,
                         bool hover) = 0;
    
    // Items is either a list or a map.  See documentation of
    // AddDropTarget() in DnDPluglet.cpp for description.
    //
    virtual void onDrop(const std::string& id,
                        const bp::Object* items) = 0;
};


#endif // __IDROPLISTENER_H__
