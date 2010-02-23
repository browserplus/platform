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
 *  bpexitcodes.h
 *  Declares exit codes from bp processes.
 *
 *  Created by David Grigsby on 2/22/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef BPEXITCODES_H_
#define BPEXITCODES_H_


namespace bp {
namespace exit {

const int kOk                           = 0;
const int kNoPermissionsManager         = -1;
const int kCantLoadConfigFile           = -2;
const int kCantProcessCommandLine       = -3;
const int kCantSetupIpcServer           = -4;
const int kCantSetupCoreletInstaller    = -5;
const int kCantSetupCoreletUpdater      = -6;
const int kCantSetupPlatformUpdater     = -7;
const int kCantSetupPermissionsUpdater  = -8;
const int kDuplicateProcess             = -9;
const int kCantRunServiceProcess        = -10;
const int kCantGetUpToDatePerms         = -11;
const int kKillswitch                   = -666;

}
}

#endif
