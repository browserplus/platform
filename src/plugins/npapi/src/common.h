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
 *  common.h
 *  BrowserPlus
 *
 *  Created by Gordon Durand on 5/7/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "npapi/npapi.h"
#include "npapi/npupp.h"
#include "npapi/npruntime.h"

// Define PLUGIN_TRACE to 1 to have the wrapper functions emit
// DebugStr messages whenever they are called.
//
//#define PLUGIN_TRACE 1	
#if PLUGIN_TRACE
#define PLUGINDEBUGSTR(msg)		::DebugStr(msg)
#else
#define PLUGINDEBUGSTR(msg)    ((void) 0)
#endif

// Function table for procs in browser called by plugin
extern "C" {
    extern NPNetscapeFuncs gBrowserFuncs;
};

#endif
