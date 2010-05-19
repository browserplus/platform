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
 * Code which implements the plugin RequireServices function
 */
#include "BPSession.h"
#include "bppluginutil.h"
#include "BPUtils/bpdefutil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "CommonErrors.h"

#ifdef WIN32
// silence "assignment within conditional expression" warning
#pragma warning(disable : 4706)
#endif

/* a utility routine to take a list of full descriptions and
 * trim them down into the correct require return */
static void
fullDescToNameAndVersion(bp::List * fullDescs, bp::List & shortDescs)
{
    using namespace bp;

    for (unsigned int i = 0; i < fullDescs->size(); ++i) {
        const Map * m = dynamic_cast<const Map *>(fullDescs->value(i));
        if (m != NULL && m->has("name", BPTString) &&
            m->has("versionString", BPTString))
        {
            std::string service = std::string(*(m->get("name")));
            std::string version = std::string(*(m->get("versionString")));    
            
            Map * shortDesc = new Map;
            shortDesc->add("service", new String(service));
            shortDesc->add("version", new String(version));            
            shortDesc->add("fullDesc", m->clone());            

            shortDescs.append(shortDesc);
        }
    }
}

// a dynamically allocated structure freed by the callback
struct RequireContext
{
    RequireContext()
        : coreletDescriptions(NULL), ec(BP_EC_OK), callback(NULL),
          transaction(NULL), bp(NULL)
    { }

    ~RequireContext() {
        if (coreletDescriptions) delete coreletDescriptions;
        if (callback) delete callback;
    }
        
    bp::List * coreletDescriptions;
    bp::List foundServices;

    // the result of the require
    BPErrorCode ec;

    // error information when ec == BP_EC_EXTENDED_ERROR
    std::string error;
    std::string verboseError;

    // the callback in JS to invoke when we're done
    plugin::Object * callback;

    BPTransaction * transaction;
    
    // browserplus object
    BPSession * bp;
};


/* static */
void
BPSession::requireReturn(void * cookie)
{
    RequireContext * ctx = (RequireContext *) cookie;   
	BPASSERT(ctx != NULL);

    BPSession * session = ctx->bp;
    BPPlugin& plugin = session->plugin();
    
    plugin::Variant * arg =  plugin.allocVariant();
    plugin::Variant * result = plugin.allocVariant();

    if (ctx->ec != BP_EC_OK) {
        const char * e = NULL, * ve = NULL;
        if (ctx->ec == BP_EC_EXTENDED_ERROR) {
            e = ctx->error.c_str();
            ve = ctx->verboseError.c_str();
        } else {
            e = BPErrorCodeToString(ctx->ec);
        }
        session->generateErrorReturn(e, ve, arg);
    } else if (ctx->coreletDescriptions == NULL &&
               ctx->foundServices.size() == 0) {
        BPLOG_ERROR("Require complete with no available service descriptions");
        session->generateErrorReturn(
            "internalError",
            "Require complete with no available service descriptions", arg);
    } else {
        // now we should cache coreletDescriptions, _before_ calling the
        // callback
        if (ctx->coreletDescriptions) {
            for (unsigned int i = 0; i < ctx->coreletDescriptions->size(); i++)
            {
                bp::service::Description desc;
                if (desc.fromBPObject(ctx->coreletDescriptions->value(i))) {
                    ctx->bp->m_loadedServices.push_back(desc);
                } else {
                    BPLOG_ERROR("Couldn't parse service description return from "
                                "daemon!  Protocol error!");
                }
            }
        } else {
            ctx->coreletDescriptions = new bp::List;
        }
        
        // now generate return values from new corelets combined with what
        // we found earlier.
        for (unsigned int i = 0; i < ctx->foundServices.size(); i++) {
            ctx->coreletDescriptions->append(
                ctx->foundServices.value(i)->clone());
        }
        
        bp::List returnList;
        fullDescToNameAndVersion(ctx->coreletDescriptions, returnList);
        if (!session->generateSuccessReturn(returnList, arg))
        {
            // what can we do if this fails?
            (void) session->generateErrorReturn(NULL, NULL, arg);
        }
    }

    BPASSERT(ctx->callback != NULL);
    plugin.callJsFunction(ctx->callback, &arg, 1, result);
    session->removeTransaction(ctx->transaction);

    delete ctx;
}


void
BPSession::requireServicesCallback(BPErrorCode ec,
                                   void * cookie,
                                   const BPCoreletDefinition ** defs,
                                   unsigned int numDefs,
                                   const char * error,
                                   const char * verboseError)
{
    RequireContext * ctx = (RequireContext *) cookie;    
    BPASSERT(ctx != NULL);

    ctx->ec = ec;
    if (error) ctx->error.append(error);
    if (verboseError) ctx->verboseError.append(verboseError);    
    if (defs) ctx->coreletDescriptions = bp::defutil::defsToJson(defs, numDefs);
    
    requireReturn(ctx);
}

bool
BPSession::requireServices(const plugin::Variant* args,
                           const plugin::Object* callback,
                           plugin::Variant* result)
{
    if (notInitialized(callback, result)) return true;

    // callback argument is required
    BPASSERT(callback != NULL);
    if (callback == NULL) return false;

    using namespace bp;

    // a list that's built up as we discover what's needed
    List neededServices;
    
    // turn the nasty Variant args into something less nasty
    Object * obj = NULL;
    BPTransaction * transaction = new BPTransaction;
    if (!variantToBPObject(args, transaction, obj))
    {
        delete transaction;
        if (obj) delete obj;
        return false;
    }

    RequireContext * ctx = new RequireContext;
    ctx->transaction = transaction;
    addTransaction(ctx->transaction);
    ctx->callback = callback->clone();
    ctx->bp = this;

    // debug output of require arguments
    if (obj != NULL) {
        BPLOG_DEBUG_STRM("require invoked: " << obj->toPlainJsonString(true));
    }
            
    if (obj == NULL || obj->type() != BPTMap || !obj->has("services", BPTList))
    {
        ctx->ec = BP_EC_EXTENDED_ERROR;
        ctx->error.append(pluginerrors::InvalidParameters);
        ctx->verboseError.append("require takes a map with a 'services' "
                                 "key which is a list of required "
                                 "components");
    } else {
        const List * argsList = (const bp::List *) obj->get("services");    
        BPASSERT(argsList != NULL);
        
        // now attempt to extract arguments from the input args
        unsigned int i;
        for (i = 0; i < argsList->size(); ++i) {
            // "service" name is required
            std::string service, version, minversion;

            const Object * m = argsList->value(i);

            if (m == NULL || m->type() != BPTMap ||
                !m->has("service", BPTString))
            {
                ctx->ec = BP_EC_EXTENDED_ERROR;
                ctx->error.append(pluginerrors::InvalidParameters);
                ctx->verboseError.append("require statements need a "
                                         "'service' key");
                break;
            }
            service = std::string(*(m->get("service")));

            // "version" and "minversion" are optional
            if (m->has("version", BPTString)) {
                version = std::string(*(m->get("version")));
            }

            if (m->has("minversion", BPTString)) {
                minversion = std::string(*(m->get("minversion")));
            }

            BPLOG_INFO_STRM("loading service " << service
                            << " v. " << version
                            << ", mv. " << minversion);

            Pluglet * pluglet = NULL;
            bp::service::Description serviceDesc;
        
            // is this service already loaded?
            if (findLoadedService(service, version, minversion, serviceDesc))
            {
                BPLOG_INFO_STRM("service " << service
                                << " (" << version
                                << ") retrieved from cache");
                ctx->foundServices.append(serviceDesc.toBPObject());
            }
            // is there an applicable pluglet?
            else if ((pluglet =
                      m_plugletRegistry->find(service, version, minversion)))
            {
                // if the service is not yet loaded, but there is an applicable
                // pluglet, extract the definition from the pluglet.
                serviceDesc = *(pluglet->describe());
            
                // add to cache
                m_loadedServices.push_back(serviceDesc);

                // append to results (transferring memeory ownership)
                ctx->foundServices.append(serviceDesc.toBPObject());
            }
            // oops, not found
            else
            {
                Map * m = new Map;
                m->add("service", new String(service.c_str()));
                if (!version.empty()) {
                    m->add("version", new String(version.c_str()));
                }
                if (!minversion.empty()) {
                    m->add("minversion", new String(minversion.c_str()));
                }
                neededServices.append(m);
            }
        }
    }

    // at this point, we have either hit an error, or found that everything
    // required is already loaded, OR have found some services we need.
    // in the first two cases we hop over and return, in the third
    // we bounce a request off the server
    if (ctx->ec != BP_EC_OK || neededServices.size() == 0)
    {
        m_threadHopper.invokeOnThread(requireReturn, (void *) ctx);
    }
    else
    {
        Map requireArgs(*((bp::Map *) obj));
        // overwrite services key with the subset of required services
        requireArgs.add("services", neededServices.clone());
    
        BPErrorCode ec = BPRequire(m_protoHand,
                                   requireArgs.elemPtr(),
                                   requireServicesCallback,
                                   (void *) ctx,
                                   BPSession::executeMethodInvokeCallbackCB,
                                   (void *) this,
                                   &(ctx->transaction->m_bpProtoTransactionID));
        
        if (ec != BP_EC_OK) {
            if (obj) delete obj;
            obj = NULL;
            delete ctx;
            return false;
        }
    }

    if (obj) delete obj;
    obj = NULL;

    return true;
}
