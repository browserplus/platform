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

/**
 * UsageReporting.h
 *
 * Responsible for reporting BP client usage data to the BP backend.
 *
 * Created by David Grigsby on 01/07/2008.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __USAGEREPORTING_H__
#define __USAGEREPORTING_H__

#include <string>

namespace bp {
namespace usage {

void reportPageUsage( const std::string& sUrl, const std::string& sUserAgent );

} // usage
} // bp


#endif // __USAGEREPORTER_H__

