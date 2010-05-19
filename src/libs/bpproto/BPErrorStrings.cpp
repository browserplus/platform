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

#include "api/BPProtocolInterface.h"
#include <stdlib.h>

static struct {
    BPErrorCode ec;
    const char * text;
} ecLookupTable[] = {
    { BP_EC_OK, "BP_EC_OK" },
    { BP_EC_GENERIC_ERROR, "BP_EC_GENERIC_ERROR" },
    { BP_EC_CONNECTION_FAILURE, "BP_EC_CONNECTION_FAILURE" },
    { BP_EC_INVALID_LOCATION, "BP_EC_INVALID_LOCATION" },
    { BP_EC_INVALID_PARAMETER, "BP_EC_INVALID_PARAMETER" },
    { BP_EC_INVALID_STATE, "BP_EC_INVALID_STATE" },
    { BP_EC_PROTOCOL_ERROR, "BP_EC_PROTOCOL_ERROR" },
    { BP_EC_NOT_IMPLEMENTED, "BP_EC_NOT_IMPLEMENTED" },
    { BP_EC_NO_SUCH_SERVICE, "BP_EC_NO_SUCH_SERVICE" },
    { BP_EC_NO_SUCH_FUNCTION, "BP_EC_NO_SUCH_FUNCTION" },
    { BP_EC_SERVICE_EXEC_ERROR, "BP_EC_SERVICE_EXEC_ERROR" },
    { BP_EC_INVALID_VERSION_STRING, "BP_EC_INVALID_VERSION_STRING" },
    { BP_EC_EXTENDED_ERROR, "BP_EC_EXTENDED_ERROR" },
    { BP_EC_SPAWN_FAILED, "BP_EC_SPAWN_FAILED" },
    { BP_EC_CONNECT_TIMEOUT, "BP_EC_CONNECT_TIMEOUT" },
    { BP_EC_PERMISSIONS_ERROR, "BP_EC_PERMISSIONS_ERROR" },
    { BP_EC_PLATFORM_BLACKLISTED, "BP_EC_PLATFORM_BLACKLISTED" },
    { BP_EC_UNAPPROVED_DOMAIN, "BP_EC_UNAPPROVED_DOMAIN" },
    { BP_EC_REQUIRE_ERROR, "BP_EC_REQUIRE_ERROR" },
    { BP_EC_PLATFORM_DISABLED, "BP_EC_PLATFORM_DISABLED" },    
    { BP_EC_PEER_ENDED_CONNECTION, "BP_EC_PEER_ENDED_CONNECTION" },
    { BP_EC_SWITCH_VERSION, "BP_EC_SWITCH_VERSION" },
};

const char *
BPErrorCodeToString(BPErrorCode ec)
{
    unsigned int i;
    const char * str = "invalid error code";
    for (i=0; i < (sizeof(ecLookupTable)/sizeof(ecLookupTable[0])); i++)
    {
        if (ecLookupTable[i].ec == ec) {
            str = ecLookupTable[i].text;
            break;
        }
    }
    return str;
}
