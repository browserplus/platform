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
 * The interface for the Drag and Drop Pluglet
 */

#ifndef __DNDPLUGLET_H__
#define __DNDPLUGLET_H__

#include "IDropListener.h"
#include "IDropManager.h"
#include "PluginCommonLib/Pluglet.h"

class DnDPluglet : public virtual Pluglet,
                   public virtual IDropListener
{
  public:
    DnDPluglet(BPPlugin* plugin,
               IDropManager* dropMgr,
               const bp::service::Description& desc);

    virtual ~DnDPluglet();

    // Pluglet methods
    virtual void execute(unsigned int tid,
                         const char* function,
                         const bp::Object* arguments,
                         bool syncInvocation,
                         plugletExecutionSuccessCB successCB,
                         plugletExecutionFailureCB failureCB,
                         plugletInvokeCallbackCB callbackCB,
                         void* callbackArgument);

  protected:
    struct DnDCallbackInfo
    {
        DnDCallbackInfo() 
            : hoverCB(0), dropCB(0), tid(0),
            callback(NULL), callbackArg(NULL) 
        {
        }

        DnDCallbackInfo(const DnDCallbackInfo& other)
            : hoverCB(other.hoverCB), dropCB(other.dropCB),
            tid(other.tid), callback(other.callback),
            callbackArg(other.callbackArg)
        {
        }

        BPCallBack hoverCB;
        BPCallBack dropCB;
        unsigned int tid;
        plugletInvokeCallbackCB callback;
        void* callbackArg;
    };

    // IDropListener methods
    virtual void onHover(const std::string& id,
                         bool hover);
    virtual void onDrop(const std::string& id,
                        const bp::Object* items);

    IDropManager* m_dropMgr;
    typedef std::map<std::string, DnDCallbackInfo> tTargetMap; 
    tTargetMap m_targets;        
};

#endif
