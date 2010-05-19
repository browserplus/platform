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
 * Code which implements the plugin DescribeServices function
 */
#include "BPSession.h"
#include "bppluginutil.h"
#include "BPUtils/bpdefutil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "CommonErrors.h"


// a dynamically allocated structure freed by the callback
struct DescribeContext
{
    DescribeContext()
        : serviceDescription(NULL),
          ec(BP_EC_OK),
          callback(NULL),
          transaction(NULL),
          bp(NULL)
    {
    }
    
    ~DescribeContext() {
        if (serviceDescription) delete serviceDescription;
        if (callback) delete callback;
        // transaction is deleted in removeTransaction()
        //if (transaction) delete transaction;
    }

    // otherwise json return
    bp::Object * serviceDescription;

    // the result of the describe
    BPErrorCode ec;

    // error information when ec == BP_EC_EXTENDED_ERROR,
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
BPSession::describeReturn(void * cookie)
{
    DescribeContext * ctx = (DescribeContext *) cookie;   
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
    } else if (ctx->serviceDescription == NULL) {
        BPLOG_ERROR("Internal Error: expected ctx->serviceDescription");
        session->generateErrorReturn("internalError", NULL, arg);
    } else {
        if (!session->generateSuccessReturn(*(ctx->serviceDescription), arg))
        {
            // what can we do if this fails?
            BPLOG_ERROR("Internal Error: cannot generate success return");
            (void) session->generateErrorReturn("internalError", NULL, arg);
        }
    }

    BPASSERT(ctx->callback != NULL);

    plugin.callJsFunction(ctx->callback, &arg, 1, result);

    plugin.freeVariant(arg);
    plugin.freeVariant(result);    

    session->removeTransaction(ctx->transaction);

    delete ctx;
}

void
BPSession::describeServiceCallback(BPErrorCode ec,
                                   void * cookie,
                                   const BPServiceDefinition * def,
                                   const char * error,
                                   const char * verboseError)
{
    DescribeContext * ctx = (DescribeContext *) cookie;   
	BPASSERT(ctx != NULL);

    ctx->ec = ec;
    if (error) ctx->error.append(error);
    if (verboseError) ctx->verboseError.append(verboseError);    
    if (def) ctx->serviceDescription = bp::defutil::defToJson(def);
    
    describeReturn(ctx);
}

bool
BPSession::describeService(const plugin::Variant* args,
                           const plugin::Object* callback,
                           plugin::Variant* result)
{
    BPErrorCode ec = BP_EC_OK;
    std::string service, version, minversion;
    
    if (notInitialized(callback, result)) return true;

    BPASSERT(callback != NULL);
    if (callback == NULL) return false;

    using namespace bp;
    
    // turn the nasty Variant args into something less nasty
    Object * obj = NULL;
    BPTransaction * transaction = new BPTransaction;
    if (!variantToBPObject(args, transaction, obj)) {
        delete transaction;        
        return false;
    }

    // after this point all errors will be returned via the callback
    // after returning from our function
    DescribeContext * ctx = new DescribeContext;
    ctx->transaction = transaction;
    addTransaction(ctx->transaction);
    ctx->callback = callback->clone();
    ctx->bp = this;

    if (obj->type() != BPTMap)
    {
        ctx->ec = BP_EC_EXTENDED_ERROR;
        ctx->error.append(pluginerrors::InvalidParameters);
        ctx->verboseError.append("describeService requires an object argument");
    }
    else if (!obj->has("service", BPTString))
    {
        ctx->ec = BP_EC_EXTENDED_ERROR;
        ctx->error.append(pluginerrors::InvalidParameters);
        ctx->verboseError.append("describeService argument requires a "
                                 "'service' key");
    }
    else
    {
        service = std::string(*(obj->get("service")));

        // "version" and "minversion" are optional
        if (obj->has("version", BPTString)) {
            version = std::string(*(obj->get("version")));
        }
        if (obj->has("minversion", BPTString)) {
            minversion = std::string(*(obj->get("minversion")));
        }
        
        // is this service already loaded?
        bp::Map * serviceDesc = NULL;
        {
            bp::service::Description d;
            if (findLoadedService(service, version, minversion, d)) {
                serviceDesc = (bp::Map *) d.toBPObject();
            }
        }

        // is there an applicable pluglet?
        Pluglet * pluglet =
            m_plugletRegistry->find(service, version, minversion);
        
        // if the service is not yet loaded, but there is an applicable
        // pluglet, extract the definition from the pluglet.
        if (!serviceDesc && pluglet != NULL)
        {
            serviceDesc = dynamic_cast<bp::Map *>(
                pluglet->describe()->toBPObject());
            BPASSERT(serviceDesc != NULL);
        }
        
        // if we found a serviceDesc, we can return it
        ctx->serviceDescription = serviceDesc;
    }

    delete obj;

    // at this point, if we found a service description OR encountered
    // an error, we should hop over and return.  Otherwise we should
    // hit the server to get a description
    if (ctx->serviceDescription || ctx->ec != BP_EC_OK) {
        // let our function return before we invoke callback
        m_threadHopper.invokeOnThread(describeReturn, (void *) ctx);
    } else {
        ec = BPDescribe(m_protoHand,
                        service.c_str(),
                        version.c_str(),
                        minversion.c_str(),
                        describeServiceCallback,
                        (void *) ctx);

        if (ec != BP_EC_OK) {
            delete ctx;
            return false;
        }
    }

    return true;
}

