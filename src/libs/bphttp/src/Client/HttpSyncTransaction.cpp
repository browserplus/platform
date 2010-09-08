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
 *  HttpSyncTransaction.cpp
 *
 *  Implements the Transaction class and related items.
 *
 *  Created by David Grigsby on 10/22/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HttpSyncTransaction.h"


using namespace bp;
using bp::http::client::Transaction;


namespace bp {
namespace http {
namespace client {


std::tr1::shared_ptr<SyncTransaction>
SyncTransaction::alloc( RequestPtr ptrRequest )
{
    std::tr1::shared_ptr<SyncTransaction> rval(new SyncTransaction(ptrRequest));
    return rval;
}


SyncTransaction::SyncTransaction( RequestPtr ptrRequest ) :
    Listener(),
    m_thrd(),
    m_pTran(),
    m_fTimeoutSecs( Transaction::defaultTimeoutSecs() ),
    m_ptrRequest( ptrRequest ),
    m_results()
{
    m_thrd.setCallBacks( startFunc, this, NULL, NULL, NULL, NULL );
}


SyncTransaction::~SyncTransaction()
{
}


void SyncTransaction::setTimeoutSec( double fSecs )
{
    m_fTimeoutSecs = fSecs;
}


http::ResponsePtr
SyncTransaction::execute( FinalStatus& results )
{
    m_thrd.run();
    m_thrd.join();

    results = m_results;
    return response();
}


void SyncTransaction::startFunc( void* ctx )
{
    SyncTransaction* self = (SyncTransaction*) ctx;
    self->m_pTran.reset(new Transaction( self->m_ptrRequest ));
    self->m_pTran->setTimeoutSec( self->m_fTimeoutSecs );

    self->m_pTran->initiate( self->shared_from_this() );
}


void SyncTransaction::onClosed()
{
    Listener::onClosed();
    stopThread( FinalStatus::eOk );
}


void SyncTransaction::onTimeout()
{
    Listener::onTimeout();
    stopThread( FinalStatus::eTimedOut );
}


void SyncTransaction::onCancel()
{
    Listener::onCancel();
    stopThread( FinalStatus::eCancelled );
}


void SyncTransaction::onError(const std::string& msg)
{
    Listener::onError( msg );
    stopThread( FinalStatus::eError, msg );
}


void SyncTransaction::stopThread( FinalStatus::Code code, std::string sMsg )
{
    // Make sure to reset transaction here, so that it
    // gets destroyed on our thread.  All juicy http
    // goodness must occur on same thread.
    m_pTran.reset();

    m_results.code = code;
    m_results.message = sMsg;
    m_thrd.stop();
}


} // namespace client
} // namespace http
} // namespace bp
