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

#ifndef __TRANSACTION_MANAGER_H__
#define __TRANSACTION_MANAGER_H__

#include <stddef.h>
#include "api/BPProtocolInterface.h"
#include "BPUtils/IPCChannel.h"

class Transaction 
{
  public:
    Transaction()
        : type(Enumerate), cookie(NULL), genericCB(NULL), describeCB(NULL),
          requireCB(NULL), resultsCB(NULL), invokeCB(NULL), invokeCookie(NULL)
    {
    }

    enum { Enumerate, Describe, GetState, Require, Invoke } type;
    
    // common to all transactions
    void * cookie;

    // common to multiple transactions
    BPGenericCallback genericCB;

    // secific to Describe
    BPDescribeCallback describeCB;

    // secific to Require
    BPRequireCallback requireCB;

    // secific to Require
    BPResultsCallback resultsCB;
    
    // common to transactions with callbacks
    BPInvokeCallback invokeCB;
    void * invokeCookie;
};

/**
 * A class to manage transactions.  Recieves responses from
 * the channel and delivers results
 */
class TransactionManager : virtual public bp::ipc::IChannelListener
{
  public:
    TransactionManager();
    
    void addTransaction(int tid, Transaction t);
    void cancelTransaction(int tid);    
    void setUserPromptCallback(BPUserPromptCallback cb, void * cookie);

    // pass a pointer to a bit that should be toggled when/if the
    // connection falls down because the peer ends it.
    void setPeerEndedBit(bool * peerEnded);
    
  private:
    std::map<int, Transaction> m_tmap;
    BPUserPromptCallback m_promptCB;
    void * m_promptCookie;
    bool * m_peerEnded;

    // three functions handle incoming messages over the IPC channel
    // (implementation of IChannelListener)
    virtual void onMessage(bp::ipc::Channel * c,
                           const bp::ipc::Message & m);
    virtual bool onQuery(bp::ipc::Channel * c,
                         const bp::ipc::Query & query,
                         bp::ipc::Response & response);
    virtual void onResponse(bp::ipc::Channel * c,
                            const bp::ipc::Response & response);

    // how we're notified when the channel falls down
    void channelEnded(bp::ipc::Channel * c,
                      bp::ipc::IConnectionListener::TerminationReason why,
                      const char * errorString);
};



#endif


