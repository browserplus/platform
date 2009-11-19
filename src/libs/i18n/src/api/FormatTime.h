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
 *  FormatTime.h
 *
 *  Created by David Grigsby on 2/19/09.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _FORMATTIME_H_
#define _FORMATTIME_H_

#include <string>
#include "BPUtils/bptime.h"

namespace bp {
namespace i18n {

// Returns a locale-specific string describing then w.r.t present.
std::string formatRelativeTime( const BPTime& tmThen, 
                                const std::string& sLocale );
                                // throw IcuException

} // i18n
} // bp

#endif

