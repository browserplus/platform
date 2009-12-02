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
 *  ServiceLogging.h
 *
 *  Support routines to permit BPUtils logging by services that link
 *  with BPUtils.
 *  
 *  Created by David Grigsby on Mon August 03 2008.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 */

#ifndef SERVICELOGGING_H_
#define SERVICELOGGING_H_

namespace bp {
namespace service {

typedef void (*LogFuncPtr)(unsigned int level, const char * fmt, ...);

// Setup BPUtils logging in the service such that BPUtils logging
// statements are fulfilled by logging through the Service API.
// Whether and how these statements are actually logged is controlled
// by the browserplus.config configuration file.
void setupBPUtilsLogging( LogFuncPtr pfnLog );


}
}

#endif
