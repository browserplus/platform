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

#include "WSProtocol.h"

#include <sstream>

const char * WSProtocol::API_PREFIX = "api";
const char * WSProtocol::WS_VERSION = "v4";
const char * WSProtocol::PERMISSIONS_PATH = "permissions";
const char * WSProtocol::AVAILABLE_SERVICES_PATH = "services";
const char * WSProtocol::SERVICE_METADATA_PATH = "service/metadata";
const char * WSProtocol::SERVICE_SYNOPSIS_PATH = "service/synopsis";
const char * WSProtocol::SERVICE_DOWNLOAD_PATH ="service/package";
const char * WSProtocol::USAGE_PATH = "usage";
const char * WSProtocol::LATEST_PLATFORM_VERSION_PATH = "platform/latest/version";
const char * WSProtocol::LATEST_PLATFORM_UPDATE_PATH = "platform/latest/update";

std::string WSProtocol::buildURL(std::string baseURL, const char * path) 
{
    std::stringstream ss;
    ss << baseURL << "/" << API_PREFIX << "/" << WS_VERSION << "/" << path;
    return ss.str();
}

bp::http::RequestPtr WSProtocol::buildRequest(const std::string & url) 
{
    using namespace bp::http;
    
    Headers h;
    h.add("Accept", "application/json");
    bp::http::RequestPtr rval(new bp::http::Request(
        Method(Method::HTTP_GET),
        bp::url::Url(url),
        Version(Version::HTTP1_1),
        h, 
        Body()));
    return rval;
}

