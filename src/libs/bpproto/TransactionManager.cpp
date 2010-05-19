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

#include "TransactionManager.h"
#include "BPProtoUtil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpfile.h"

TransactionManager::TransactionManager()
    : m_promptCB(NULL), m_promptCookie(NULL), m_peerEnded(NULL)
{
}

void
TransactionManager::addTransaction(int tid, Transaction t)
{
    m_tmap[tid] = t;
}

void
TransactionManager::cancelTransaction(int tid)
{
    std::map<int, Transaction>::iterator it;
    it = m_tmap.find(tid);
    if (it != m_tmap.end()) m_tmap.erase(it);
}

void
TransactionManager::onMessage(bp::ipc::Channel *,
                              const bp::ipc::Message & m)
{
    BPLOG_DEBUG_STRM("Message received (" << m.command()
                     << "): " << m.toHuman());

    // handle callbacks
    if (!m.command().compare("InvokeCallback")) {
        if (!m.payload() || !m.payload()->has("tid", BPTInteger)) {
            BPLOG_WARN("InvokeCallback message received without 'tid', "
                       "dropping...");
            return;
        }
        if (!m.payload()->has("callbackInfo", BPTMap) ||
            !(m.payload()->get("callbackInfo")->has("callback", BPTInteger)))
        {
            BPLOG_WARN("InvokeCallback message received without "
                       "'callbackInfo/callback', dropping...");
            return;
        }
        
        // attain the id && callback handle
        unsigned int id = (unsigned int) ((long long) *(m.payload()->get("tid")));
        BPInteger cbHand = *(m.payload()->get("callbackInfo")->get("callback"));

        // attain the corresponding transaction
        std::map<int, Transaction>::iterator it = m_tmap.find(id);
        if (it == m_tmap.end()) {
            BPLOG_WARN_STRM("'InvokeCallback' received for unknown "
                            << "transaction: " << id);
            return;
        }
        
        // now 
        if (it->second.invokeCB != NULL) {
            // extract the parameters and pass them to the callback
            // as a BPElement pointer
            const bp::Object * bpParams =
                m.payload()->get("callbackInfo")->get("parameters");
            const BPElement * rv = NULL;
            if (bpParams) rv = bpParams->elemPtr();
            it->second.invokeCB(it->second.invokeCookie,
                                id, cbHand, rv);
        }
    } else {
        BPLOG_WARN_STRM("Invalid Message received from daemon: "
                        << m.command());
    }
}

bool
TransactionManager::onQuery(bp::ipc::Channel *,
                            const bp::ipc::Query & query,
                            bp::ipc::Response &)
{
    BPLOG_DEBUG_STRM("Query received (" << query.command()
                     << "): " << query.toHuman());

    // used on null arguments
    bp::Null nullArguments;

    // handle user prompt requests
    if (!query.command().compare("PromptUser")) {
        if (query.payload() && query.payload()->has("path", BPTString)) {
            std::string path = std::string(*(query.payload()->get("path")));
            const BPElement * arguments = NULL;
            if (query.payload()->has("arguments")) {
                arguments = query.payload()->get("arguments")->elemPtr();
            } else {
                arguments = nullArguments.elemPtr();
            }
            
            if (m_promptCB) {
                m_promptCB(m_promptCookie, path.c_str(),
                           arguments, query.id());
            }
        } else {
            BPLOG_WARN_STRM("Malformed prompt user message received: "
                            << query.toHuman());
            
        }
    } else {
        BPLOG_WARN_STRM("Invalid Query received from daemon: "
                        << query.command());
    }
    return false;
}

void
TransactionManager::onResponse(bp::ipc::Channel *,
                               const bp::ipc::Response & response)
{
    BPLOG_DEBUG_STRM("Response received (" << response.command()
                     << "): " << response.toHuman());
    
    // attain the id of the response
    unsigned int id = response.responseTo();
    
    // attain the corresponding transaction
    std::map<int, Transaction>::iterator it = m_tmap.find(id);
    if (it == m_tmap.end()) {
        BPLOG_WARN_STRM("Response received for unknown transaction: "
                        << id);
        return;
    }
    
    // now we know this is a valid response to a message we sent,
    // let's process the body
    BPErrorCode ec = BP_EC_OK;

    bp::Map em;
    const bp::Object * payload = NULL;
    // used when payload is absent
    bp::Null nullPayload;

    // ALL responses have a success key which indicates wether the
    // operation succeeded
    if (!response.payload() ||
        !response.payload()->has("success", BPTBoolean))
    {
        ec = BP_EC_PROTOCOL_ERROR;
    }
    else if ((bool) *(response.payload()->get("success")))
    {
        // for success responses we expect an optional payload
        if (response.payload()->has("value")) {
            payload = response.payload()->get("value");
        } else {
            payload = &nullPayload;
        }
    }
    else
    {
        // an error!
        std::string err, verbErr;
        mapResponseToErrorCode(response.payload(), ec, err, verbErr);
        if (ec == BP_EC_EXTENDED_ERROR) {
            if (!err.empty()) {
                em.add("error", new bp::String(err));
            }
            if (!verbErr.empty()) {
                em.add("verboseError", new bp::String(verbErr));
            }
            payload = &em;
        }
    }
    
    // at this point EC holds the error code, payload holds a pointer
    // to the payload.  Some protocol messages (those that take
    // GenericCallbackPointers) are ready to return

    Transaction t = it->second;
    
    // now remove the transaction (before we call back into our
    // client who may chose to delete us.)
    m_tmap.erase(it);

    if ((!response.command().compare("ActiveServices") &&
         t.type == Transaction::Enumerate) ||
        (!response.command().compare("GetState") &&
         t.type == Transaction::GetState))
    {
        if (t.genericCB) {
            t.genericCB(ec, t.cookie, payload ? payload->elemPtr() : NULL);
        }
    }
    else if (!response.command().compare("Invoke"))
    {
        if (t.resultsCB) {
            t.resultsCB(t.cookie, id, ec,
                         payload ? payload->elemPtr() : NULL);
        }
    }
    else if (!response.command().compare("Require"))
    {
        if (t.requireCB) {        
            BPCoreletDefinition ** defs = NULL;
            unsigned int numDefs = 0;
    
            std::string e, ve;
    
            if (ec == BP_EC_OK) {
                // translate "results" into a list of BPCoreletDefinition 
                if (!payload || payload->type() != BPTList) {
                    ec = BP_EC_PROTOCOL_ERROR;
                } else {
                    std::vector<const bp::Object *> l = *payload;

                    numDefs = l.size();
                    defs = new BPCoreletDefinition*[numDefs];
                    for (unsigned int i = 0; i < numDefs; ++i) {
                        defs[i] = objectToDefinition(l[i]);
                    }
                }
            } else if (ec == BP_EC_EXTENDED_ERROR) {
                extractExtendedError(payload, e, ve);
            }
    
            // invoke client callback
            t.requireCB(ec, t.cookie,
                         (const BPCoreletDefinition**) defs,
                         numDefs,
                         e.empty() ? NULL : e.c_str(),
                         ve.empty() ? NULL : ve.c_str());
            
            if (ec == BP_EC_OK) {
                for (unsigned int i = 0; i < numDefs; ++i) {
                    freeDefinition(defs[i]);
                }
                delete [] defs;
            }
        }
    }
    else if (!response.command().compare("Describe") &&
             t.type == Transaction::Describe)
    {
        BPCoreletDefinition * def = NULL;
        if (ec == BP_EC_OK) {
            // extract payload
            if (payload) {
                // translate "results" into a BPCoreletDefinition 
                def = objectToDefinition(payload);
            }
            if (def == NULL) ec = BP_EC_PROTOCOL_ERROR;
        }
    
        // for extended errors, let's extract error and verbose eror 
        std::string e;
        std::string ve;        

        if (ec == BP_EC_EXTENDED_ERROR) {
            if (response.payload()->has("error", BPTString)) {
                e = std::string(*(response.payload()->get("error")));
            }
            if (response.payload()->has("verboseError", BPTString)) {
                ve = std::string(*(response.payload()->get("verboseError")));
            }
        }
    
        // invoke client callback
        if (t.describeCB) {
            t.describeCB(ec, t.cookie, def,
                          e.empty() ? NULL : e.c_str(),
                          ve.empty() ? NULL : ve.c_str());
        }
    }
    else
    {
        BPLOG_WARN_STRM("Unexpected response received: "
                        << response.command());        
    }
}

void
TransactionManager::channelEnded(
    bp::ipc::Channel *,
    bp::ipc::IConnectionListener::TerminationReason why,
    const char * errorString)
{
    BPLOG_INFO_STRM(
        "Connection to BrowserPlusCore ended ("
        << bp::ipc::IConnectionListener::terminationReasonToString(why)
        << ")" << (errorString ? " - " : "")
        << (errorString ? errorString : ""));
    
    if (why != bp::ipc::IConnectionListener::DisconnectCalled) {
        if (m_peerEnded) *m_peerEnded = true;
    }
}

void
TransactionManager::setUserPromptCallback(BPUserPromptCallback cb,
                                          void * cookie)
{
    m_promptCB = cb;
    m_promptCookie = cookie;
}

void
TransactionManager::setPeerEndedBit(bool * peerEnded)
{
    m_peerEnded = peerEnded;
}
