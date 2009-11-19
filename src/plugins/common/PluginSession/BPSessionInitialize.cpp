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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * Code which implements the plugin Initialize function
 */

#include <assert.h>
#include "BPSession.h"
#include "BPUtils/BPLog.h"
#include "PluginCommonLib/CommonErrors.h"

// disable warnings about assignment within conditional expression
#ifdef WIN32
#pragma warning(disable:4706)
#endif

// a dynamically allocated structure freed by the callback for async
// case, by caller for sync case.
struct InitializeContext
{
    // The javascript function to callback when connect is complete
    plugin::Object * callback;

    // browserplus object
    BPSession * bp;
};

static bool
generateReturn(const BPSession * session, BPErrorCode ec,
               plugin::Variant* result,
               const char * error, const char * verboseError)
{
    bool rv = true;
    
    if (ec == BP_EC_OK) {
        bp::Null v;
        rv = session->generateSuccessReturn(v, result);
    } else {
        const char * err = NULL;
        switch (ec) {
            case BP_EC_UNAPPROVED_DOMAIN:
                err = pluginerrors::UnapprovedDomain;
                break;
            case BP_EC_PLATFORM_BLACKLISTED:
                err = pluginerrors::PlatformBlacklisted;
                break;
            case BP_EC_PERMISSIONS_ERROR:
                err = pluginerrors::PermissionsError;
                break;
            case BP_EC_PLATFORM_DISABLED:
                err = pluginerrors::PlatformDisabled;
                break;
            case BP_EC_NOT_INSTALLED:
                err = pluginerrors::NotInstalled;
                break;
            case BP_EC_SWITCH_VERSION:
                err = pluginerrors::SwitchVersion;
                break;
            case BP_EC_EXTENDED_ERROR:
                err = error;
                break;
            default:
                err = pluginerrors::ConnectionError;
                break;
        }
        rv = session->generateErrorReturn(err, verboseError, result);
    }
    return rv;
}

void
BPSession::connectCallback(BPErrorCode ec,
                           void * cookie,
                           const char * error,
                           const char * verboseError)
{
    BPLOG_INFO_STRM("\t-> BPConnect callback returns " << 
                    BPErrorCodeToString(ec));
    
    InitializeContext * ctx = (InitializeContext *) cookie;    
    assert(ctx != NULL);

    BPSession* session = ctx->bp;
    BPPlugin& plugin = session->plugin();
    
    plugin::Variant* arg = plugin.allocVariant();
    plugin::Variant* result = plugin.allocVariant();

    // here's one of two places initialized is set to true
    if (ec == BP_EC_OK) {
        session->m_initialized = true;
    }

    (void) generateReturn(session, ec, arg, error, verboseError);

    assert(ctx->callback != NULL);
    plugin.callJsFunction(ctx->callback, &arg, 1, result);

    plugin.freeVariant(arg);
    plugin.freeVariant(result);    

    delete ctx->callback;
    delete ctx;
}

void
BPSession::alreadyConnected(void * cookie)
{

    InitializeContext * ctx = (InitializeContext *) cookie;    
    ctx->bp->connectCallback(BP_EC_OK, cookie, NULL, NULL);
#ifdef WIN32
    // Wow, doze complains that "ctx" is initialized but
    // not referenced.  I guess that "ctx->" doesn't count.
    // #pragma warning(disable : 4189) didn't work either.
    // Bad compiler, no cookies for you!
    ctx;
#endif
}

bool
BPSession::initialize(const plugin::Variant* args,
                      const plugin::Object* callback, 
                      plugin::Variant* result)
{
    using namespace bp;

    assert(callback != NULL);
    if (callback == NULL)
        return false;

    std::string locale;
    if (args != NULL) {
        Object* obj = NULL;
        Map* argsMap = NULL;
        BPTransaction* transaction = new BPTransaction;
        if (!variantToBPObject(args, transaction, obj)
            || !(argsMap = dynamic_cast<Map*>(obj))) {
            generateErrorReturn(pluginerrors::InvalidParameters, NULL, result);
            delete transaction;
            delete obj;
            return false;
        }
        delete transaction;
        
        const String* s = dynamic_cast<const String*>(argsMap->value("locale"));
        if (s) {
            locale = s->value();
        }
    }
    
    // tell pluglets about locale
    if (!locale.empty()) {
        std::list<Pluglet*> pl = m_plugletRegistry->availablePluglets();
        std::list<Pluglet*>::iterator it;
        for (it = pl.begin(); it != pl.end(); ++it) {
            (*it)->setLocale(locale);
        }
    }

    InitializeContext * ctx = new InitializeContext;
    ctx->bp = this;
    ctx->callback = callback->clone();
    
    // If we're already initialized, use thread hopper to allow function
    // to return and then immediately invoke callback
    if (m_initialized) {
        m_threadHopper.invokeOnThread(alreadyConnected, (void *) ctx);
        return true;
    }

    // first let's set a handler for the user prompt callback
    BPErrorCode ec = BPSetUserPromptCallback(m_protoHand,
                                             handleUserPrompt,
                                             (void *) this);
    if (ec != BP_EC_OK) {
        BPLOG_ERROR("Failed to set user prompt handler!");        
    }
        
    // determine current URL if possible
    std::string url;
    if (!plugin().getCurrentURL(url)) {
        BPLOG_ERROR("\t-> COULDN'T DETERMINE URL!");
    } else {
        BPLOG_DEBUG_STRM("\t-> url: '" << url << "'");
    }

    // access user agent
    std::string userAgent = plugin().getUserAgent();
    BPLOG_DEBUG_STRM("\t-> userAgent: '" << userAgent << "'");

    // now connect to browserplus.
    ec = BPConnect(m_protoHand, url.c_str(),
                   locale.c_str(), userAgent.c_str(),
                   connectCallback, (void *) ctx);

    BPLOG_INFO_STRM("\t-> BPConnect function returns sync "
                    << BPErrorCodeToString(ec));

    return true;
}
