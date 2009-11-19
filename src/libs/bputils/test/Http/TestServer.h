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

/**
 * TestServer.h
 * A HTTP server with associated handlers for testing.
 *
 * Created by Lloyd Hilaiel on 7/18/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __HTTPTESTSERVER_H__
#define __HTTPTESTSERVER_H__

#include "TestingFramework/TestingFramework.h"
#include "BPUtils/HttpResponse.h"
#include "BPUtils/HttpServer.h"

class TestServer
{
  public:
    TestServer();
    ~TestServer();    

    /* run the test server, a noop if already called */
    void run();

    /* stop the test server, noop if not running. */
    void stop();
    
    /* after running, calling this function will return a localhost
     * URL that will return a small response body of content-type
     * "text/plain".  The output response parameter may be queried
     * for the data that should be returned from the URL. */
    std::string simpleTransaction(const bp::http::Response * &response);

    /* similar to simpleTransaction.
     * a url that you may hit to get a 404 response with a small
     * "text/plain" response body. */ 
    std::string notFoundTransaction(const bp::http::Response * &response);

    /* Returns a 302 redirect that will redirect the client to
     * simpleTransaction */ 
    std::string redirectTransaction(const bp::http::Response * &response);

    /* similar to simpleTransaction.
     * a url that you may hit to get a "image/jpeg" binary response.
     * the output 'response' parameter contains the full body for a
     * byte by byte comparison */ 
    std::string binaryTransaction(const bp::http::Response * &response);

    /* Returs a url to which you may do an http POST operation.  The
     * response body will be the same as the request body.
     * Certain query string parameters will modify the behavior:
     *     DelaySec     The response will be delayed the specified
     *                  number of seconds.
     *     EchoHeaders  Echo the request headers, no the request body,
     *                  in the response body.              
     */
    std::string getEchoUrl();

    std::string getSimpleUrl();

    /* Returs a url fromo which you may do an http GET operation.
     * The response body is currently always all 0's.
     * Certain query string parameters will modify the response behavior:
     *     respLenKB        Sets the response length in KB.
     *     packetDelaySec   Sets a delay in sec before each response packet.
     */
    std::string getShapingUrl();
    
private:
    /* The various responses we deliver, populated at class construction,
     * and unchanged thereafter, mitigating threading issues. */
    bp::http::Response m_simpleResponse;
    bp::http::Response m_notFoundResponse;
    bp::http::Response m_binaryResponse;
    bp::http::Response m_redirectResponse;

    /* our webserver */
    bp::http::server::Server m_server;

    /* the ephemeral port upon which we're running, zero means the
     * server hasn't been started yet */
    unsigned short int m_port;

    /* forward declarations of our IHandler that deliver the responses */    
    class TestServerHandler * m_handler;
    friend class TestServerHandler;

    class EchoHandler * m_echoHandler;
    friend class EchoHandler;

    class ShapingHandler * m_shapingHandler;
    friend class ShapingHandler;
    
    // utility routine to build a url from a path and m_port
    std::string getMyURL(const char * path);
};

#endif
