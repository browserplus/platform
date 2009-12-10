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
 *  DnDPlugletFactoryAx.h
 *  BrowserPlusPlugin
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __DNDPLUGLETFACTORYNAX_H__
#define __DNDPLUGLETFACTORYAX_H__

#include "DnDPlugletFactoryAx.h"

static std::list<Pluglet*> 
DnDPlugletFactoryAx::createPluglets( BPPlugin* pPlugin
                                     IDropManager* pDropMgr )
{
    std::list<Pluglet*> rval;
    std::list<bp::service::Description>::const_iterator it;
    for (it = m_descriptions.begin(); it != m_description.end(); ++it) {
        rval.push_back(DnDPluglet( pPlugin, pDropMgr, it->versionString()));
    }
    return rval;
}

