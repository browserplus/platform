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

/**
 * NPAPI Drag and Drop Pluglet
 */

#ifndef __DNDPLUGLETNPAPI_H__
#define __DNDPLUGLETNPAPI_H__

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include "PluginCommonLib/DnDPluglet.h"
#include "WindowedPluglet.h"

class DnDPlugletNPAPI : public virtual DnDPluglet,
                        public virtual WindowedPluglet
{
  public:
    DnDPlugletNPAPI(NPP instance, 
                    BPPlugin* plugin,
                    const bp::service::Description& desc);
    virtual ~DnDPlugletNPAPI();

    // inherited from WindowedPluglet
    virtual void setWindow(NPWindow*);

  protected: 
    NPP m_npp;
};

#endif
