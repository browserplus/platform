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
 *  LogPluglet.h
 *
 *  Pluglet that allows js to access plugin log facility.
 *  
 *  Created by David Grigsby on 12/12/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __LOGPLUGLET_H__
#define __LOGPLUGLET_H__

#include "PluginCommonLib/Pluglet.h"
#include "PluginCommonLib/BPPlugin.h"

class LogPluglet : public Pluglet
{
public:
    LogPluglet( BPPlugin* plugin );
    ~LogPluglet();

    void execute( unsigned int tid,
                  const char* function,
                  const bp::Object* arguments,
                  bool bSyncInvocation,
                  plugletExecutionSuccessCB successCB,
                  plugletExecutionFailureCB failureCB,
                  plugletInvokeCallbackCB callbackCB,
                  void* callbackArgument);

    const bp::service::Description * describe();
};

#endif
