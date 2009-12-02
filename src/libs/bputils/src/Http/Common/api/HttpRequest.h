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
 *  HttpRequest.h
 *
 *  Declares HttpRequest and related items.
 *
 *  Created by David Grigsby on 7/16/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPREQUEST_H_
#define _HTTPREQUEST_H_

#include "bptr1.h"
#include "bpurl.h"
#include "HttpBody.h"
#include "HttpHeaders.h"
#include "HttpListener.h"
#include "HttpMethod.h"
#include "HttpVersion.h"


namespace bp {
namespace http {


struct Request
{
    Request( const Method& method_ = Method(),
             const bp::url::Url& url_ = bp::url::Url(),
             const Version& version_ = Version(),
             const Headers& headers_ = Headers(),
             const Body& body_ = Body() ) :
        method( method_ ),
        url( url_ ),
        version( version_ ),
        headers( headers_ ),
        body( body_ )
    {}
    
    Method          method;
    bp::url::Url    url;
    Version         version;
    Headers         headers;
    Body            body;
}; // Request

typedef std::tr1::shared_ptr<Request> RequestPtr;


} // namespace http
} // namespace bp


#endif // _HTTPREQUEST_H

