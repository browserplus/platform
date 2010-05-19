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
 * (c) Yahoo 2007, all rights reserved
 * Written by Lloyd Hilaiel, on or around Tue May  8 15:11:53 MDT 2007
 *
 * This is the interface to a library which encapsulates the lightweight
 * protocol spoken by BrowserPlusCore.
 */

#include "api/BPProtocolInterface.h"
#include <string.h>
#include <stdlib.h>
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "SessionCreator.h"
#include "TransactionManager.h"


#define CHECK_HAND_STATE(_h) \
    if ((_h) == NULL) return BP_EC_INVALID_PARAMETER;  \
    if ((_h)->peerEndedConnection) return BP_EC_PEER_ENDED_CONNECTION;


void
BPInitialize()
{
}

void
BPShutdown()
{
}

struct BPProto_t 
{
    // the main channel
    bp::ipc::Channel channel;
    
    // responsible for handling incoming protocol messages
    TransactionManager manager;
    
    // non-null during initial session creation
    class SessionCreatorListener * listener;

    bool peerEndedConnection;
};

BPProtoHand BPAlloc()
{
    BPLOG_DEBUG("BPAlloc called");

    BPProtoHand hand = new BPProto_t;

    hand->listener = NULL;
    hand->peerEndedConnection = false;

    hand->manager.setPeerEndedBit(&(hand->peerEndedConnection));
    
    return hand;
}

class SessionCreatorListener : public ISessionCreatorListener
{
public:
    SessionCreatorListener(BPProtoHand hand,
                           BPConnectCallback connectCB,
                           void * cookie)
        : m_hand(hand), m_connectCB(connectCB), m_cookie(cookie),
          creator(&(hand->channel))
    {
        creator.setListener(this);
    }

    void start(const char * uri, const char * locale, const char * userAgent) 
    {
        creator.createSession(uri, locale, userAgent);
    }

    ~SessionCreatorListener() { }
private:
    BPProtoHand m_hand;
    BPConnectCallback m_connectCB;
    void * m_cookie;
    SessionCreator creator;
    
    void sessionCreated() 
    {
        creator.setListener(NULL);
        m_hand->listener = NULL;
        m_hand->channel.setListener(&(m_hand->manager));

        // notify client, remove self from handle
        if (m_connectCB) {
            m_connectCB(BP_EC_OK, m_cookie, NULL, NULL);
        }

        // now we delete ourselves.
        delete this;
    }
    
    void sessionCreationFailed(BPErrorCode e,
                               const std::string & errorString,
                               const std::string & verboseError)
    {
        // notify client, remove self from handle
        if (m_connectCB) {
            m_connectCB(e, m_cookie,
                        (!errorString.empty()) ? errorString.c_str() : NULL,
                        (!verboseError.empty()) ? verboseError.c_str() : NULL);
        }
        
        creator.setListener(NULL);
        m_hand->listener = NULL;
        m_hand->channel.setListener(NULL);
        // now we delete ourselves.
        delete this;
    }
};

void BPFree(BPProtoHand hand)
{
    BPLOG_DEBUG("BPFree called");
    if (hand) {
        if (hand->listener) delete hand->listener;
        hand->listener = NULL;
        delete hand;
        hand = NULL;
    }
}
    
BPErrorCode
BPConnect(BPProtoHand hand,
          const char * uri,
          const char * locale,
          const char * userAgent,
          BPConnectCallback connectCB,
          void * cookie)
{
    CHECK_HAND_STATE(hand);

    BPASSERT( hand->listener == NULL );
    hand->listener = new SessionCreatorListener(hand, connectCB, cookie);
    hand->listener->start(uri, locale, userAgent);
    
    return BP_EC_OK;
}

BPErrorCode
BPSetUserPromptCallback(
    BPProtoHand hand,
    BPUserPromptCallback cb,
    void * cookie)
{
    BPLOG_DEBUG("BPSetUserPromptCallback called");

    CHECK_HAND_STATE(hand);
    
    if (!cb) return BP_EC_INVALID_PARAMETER;

    hand->manager.setUserPromptCallback(cb, cookie);

    return BP_EC_OK;
}

BPErrorCode
BPExecute(BPProtoHand hand,
          const char * service,
          const char * serviceVersion,
          const char * functionName,
          const BPElement * args,
          BPResultsCallback resultsCB,
          void * resultsCookie,
          BPInvokeCallback invokeCB,
          void * invokeCookie,
          unsigned int * tidRV)
{
    CHECK_HAND_STATE(hand);

    // validate parameters
    if (service == NULL || serviceVersion == NULL || functionName == NULL)
    {
        return BP_EC_INVALID_PARAMETER;
    }

    bp::ipc::Query q;
    q.setCommand("Invoke");

    bp::Map * m = new bp::Map;
    m->add("service", new bp::String(service));
    m->add("version", new bp::String(serviceVersion));    
    m->add("function", new bp::String(functionName));    
    if (args) m->add("arguments", bp::Object::build(args));
    q.setPayload(m);

    // attempt to send the query
    if (!hand->channel.sendQuery(q)) return BP_EC_INVALID_STATE;

    // register the transaction
    Transaction t;
    t.type = Transaction::Invoke;
    t.resultsCB = resultsCB;
    t.cookie = resultsCookie;
    t.invokeCB = invokeCB;
    t.invokeCookie = invokeCookie;
    hand->manager.addTransaction(q.id(), t);
    
    if (tidRV != NULL) *tidRV = q.id();

    return BP_EC_OK;
}

BPErrorCode
BPCancel(BPProtoHand hand, unsigned int)
{
    CHECK_HAND_STATE(hand);
    // TODO: implement this to allow cancelling of transactions
    return BP_EC_NOT_IMPLEMENTED;
}

BPErrorCode BPRequire(BPProtoHand hand,
                      const BPElement * args,
                      BPRequireCallback requireCB,
                      void * requireCookie,
                      BPInvokeCallback invokeCB,
                      void * invokeCookie,
                      unsigned int * tidRV)
{
    CHECK_HAND_STATE(hand);

    if (!args || args->type != BPTMap || requireCB == NULL) {
        return BP_EC_INVALID_PARAMETER;
    }

    bp::Map * argsMap = (bp::Map *) bp::Object::build(args);

    bp::ipc::Query q;
    q.setCommand("Require");
    q.setPayload(argsMap);    // NOTE: query retains ownership of argsMap

    // validate a bit.  only two allowed members.
    if (!argsMap->has("services", BPTList)) {
        return BP_EC_INVALID_PARAMETER;
    }

    bool hasCallback = argsMap->has("progressCallback", BPTCallBack);
    if ((hasCallback && argsMap->size() != 2) ||
        (!hasCallback && argsMap->size() != 1))
    {
        return BP_EC_INVALID_PARAMETER;        
    }

    // attempt to send the query
    if (!hand->channel.sendQuery(q)) return BP_EC_INVALID_STATE;

    // register the transaction
    Transaction t;
    t.type = Transaction::Require;
    t.requireCB = requireCB;
    t.cookie = requireCookie;
    t.invokeCB = invokeCB;
    t.invokeCookie = invokeCookie;
    hand->manager.addTransaction(q.id(), t);
    
    if (tidRV != NULL) *tidRV = q.id();

    return BP_EC_OK;
}

BPErrorCode BPDescribe(BPProtoHand hand,
                       const char * coreletName,
                       const char * coreletVersion,
                       const char * coreletMinversion,
                       BPDescribeCallback describeCB,
                       void * cookie)
{
    CHECK_HAND_STATE(hand);

    // if no callback is provided, the call is useless
    if (describeCB == NULL || coreletName == NULL) {
        return BP_EC_INVALID_PARAMETER;
    }
    
    // build the query
    bp::ipc::Query q;
    q.setCommand("Describe");

    {
        bp::Map m;
        m.add("name", new bp::String(coreletName));
        if (coreletVersion) {
            m.add("version", new bp::String(coreletVersion));
        }
        if (coreletMinversion) {
            m.add("minversion", new bp::String(coreletMinversion));
        }
        q.setPayload(m);
    }
    
    // attempt to send the query
    if (!hand->channel.sendQuery(q)) return BP_EC_INVALID_STATE;

    // register the transaction
    Transaction t;
    t.type = Transaction::Describe;
    t.cookie = cookie;
    t.describeCB = describeCB;
    hand->manager.addTransaction(q.id(), t);
    
    return BP_EC_OK;
}

BPErrorCode
BPEnumerate(BPProtoHand hand,
            BPGenericCallback enumerateCB,
            void * cookie)
{
    CHECK_HAND_STATE(hand);
    
    // if no callback is provided, the call is useless
    if (enumerateCB == NULL) return BP_EC_INVALID_PARAMETER;

    // build the query
    bp::ipc::Query q;
    q.setCommand("ActiveServices");
    
    // attempt to send the query
    if (!hand->channel.sendQuery(q)) return BP_EC_INVALID_STATE;

    // register the transaction
    Transaction t;
    t.type = Transaction::Enumerate;
    t.cookie = cookie;
    t.genericCB = enumerateCB;
    hand->manager.addTransaction(q.id(), t);
    
    return BP_EC_OK;
}

BPErrorCode
BPGetState(BPProtoHand hand,
           const char * state,
           BPGenericCallback stateCB,
           void * cookie)
{
    CHECK_HAND_STATE(hand);

    // if no callback is provided, the call is useless
    if (stateCB == NULL || state == NULL) {
        return BP_EC_INVALID_PARAMETER;
    }
    
    // build the query
    bp::ipc::Query q;
    q.setCommand("GetState");
    q.setPayload(bp::String(state));
    
    // attempt to send the query
    if (!hand->channel.sendQuery(q)) return BP_EC_INVALID_STATE;

    // register the transaction
    Transaction t;
    t.type = Transaction::GetState;
    t.cookie = cookie;
    t.genericCB = stateCB;
    hand->manager.addTransaction(q.id(), t);

    return BP_EC_OK;
}

BPErrorCode
BPSetState(BPProtoHand hand,
           const char * state,
           const BPElement * newValue)
{
    CHECK_HAND_STATE(hand);

    // if no callback is provided, the call is useless
    if (state == NULL || newValue == NULL) {
        return BP_EC_INVALID_PARAMETER;
    }
    
    // build the query
    bp::ipc::Message m;
    m.setCommand("SetState");
    bp::Map * pl = new bp::Map;
    pl->add("state", new bp::String(state));
    pl->add("newValue", bp::Object::build(newValue));    
    m.setPayload(pl);
    
    // attempt to send the query
    if (!hand->channel.sendMessage(m)) return BP_EC_INVALID_STATE;

    return BP_EC_OK;
}


BPErrorCode
BPDeliverUserResponse(BPProtoHand hand, unsigned int tid,
                      const BPElement * response)
{
    CHECK_HAND_STATE(hand);

    BPLOG_DEBUG_STRM("delivering user response to " << tid);

    bp::ipc::Response r(tid);
    r.setCommand("PromptUser");

    if (response != NULL) {
        bp::Object * responseObj = bp::Object::build(response);
        BPASSERT(responseObj != NULL);
        r.setPayload(responseObj);
    }

    BPLOG_DEBUG_STRM("sending prompt user response: " << r.toHuman());
    
    if (!hand->channel.sendResponse(r)) {
        return BP_EC_INVALID_STATE;
    }
    
    return BP_EC_OK;
}

