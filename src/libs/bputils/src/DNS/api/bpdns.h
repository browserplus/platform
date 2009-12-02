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
 *  bpdns.h
 *
 *  Created by Lloyd Hilaiel on 1/7/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPDNS_H__
#define __BPDNS_H__

#include <vector>
#include <string>

namespace bp {
    namespace dns {

        /**
         * given a string, attempt to resolve it to hostname(s).
         * the return vector will be empty if resolution failed.
         * if non empty, the first element is the "official" name
         * and any additional elements are aliases.
         *
         * CAVEATS: supports ipv4 and ipv6 on osx, only ipv4 on doze.
         */
        std::vector<std::string> ipToNames(const std::string & ip);

    } // dns
} // bp

#endif
