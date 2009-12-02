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
 *  HttpResponse.h
 *
 *  Declares HttpResponse class and related items.
 *
 *  Created by David Grigsby on 7/15/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _HTTPRESPONSE_H_
#define _HTTPRESPONSE_H_

#include "bptr1.h"
#include "HttpBody.h"
#include "HttpHeaders.h"
#include "HttpStatus.h"
#include "HttpVersion.h"


namespace bp {
namespace http {


struct Response
{
    Version version;
    Status  status;
    Headers headers;
    Body    body;
};

typedef std::tr1::shared_ptr<Response> ResponsePtr;


} // namespace http
} // namespace bp


#endif // _HTTPRESPONSE_H

