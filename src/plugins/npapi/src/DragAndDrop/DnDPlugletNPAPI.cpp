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

// Disable "nameless struct/union" warnings that come from winnt.h that is 
// evidently included through some npapi header.
#ifdef WIN32
#pragma warning (disable: 4201)
#endif

#include "PluginCommonLib/IDropManager.h"
#include "DnDPlugletNPAPI.h"
#include "DropManagerFactory.h"
#include "InterceptDropManager.h"


DnDPlugletNPAPI::DnDPlugletNPAPI(NPP instance, 
                                 BPPlugin* plugin)
    // must explicitly invoke Pluglet ctor
    : Pluglet(plugin), DnDPluglet(plugin, NULL), m_npp(instance)
{
}


DnDPlugletNPAPI::~DnDPlugletNPAPI()
{
	delete m_dropMgr;
}


void
DnDPlugletNPAPI::setWindow(NPWindow * window)
{
    if (m_dropMgr == NULL) {
        m_dropMgr = DropManagerFactory::create(m_npp, window, this);
    } else {
        InterceptDropManager* dman = dynamic_cast<InterceptDropManager*>(m_dropMgr);
        if (dman) {
            dman->setWindow(window);
        }
    }
}
