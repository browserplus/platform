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
 * Code which implements the plugin InvokeMethod function
 */

#include "BPSession.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "PluginCommonLib/bppluginutil.h"
#include "PluginCommonLib/CommonErrors.h"


// a dynamically allocated structure freed by the callback for async
// case, by caller for sync case.
struct ExecuteContext
{
    bp::Object * results;

    // a callback.  when non-null, this indicates 
    plugin::Object * callback;

    // session this transaction is associated with object
    BPSession * bp;

    // session this transaction is associated with object
    BPTransaction * transaction;

    BPErrorCode ec;
    std::string error;
    std::string verboseError;
};

void
BPSession::executeMethodRelayResultsCB(void * cookie)
{
    ExecuteContext * ctx = (ExecuteContext *) cookie;    

    BPSession* session = ctx->bp;
    BPPlugin& plugin = session->plugin();

    plugin::Variant* arg = plugin.allocVariant();
    plugin::Variant* result = plugin.allocVariant();

    // translate results
    {
        bp::Object * safeObj = NULL;
        if (ctx->results &&
            bp::pluginutil::toBrowserSafeRep(ctx->results, safeObj))
        {
            delete ctx->results;
            ctx->results = safeObj;
        }
    }

    if (ctx->ec != BP_EC_OK) {
        const char * e = NULL, * ve = NULL;
        if (ctx->ec == BP_EC_EXTENDED_ERROR) {
            e = ctx->error.c_str();
            ve = ctx->verboseError.c_str();
        } else {
            e = BPErrorCodeToString(ctx->ec);
        }
        session->generateErrorReturn(e, ve, arg);
    }
    else if (ctx->results == NULL)
    {
        BPLOG_ERROR("Internal Error: expected non-null ctx->results");
        session->generateErrorReturn("internalError", NULL, arg);
    }
    else if (!session->generateSuccessReturn(*(ctx->results), arg))
    {
        // what can we do if this fails?
        BPLOG_ERROR("Internal Error: cannot generate success return");
        (void) session->generateErrorReturn("internalError", NULL, arg);
    }

    BPASSERT(ctx->callback != NULL);

    (void) plugin.callJsFunction(ctx->callback, &arg, 1, result);

    plugin.freeVariant(arg);
    plugin.freeVariant(result);    

    delete ctx->callback;

    session->removeTransaction(ctx->transaction);

    if (ctx->results) delete ctx->results;
    delete ctx;
}

void
BPSession::pletSuccessCB(void * cookie, unsigned int /*tid*/,
                         const bp::Object * returnValue)
{
    ExecuteContext * ctx = (ExecuteContext *) cookie;    
    BPASSERT(ctx != NULL);

    ctx->ec = BP_EC_OK;
    if (returnValue) {
        ctx->results = returnValue->clone();
    }

    // we need to call back into javascript from the correct thread
    BPASSERT(ctx->callback != NULL);

    ctx->bp->m_threadHopper.invokeOnThread(
        executeMethodRelayResultsCB, (void *) ctx);
}

void
BPSession::pletFailureCB(void * cookie, unsigned int /*tid*/,
                         const char * error,
                         const char * verboseError)
{
    ExecuteContext * ctx = (ExecuteContext *) cookie;    

    BPASSERT(ctx != NULL);

    ctx->ec = BP_EC_EXTENDED_ERROR;
    ctx->error.append(error ? error : "unknown");
    if (verboseError) ctx->verboseError.append(verboseError); 

    ctx->bp->m_threadHopper.invokeOnThread(
        executeMethodRelayResultsCB, (void *) ctx);
}

void
BPSession::executeMethodResultsCB(void * cookie,
                                  unsigned int /*tid*/,
                                  BPErrorCode ec,
                                  const BPElement * results)
{
    ExecuteContext * ctx = (ExecuteContext *) cookie;    

    BPASSERT(ctx != NULL);

    ctx->ec = ec;
    if (ctx->ec == BP_EC_OK || ctx->ec == BP_EC_EXTENDED_ERROR) {
        ctx->results = bp::Object::build(results);
    } 

    if (ctx->ec == BP_EC_EXTENDED_ERROR && ctx->results) {
        // extract error and verbose error
        if (ctx->results->has("error", BPTString)) {
            ctx->error = std::string(*(ctx->results->get("error")));
        }

        if (ctx->results->has("verboseError", BPTString)) {
            ctx->verboseError =
                std::string(*(ctx->results->get("verboseError")));
        }
        delete ctx->results;
        ctx->results = NULL;
    }
    
    BPASSERT(ctx->callback != NULL);

    // call back into javascript
    executeMethodRelayResultsCB((void *) ctx);
}

bool
BPSession::executeMethod(const std::string &service,
                         const std::string &version,
                         const std::string &method,
                         const plugin::Variant * arguments,
                         const plugin::Object * callback, 
                         plugin::Variant * result)
{
    if (notInitialized(callback, result)) return true;

    BPLOG_INFO_STRM("executeMethod " << service <<
                    "[" << version << "]." << method);

    BPASSERT(callback != NULL);
    if (callback == NULL) return false;
    
    using namespace bp;

    // now we'll map handles and callbacks, and validate we can
    // access the input arguments
    bp::Object * argsObj = NULL;
    bp::Map * args = NULL;

    BPTransaction * transaction = new BPTransaction;
    if (!variantToBPObject(arguments, transaction, argsObj) ||
        argsObj->type() != BPTMap)
    {
        delete transaction;
        delete argsObj;
        return false;
    }

    args = dynamic_cast<bp::Map *>(argsObj);
    argsObj = NULL;

    // Log method args at debug level.
    if (args) {
        BPLOG_DEBUG_STRM(service << "[" << version << "]." << method <<
                         "(" << args->toPlainJsonString(true) << ")");
    }
    
    // allocate an execution context ptr.
    ExecuteContext * ctx = new ExecuteContext;
    ctx->callback = callback->clone();
    ctx->bp = this;
    ctx->results = NULL;
    ctx->ec = BP_EC_OK;
    ctx->transaction = transaction;    

    // From this point on all errors will be returned via the callback
    addTransaction(ctx->transaction);

    bp::service::Description serviceDesc;
    bp::service::Function funcDesc;
    std::string vErr;

    // ensure service is loaded
    if (!findLoadedService(service, version, "", serviceDesc)) {
        std::stringstream ss;
        ss << service << " (" << version << ") hasn't been loaded";
        BPLOG_INFO_STRM("invoke failure: " << ss.str());
        ctx->ec = BP_EC_EXTENDED_ERROR;
        ctx->error.append(pluginerrors::ServiceNotLoaded);
        ctx->verboseError.append(ss.str());
    }
    // ensure the method exists on the service
    else if (!serviceDesc.getFunction(method.c_str(), funcDesc))
    {
        std::stringstream ss;
        ss << service << " doesn't have a '" << method << "' method";
        BPLOG_INFO_STRM("invoke failure: " << ss.str());
        ctx->ec = BP_EC_EXTENDED_ERROR;
        // Add to common errors 
        ctx->error.append("BP.noSuchFunction");
        ctx->verboseError.append(ss.str());
    }
    // validate arguments
    else if (!(vErr = bp::service::validateArguments(funcDesc, args)).empty())
    {
        BPLOG_INFO_STRM("invalid parameters in invoke: " << vErr);        
        ctx->ec = BP_EC_EXTENDED_ERROR;
        ctx->error.append(pluginerrors::InvalidParameters);
        ctx->verboseError = vErr;
    }

    bool isPluglet = false;
    
    // now let's add the transaction to the list
    if (ctx->ec == BP_EC_OK) {
        // we must either invoke the pluglet or the corelet

        // first, check if this is a pluglet
        Pluglet * pluglet = m_plugletRegistry->find(service, version, "");
        if (pluglet != NULL) {
            pluglet->execute(ctx->transaction->tid(), method.c_str(),
                             args, callback == NULL, pletSuccessCB,
                             pletFailureCB, pletCallbackCB, (void *) ctx);
            isPluglet = true;
        } else {
            ctx->ec = BPExecute(m_protoHand, service.c_str(), version.c_str(),
                                method.c_str(),
                                args ? args->elemPtr() : NULL,
                                BPSession::executeMethodResultsCB,
                                (void *) ctx,
                                BPSession::executeMethodInvokeCallbackCB,
                                (void *) this,
                                &(ctx->transaction->m_bpProtoTransactionID));
        }
    }
    
    // handle errors
    if (!isPluglet && ctx->ec != BP_EC_OK) {
        delete ctx->transaction;
        ctx->transaction = NULL;
        // hop over to the return function and return after the
        // function does
        m_threadHopper.invokeOnThread(executeMethodRelayResultsCB,
                                      (void *) ctx);    
    }

    if (args) delete args;

    return true;
}

// a class passed from the thread that BPProtocol calls us on, via
// thread hopper, over to the ui thread where we can call back into
// user javascript.
class CallbackInvocationContext
{
public:
    unsigned int m_protoTid;
    unsigned int m_tid;
    long long int m_cbHand;
    bp::Object * m_args;
    BPSession * bp;
};

void 
BPSession::executeMethodInvokeCallbackRelayCB(void * cookie)
{
    CallbackInvocationContext * cic = (CallbackInvocationContext *) cookie;
    BPPlugin& plugin = cic->bp->plugin();

    plugin::Variant* result = plugin.allocVariant();
    plugin::Variant* arg = plugin.allocVariant();

    {
        bp::Object * jsonObj = NULL;
    
        // now we can translate results
        if (cic->m_args && bp::pluginutil::toBrowserSafeRep(cic->m_args, jsonObj)) {
            (void) plugin.evaluateJSON(jsonObj, arg);
        }
        
        if (jsonObj) delete jsonObj;
    }

    // now let's find the transaction
    plugin::Object * callback = NULL;
    
    BPTransaction * trans = NULL;
    if (cic->m_protoTid != 0) {
        trans = cic->bp->findTransactionByProtoID(cic->m_protoTid);    
    } else {
        trans = cic->bp->findTransactionByTID(cic->m_tid);    
    }
    
    if (trans) {
        callback = trans->findCallback(cic->m_cbHand);
    }

    // now we can invoke JS function
    if (callback) {
        (void) plugin.callJsFunction(callback, &arg, 1, result);    
    }

    plugin.freeVariant(result);
    plugin.freeVariant(arg);
    delete cic;
}

void
BPSession::executeMethodInvokeCallbackCB(void * cookie,
                                         unsigned int tid,
                                         long long int cbHand,
                                         const BPElement * params)
{
    CallbackInvocationContext * cic = new CallbackInvocationContext;
    cic->m_protoTid = tid;
    cic->m_tid = 0;
    cic->m_cbHand = cbHand;
    cic->m_args = NULL;
    cic->bp = (BPSession *) cookie;
    if (params) {
        cic->m_args = bp::Object::build(params);
    }
    executeMethodInvokeCallbackRelayCB((void *) cic);    
}

void
BPSession::pletCallbackCB(void * cookie, unsigned int tid,
                          BPCallBack cbHand,
                          const bp::Object * args)
{
    ExecuteContext * ctx = (ExecuteContext *) cookie;    

    CallbackInvocationContext * cic = new CallbackInvocationContext;
    cic->m_tid = tid;
    cic->m_protoTid = 0;
    cic->m_cbHand = cbHand;
    cic->m_args = NULL;
    cic->bp = ctx->bp;
    if (args) {
        cic->m_args = args->clone();
    }
    cic->bp->m_threadHopper.invokeOnThread(
        executeMethodInvokeCallbackRelayCB, (void *) cic);    
}
