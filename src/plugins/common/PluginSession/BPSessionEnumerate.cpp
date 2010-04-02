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

/**
 * Code which implements the plugin EnumerateServices function
 */
#include "BPSession.h"
#include "bppluginutil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "CommonErrors.h"



// a dynamically allocated structure freed by the callback for async
// case, by caller for sync case.
struct EnumerateContext
{
    bp::List serviceList;
    plugin::Object * callback;
    BPSession * bp;
};

void
BPSession::enumerateServicesCallback(BPErrorCode ec,
                                     void * cookie,
                                     const BPElement * corelets)
{
    EnumerateContext * ctx = (EnumerateContext *) cookie;    
    BPASSERT(ctx != NULL);

    if (ec == BP_EC_OK && corelets != NULL) {
        // now we should append the results of the enumerate call to
        // our bp::List containing combined pluglets and corelets
        if (!bp::pluginutil::appendEnumerateResultsToList(corelets,
                                                          ctx->serviceList))
        {
            BPLOG_ERROR("Error in BPEnumerate protcol response");
            ec = BP_EC_PROTOCOL_ERROR;
        }
    }

    BPSession* session = ctx->bp;
    BPPlugin& plugin = session->plugin();
    
    plugin::Variant* arg = plugin.allocVariant();
    plugin::Variant* result = plugin.allocVariant();
    
    if (ec == BP_EC_OK) {
        if (!session->generateSuccessReturn(ctx->serviceList, arg)) {
            // what can we do if this fails?
            (void) session->generateErrorReturn(NULL, NULL, arg);
        }
    } else {
        session->generateErrorReturn(BPErrorCodeToString(ec), NULL, arg);
    }
    
    BPASSERT(ctx->callback != NULL);
    plugin.callJsFunction(ctx->callback, &arg, 1, result);

    delete ctx->callback;
    delete ctx;
}

bool
BPSession::enumerateServices(const plugin::Object* callback,
                             plugin::Variant* result)
{
    if (notInitialized(callback, result)) return true;

    BPASSERT(callback != NULL);
    if (callback == NULL) return false;

    using namespace bp;

    // build up available pluglets as a json string
    std::list<Pluglet *> pluglets;
    std::list<Pluglet *>::iterator it;

    EnumerateContext * ctx = new EnumerateContext;
    ctx->callback = callback->clone();
    ctx->bp = this;

    pluglets = m_plugletRegistry->availablePluglets();
    
    for (it = pluglets.begin(); it != pluglets.end(); it++)
    {
        const bp::service::Description * def;
        def = (*it)->describe();
        BPASSERT(def != NULL);
        Map * m = new Map;
        m->add("name", new String(def->name()));
        m->add("version", new String(def->versionString()));
        m->add("type", new String("built-in"));
        ctx->serviceList.append(m);
    }

    BPErrorCode ec;
    
    ec = BPEnumerate(m_protoHand,
                     enumerateServicesCallback,
                     (void *) ctx);

    if (ec != BP_EC_OK) {
        delete ctx;
        return false;
    }

    return true;
}
