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
 * BPProtoUtil.h -- custom little tools used by the service library.
 */
#include "BPProtoUtil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bperrorutil.h"

#include <string.h>

void freeBPElement(BPElement * elem)
{
    if (elem == NULL) return;

    switch(elem->type) {
        case BPTMap: 
            for (unsigned int i = 0; i < elem->value.mapVal.size; i++)
            {
                freeBPElement(elem->value.mapVal.elements[i].value);
            }
            free(elem->value.mapVal.elements);
            break;
        case BPTList: {
            for (unsigned int i = 0; i < elem->value.listVal.size; i++)
            {
                freeBPElement(elem->value.listVal.elements[i]);
            }
            free(elem->value.listVal.elements);
        }
        default: 
            break;
    }

    free(elem);

    return;
}

static char *
getStringValue(const bp::Object * obj)
{
    const char * str = NULL;
    if (obj != NULL) {
        const bp::String * s = dynamic_cast<const bp::String *>(obj);
        if (s != NULL) {
            str = s->value();
        }
    }
    return (BPString) str;
}

static bool
getBoolValue(const bp::Object * obj)
{
    bool bv = false;
    if (obj != NULL) {
        const bp::Bool * s = dynamic_cast<const bp::Bool *>(obj);
        if (s != NULL) {
            bv = s->value();
        }
    }
    return bv;
}

static long long int
getIntegerValue(const bp::Object * obj)
{
    long long int iv = 0;
    if (obj != NULL) {
        const bp::Integer * s = dynamic_cast<const bp::Integer *>(obj);
        if (s != NULL) {
            iv = s->value();
        }
    }
    return iv;
}

static BPType
getTypeValue(const bp::Object * obj)
{
    BPType t = BPTNull;
    const char * type = getStringValue(obj);

    if (type != NULL) {
        if (!strcmp(type, "null")) t = BPTNull;
        else if (!strcmp(type, "boolean")) t = BPTBoolean;
        else if (!strcmp(type, "integer")) t = BPTInteger;
        else if (!strcmp(type, "double")) t = BPTDouble;
        else if (!strcmp(type, "string")) t = BPTString;
        else if (!strcmp(type, "map")) t = BPTMap;
        else if (!strcmp(type, "list")) t = BPTList;
        else if (!strcmp(type, "callback")) t = BPTCallBack;
        else if (!strcmp(type, "path")) t = BPTPath;
        else if (!strcmp(type, "any")) t = BPTAny;
    }
    return t;
}

BPServiceDefinition * objectToDefinition(const bp::Object * obj)
{
    BPServiceDefinition * def = NULL;
    
    // ensure that the object is well formed
    if (obj == NULL || obj->type() != BPTMap)
    {
        return def;
    }

    // grab a pointer to the to level object
    const bp::Map * m = dynamic_cast<const bp::Map *>(obj);
    BPASSERT(m != NULL);

    // we seem good.  allocate the top level service definition structure.
    def = (BPServiceDefinition *) calloc(1, sizeof(BPServiceDefinition));
    
    // now pull out all the information we can
    def->serviceName = getStringValue(m->get("name"));
    def->docString = getStringValue(m->get("documentation"));

    // handle version
    const bp::Map * verMap = dynamic_cast<const bp::Map *>(m->get("version"));
    if (verMap != NULL) {
        def->majorVersion =
            (unsigned int) getIntegerValue(verMap->get("major"));
        def->minorVersion =
            (unsigned int) getIntegerValue(verMap->get("minor"));
        def->microVersion =
            (unsigned int) getIntegerValue(verMap->get("micro"));
    }

    // now let's deal with functions
    if (m->has("functions", BPTList)) {
        const bp::List * funcs =
            (dynamic_cast<const bp::List *> (m->get("functions")));
        def->numFunctions = funcs->size();
        def->functions = (BPFunctionDefinition *)
            calloc(funcs->size(), sizeof(BPFunctionDefinition));
        
        // now iterate over available functions
        for (unsigned int i = 0; i < funcs->size(); i++)
        {
            const bp::Map * func =
                dynamic_cast<const bp::Map *>(funcs->value(i));
            if (func != NULL)
            {
                def->functions[i].functionName = 
                    getStringValue(func->get("name"));
                def->functions[i].docString = 
                    getStringValue(func->get("documentation"));

                const bp::List * args =
                    dynamic_cast<const bp::List *>(func->get("parameters"));
                
                if (args != NULL && args->size() > 0) {
                    def->functions[i].numArguments = args->size();
                    def->functions[i].arguments = (BPArgumentDefinition *)
                        calloc(args->size(), sizeof(BPArgumentDefinition));
                    
                    // now iterate over arguments
                    for (unsigned int j = 0; j < args->size(); j++)
                    {
                        BPArgumentDefinition * argdef = 
                            def->functions[i].arguments + j;

                        const bp::Map * arg =
                            dynamic_cast<const bp::Map *>(args->value(j));

                        if (!arg) {
                            BPLOG_WARN_STRM("bad argument def: "
                                            << args->value(j)->toJsonString());
                            continue;
                        }
                        argdef->name = getStringValue(arg->get("name"));
                        argdef->docString =
                            getStringValue(arg->get("documentation"));

                        argdef->required =
                            getBoolValue(arg->get("required"));
                        argdef->type = getTypeValue(arg->get("type"));
                    }
                }
            }
        }
    }

    return def;
}

void freeDefinition(BPServiceDefinition * definition)
{
    if (definition != NULL) {
        // TODO: implement me!  this is memory leak!
        free(definition);
    }
}

bool startupDaemon(bp::process::spawnStatus& spawnStatus) 
{
    BPLOG_INFO("starting daemon --");

    bp::file::Path binaryPath = bp::paths::getDaemonPath();
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

