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

/*
 *  HttpTransaction_Windows.cpp
 *
 *  Implements the Transaction class and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HttpTransaction.h"
#include "BPLog.h"
#include "bpconvert.h"
#include "bpthreadhopper.h"
#include "bpfile.h"
#include "bprunloop.h"
#include "bptimer.h"
#include "HttpListener.h"

#include <map>

using namespace std;

#warning "http client not implemented on linux!"

using bp::error::lastErrorString;

namespace bp {
namespace http {
namespace client {

class Transaction::Impl
{
public:
    Impl() { }
    ~Impl() { }
};



Transaction::Transaction(RequestPtr ptrRequest) :
    m_pImpl(new Impl())
{
    BPLOG_DEBUG_STRM("transaction to " <<  ptrRequest->url.toString());
}


Transaction::~Transaction()
{
}


double
Transaction::defaultTimeoutSecs()
{
    // XXX
    return 0.0;
}

        
void
Transaction::initiate(IListener* pListener)
{
    if (pListener == NULL) {
        BP_THROW_FATAL("null listener");
    }
    // XXX
}


void
Transaction::cancel()
{
    // XXX
}



RequestPtr
Transaction::request() const
{
    // XXX
    return RequestPtr();
}


const std::string&
Transaction::userAgent() const
{
    // XXX
    return std::string();
}


void
Transaction::setUserAgent(const std::string& sUserAgent)
{
    // XXX
}


double
Transaction::timeoutSec() const
{
    // XXX
}

void
Transaction::setTimeoutSec(double fSecs)
{
    // XXX
}


} // namespace client
} // namespace http
} // namespace bp
