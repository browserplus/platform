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

/**
 * The interface for the FileSave Pluglet
 */

#ifndef __FILESAVEPLUGLET_H__
#define __FILESAVEPLUGLET_H__

#include "PluginCommonLib/Pluglet.h"
#include "PluginCommonLib/BPPlugin.h"
#include <list>

class FileSavePluglet : public Pluglet
{
  public:
    FileSavePluglet(BPPlugin* plugin,
                    const bp::service::Description& desc);
    virtual ~FileSavePluglet();

    /** defined per plugin per platform */
    void execute(unsigned int tid,
                 const char* function,
                 const bp::Object* arguments,
                 bool syncInvocation,
                 plugletExecutionSuccessCB successCB,
                 plugletExecutionFailureCB failureCB,
                 plugletInvokeCallbackCB   callbackCB,
                 void* callbackArgument);
};

#endif
