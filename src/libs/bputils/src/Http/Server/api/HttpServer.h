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
/**
 * HttpServer.h  - An abstraction around an embedded http server
 */

#ifndef HTTP_SERVER_H__
#define HTTP_SERVER_H__

#include "BPUtils/bpsync.h"
#include "BPUtils/bpthread.h"
#include "BPUtils/HttpHandler.h"

namespace bp { namespace http { namespace server {


class Server
{
public:
    Server();
    ~Server();

    /** bind the HTTP server to a specified port.
     *  (the server will always bind to localhost).
     *  port - input is the port to bind to.  If zero, an ephemeral
     *         port will be bound, and the port will be stored in
     *         &port.
     *  returns true on success, false on failure
     */
    bool bind( unsigned short& port);

    /** mount a handler to service incoming requests.  client owns handler
     *  and must ensure that it is not deallocated until after stop()
     *  is called. */    
    bool mount( const std::string& uriRegex, IHandler* h );

    /** spawn a thread to run the http server.  non-blocking. */
    bool start();

    /** stop the http server.  by the time we return all resources
     *  shall have been freed, and the spun thread shutdown.
     *  if stop() is not called, cleanup will occur in the Server
     *  destructor */
    bool stop();

private:
    class Impl;
    std::auto_ptr<Impl> m_pImpl;

    Server( const Server& );
    Server& operator=( const Server& );
};


} } } // bp::http::server

#endif    
