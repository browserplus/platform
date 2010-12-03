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
 * BPProtoUtil.cpp -- custom little tools used by the service library.
 */
#include "BPProtoUtil.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/BPLog.h"
#include "platform_utils/ProductPaths.h"

//#include <string.h>

bool startupDaemon(bp::process::spawnStatus& spawnStatus) 
{
    BPLOG_INFO("starting daemon --");

    boost::filesystem::path binaryPath = bp::paths::getDaemonPath();
    BPLOG_INFO_STRM("path to binary: " << binaryPath);

    // execute the process
    if (!bp::process::spawn(binaryPath, std::vector<std::string>(), 
                            &spawnStatus)) {
        return false;
    }

    BPLOG_INFO_STRM("proc spawned: " << spawnStatus.pid);
    
    return true;
}

void
mapResponseToErrorCode(const bp::Object * obj,
                       BPErrorCode & ec,
                       std::string & errorString,
                       std::string & verboseError)
{
    ec = BP_EC_PROTOCOL_ERROR;
    verboseError.clear();
    errorString.clear();

    if (obj == NULL) return;

    // for errors we expect an error and optional verboseError
    // "extended" errors are when we either don't recognize the
    // error code, or when a verboseError is present
    
    // TODO: perhaps we should rework this and have errors always returned
    //  as structures with:
    //     1. numeric code
    //     2. short error string,
    //     3. verbose error
    if (obj->has("error", BPTString)) {
        errorString = std::string(*(obj->get("error")));
    }

    if (obj->has("verboseError", BPTString)) {
        verboseError = std::string(*(obj->get("verboseError")));
    }

    // error is required
    if (errorString.empty()) {
        ec = BP_EC_PROTOCOL_ERROR;
    }
    // presence of verboseError indicates "extended error"
    else if (!verboseError.empty()) 
    {
        ec = BP_EC_EXTENDED_ERROR;
    }
    // otherwise it's something we can map to an error code
    else
    {
        // now determine the results of the command
        if (!errorString.compare("BP.noSuchService")) {
            ec = BP_EC_NO_SUCH_SERVICE;
        }
        else if (!errorString.compare("BP.noSuchFunction"))
        {
            ec = BP_EC_NO_SUCH_FUNCTION;
        }
        else if (!errorString.compare("BP.serviceExecError"))
        {
            ec = BP_EC_SERVICE_EXEC_ERROR;
        }
        else if (!errorString.compare("BP.extendedError"))
        {
            ec = BP_EC_EXTENDED_ERROR;
        }
        else if (!errorString.compare("BP.permissionsError"))
        {
            ec = BP_EC_PERMISSIONS_ERROR;
        }
        else if (!errorString.compare("BP.platformBlacklisted"))
        {
            ec = BP_EC_PLATFORM_BLACKLISTED;
        }
        else if (!errorString.compare("BP.unapprovedDomain"))
        {
            ec = BP_EC_UNAPPROVED_DOMAIN;
        }
        else if (!errorString.compare("BP.requireError"))
        {
            ec = BP_EC_REQUIRE_ERROR;
        }
        else if (!errorString.compare("BP.genericError"))
        {
            ec = BP_EC_GENERIC_ERROR;
        }
        else 
        {
            // unknown error code, return it as a string
            ec = BP_EC_EXTENDED_ERROR;
        }
    }
}

void logExtendedErrorInfo( const std::string& sContext,
                           const bp::Map& bpMap )
{
    std::string sLog = sContext;

    const bp::Object* pObj = bpMap.value( "error" );
    if (pObj && pObj->type()==BPTString)
    {
        sLog.append( ": " );
        sLog.append( dynamic_cast<const bp::String*>(pObj)->value() );
    }

    pObj = bpMap.value( "verboseError" );
    if (pObj && pObj->type()==BPTString)
    {
        sLog.append( ": " );
        sLog.append( dynamic_cast<const bp::String*>(pObj)->value() );
    }
    
    BPLOG_ERROR( sLog );
}

void
extractExtendedError(const bp::Object * obj,
                     std::string &e, std::string &ve)
{
    e.clear();
    ve.clear();

    if (obj) {
        if (obj->has("error", BPTString)) {
            e = std::string(*(obj->get("error")));
        }
        if (obj->has("verboseError", BPTString)) {
            ve = std::string(*(obj->get("verboseError")));
        }
    }

    if (e.empty()) e = "bp.protocolError";
}

