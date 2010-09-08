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
 * WSProtocol - an abstraction around interaction with the BP WebServices
 *              API which encapsulates all protocol strings, and some
 *              utility routines.  
 */

#ifndef __WSPROTOCOL_H__
#define __WSPROTOCOL_H__

#include <string>
#include "bphttp/HttpRequest.h"

namespace WSProtocol
{
    // all constant strings which are part of the web services protocol
    extern const char * API_PREFIX;
    extern const char * WS_VERSION;
    extern const char * PERMISSIONS_PATH;
    extern const char * AVAILABLE_SERVICES_PATH;
    extern const char * SERVICE_METADATA_PATH;
    extern const char * SERVICE_DOWNLOAD_PATH;
    extern const char * USAGE_PATH;
    extern const char * SERVICE_SYNOPSIS_PATH;
    extern const char * LATEST_PLATFORM_VERSION_PATH;
    extern const char * LATEST_PLATFORM_UPDATE_PATH;
    
    std::string buildURL(std::string baseURL, const char * path);

    bp::http::RequestPtr buildRequest(const std::string & url);
};

#endif
