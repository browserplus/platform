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
 * OS.h - tools to determine what system we're running on.
 *
 * Original author: ehill 2007/05/21
 * Adapted for BrowserPlus: lloydh 2007/07/23
 */

#ifndef __OS_H__
#define __OS_H__

#include <string>

namespace bp
{
    namespace os 
    {
        typedef enum {
            Unknown,
            Windows,
            OSX,
            Linux
        } PlatformType;

        PlatformType Platform();
        
        std::string PlatformAsString();
        
        std::string PlatformVersion();
        
        std::string ServicePack();
        
        std::string CurrentUser();

        bool Is64Bit();
        
        bool IsDeprecated();
    };
};

#endif
