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
 *  HttpListener.h
 *
 *  Declares IHttpListener and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPLISTENER_H_
#define _HTTPLISTENER_H_

#include "bpstrutil.h"
#include "bpurl.h"
#include "HttpHeaders.h"
#include "HttpStatus.h"
#include "HttpResponse.h"


namespace bp {
namespace http {
namespace client {

// Interface for listener to Transaction::initiate().  All callbacks will
// be invoked on the thread which called Transaction::initiate().
//
class IListener
{
public:
    // Beginning connection
    //
    virtual void onConnecting() = 0;
    
    // Connection completed
    //
    virtual void onConnected() = 0;
    
    // Request for redirect to newUrl.
    //
    virtual void onRedirect( const bp::url::Url& newUrl ) = 0;
    
    // HTTP request sent
    //
    virtual void onRequestSent() = 0;
    
    // Response status and headers received.  In rare cases more than one
    // status/headers will be received (for example in the case of an HTTP load
    // where the content type of the load data is multipart/x-mixed-replace).
    // In the event this occurs, listener should discard all data previously
    // delivered by onResponseBodyBytes(), and should be prepared to handle the
    // potentially different MIME type reported by the newly reported URL response.
    //
    virtual void onResponseStatus( const Status& status,
                                   const Headers& headers ) = 0;
    
    // Response body received.  Listener should concatenate the newly
    // available bytes.  The pBytes pointer is only guaranteed to remain
    // valid thru the life of this callback.
    //
    virtual void onResponseBodyBytes( const unsigned char* pBytes, 
                                      unsigned int size ) = 0;
    
    // Transmit progress callback for async operations.  If the total size 
    // to be transferred is unknown, totalBytes and percent will be zero, but
    // bytesProcessed will be valid.  Unless an error occurs, clients 
    // are guaranteeed to receive 0% and 100%, and the same percent will not
    // be sent more that once.
    //
    virtual void onSendProgress( size_t bytesProcessed,
                                 size_t totalBytes,
                                 double percent ) = 0;
    
    // Receive progress callback for async operations.  If the total size 
    // to be transferred is unknown, totalBytes and percent will be zero, but
    // bytesProcessed will be valid.  Unless an error occurs, clients 
    // are guaranteeed to receive 0% and 100%, and the same percent will not
    // be sent more that once.
    //
    virtual void onReceiveProgress( size_t bytesProcessed,
                                    size_t totalBytes,
                                    double percent ) = 0;
    
    // A listener will get exactly one of onClosed(), onTimeout(), onCancel(),
    // or onError().  Once one of these has been received, the listener
    // will receive no more callbacks

    // Request complete.  Be very careful, this does not indicate
    // that the transaction is completely done.  onClosed() is still
    // on the way.
    //
    virtual void onComplete() = 0;
    
    // Connection closed
    //
    virtual void onClosed() = 0;
    
    // Timeout occurred
    virtual void onTimeout() = 0;
    
    // Transaction was cancelled
    virtual void onCancel() = 0;
    
    // Transaction hit an error
    virtual void onError( const std::string& msg ) = 0;

    virtual ~IListener() {};
};


// A default implementation of IListener which builds up a response object
//
class Listener : public virtual IListener
{
 public:
    Listener() : m_pResponse(new Response) {
    }
    virtual ~Listener() {
    }

    // IListener interface

    virtual void onConnecting() {
    }
    virtual void onConnected() {
    }
    virtual void onRedirect( const bp::url::Url& /*newUrl*/ ) {}
    virtual void onRequestSent() {}
    virtual void onResponseStatus( const Status& status,
                                   const Headers& headers ) {
        m_pResponse->body.clear();
        m_pResponse->status = status;
        m_pResponse->headers = headers;
    }
    virtual void onResponseBodyBytes( const unsigned char* pBytes, 
                                      unsigned int size ) {
        m_pResponse->body.append( pBytes, size );
    }
    virtual void onSendProgress( size_t /*bytesProcessed*/,
                                 size_t /*totalBytes*/,
                                 double /*percent*/ ) {
    }
    virtual void onReceiveProgress( size_t /*bytesProcessed*/,
                                    size_t /*totalBytes*/,
                                    double /*percent*/ ) {
    }
    virtual void onComplete() {
    }
    virtual void onClosed() {
    }
    virtual void onTimeout() {
    }
    virtual void onCancel() {
    }
    virtual void onError( const std::string& /*msg*/ ) {
    }

    ResponsePtr response() const {
        return m_pResponse;
    }

 protected:
    ResponsePtr m_pResponse;

 private:
    // no copy/assign semantics, declared but not defined
    Listener(const Listener&);
    Listener& operator=(const Listener&);
};

} // namespace client
} // namespace http
} // namespace bp


#endif // _HTTPLISTENER_H

