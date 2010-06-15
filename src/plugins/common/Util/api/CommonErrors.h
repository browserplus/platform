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

#ifndef __COMMONERRORS_H__
#define __COMMONERRORS_H__

namespace pluginerrors {

// an unknown/unspecified condition has been hit
extern const char * UnknownError;

// attempt to invoke a method on a service that is not loaded
extern const char * ServiceNotLoaded;

// generic internal error, the "impossible" has happened
extern const char * InternalError;

// invalid parameters
extern const char * InvalidParameters;

// A plugin function is called before "Initialize"
extern const char * NotInitialized;

// The Platform isn't installed or is partially uninstalled
extern const char * NotInstalled;

// Error connecting to daemon
extern const char * ConnectionError;

// A different kind of internal error as far as the js programmer is
// concerned, but means that BPProtocol violated its contract, and
// conveys that information up.
extern const char * ProtocolError;

// unable to update permissions
extern const char * PermissionsError;

// platform is blacklisted
extern const char * PlatformBlacklisted;

// platform is disabled
extern const char * PlatformDisabled;

// not an approved domain
extern const char * UnapprovedDomain;

// A newer platform version is available and javascript should use that
// one instead
extern const char * SwitchVersion;
};

#endif
    
