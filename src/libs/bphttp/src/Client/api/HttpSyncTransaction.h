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

/*
 *  HttpSyncTransaction.h
 *
 *  Declares HttpSyncTransaction and related items.
 *
 *  Created by David Grigsby on 10/22/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPSYNCTRANSACTION_H_
#define _HTTPSYNCTRANSACTION_H_

#include "BPUtils/bprunloopthread.h"
#include "HttpListener.h"
#include "HttpRequest.h"
#include "HttpTransaction.h"


namespace bp {
namespace http {
namespace client {


//////////////////////////////////////////////////////////////////////
// SyncTransaction
// 
// Represents a synchronous http transaction
class SyncTransaction : public http::client::Listener,
                        public std::tr1::enable_shared_from_this<SyncTransaction>
{
public:
    // A SyncTransaction must be managed with a shared_ptr, enforce that
    static std::tr1::shared_ptr<SyncTransaction> alloc( RequestPtr ptrRequest );

    virtual ~SyncTransaction();

// Classes
public:
    // Holds transaction final status info
    struct FinalStatus
    {
        enum Code { eOk, eTimedOut, eCancelled, eError };
        FinalStatus() : code(eOk), message() {}
        Code code;
        std::string message;
    };
    
// Public Methods
public:
    void setTimeoutSec( double fTimeoutSec );
    
    // Synchronously execute http transaction.
    // This blocks until complete, timeout, error, or cancellation.
    // Note: you must examine FinalStatus in order to interpret the
    //       returned ResponsePtr correctly.
    http::ResponsePtr execute( FinalStatus& results );

// IListener    
public:
    // overrides from Listener
    void onClosed();
    void onTimeout();
    void onCancel();
    void onError(const std::string& msg);

// Internal Methods
private:
    static void startFunc( void* ctx );
    
    void stopThread( FinalStatus::Code code, std::string sMsg="" );

// State    
private:
    runloop::RunLoopThread  m_thrd;
    TransactionPtr          m_pTran;
    double                  m_fTimeoutSecs;
    RequestPtr              m_ptrRequest;
    FinalStatus             m_results;

protected:
    SyncTransaction( RequestPtr ptrRequest );
    
// Prevent copying
private:
    SyncTransaction( const SyncTransaction& );
    SyncTransaction& operator=( const SyncTransaction& );
};

typedef std::tr1::shared_ptr<SyncTransaction> SyncTransactionPtr;

} // namespace client
} // namespace http
} // namespace bp


#endif // _HTTPSYNCTRANSACTION_H

