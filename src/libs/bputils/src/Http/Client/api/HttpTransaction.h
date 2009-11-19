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

/*
 *  HttpTransaction.h
 *
 *  Declares HttpTransaction and related items.
 *
 *  Created by David Grigsby on 7/19/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPTRANSACTION_H_
#define _HTTPTRANSACTION_H_

#include <boost/scoped_ptr.hpp>
#include "HttpRequest.h"
#include "HttpResponse.h"


namespace bp {
namespace http {
namespace client {


class Transaction
{
// Construction/destruction
public:
    Transaction( RequestPtr ptrRequest );
    ~Transaction();

// Class-scope Items
public:
    static double defaultTimeoutSecs();
    
// Methods
public:
    // Initiate the asynchronous transaction.
    // This call will start the transaction and return immediately.
    // Specifying a NULL listener will cause a fatal exception.
    // Caller should not modify request body until request completes.
    void initiate( IListener* pListener );
    
    // Cancel a transaction.  For an asynchronous request, the
    // the listener's onCancel() will be invoked.  For a 
    // synchronous request, the returned FinalStatus will
    // have a code of eCancelled
    void cancel();
    
// Accessors
public:
    RequestPtr request() const;
    
    // Return the current reported user agent
    const std::string& userAgent() const;

    // Set the user agent to report
    void setUserAgent( const std::string& sUserAgent );
            
    // Return current timeout in seconds
    double timeoutSec() const;

    // Set timeout value in seconds.  If a timeout occurs
    // for an asynchronous request, the the listener's 
    // onTimeout() will be invoked.  For a synchronous 
    // request, the returned FinalStatus will
    // have a code of eTimedOut
    void setTimeoutSec( double fSecs );
    
// State
private:    
    class Impl;
    boost::scoped_ptr<Impl> m_pImpl;

// Prevent copying
private:
    Transaction( const Transaction& );
    Transaction& operator=( const Transaction& );
};


} // namespace client
} // namespace http
} // namespace bp


#endif // _HTTPTRANSACTION_H

