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
/**
 * HttpServer.cpp  - An abstraction around an embedded http server
 */

#include "HttpServer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "BPUtils/BPLog.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bptime.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "mongoose/mongoose.h"

using namespace std;
using bp::strutil::safeStr;

namespace bp {
namespace http {
namespace server {


///////////////////////////////////////////////////////////////////////////
// class Transaction::Impl
//
// Note: pimpl idiom
//
class Server::Impl
{
public:
    Impl();
    ~Impl();

   
    bool bind( unsigned short& port );
    bool mount( const string& uriRegex, IHandler* h );
    bool start();
    bool stop();

private:
    enum State { init, created, started, stopped };
    static void * handlerCallback( enum mg_event event,
                                   struct mg_connection *conn,
                                   const struct mg_request_info *request_info );

    State       m_state;
    mg_context* m_pCtx;
	std::map<const std::string, IHandler*> m_callbacks;
	static std::map<const std::string, IHandler*> s_callbacks;
};
std::map<const std::string, IHandler*> Server::Impl::s_callbacks;


///////////////////////////////////////////////////////////////////////////
// Server::Impl Methods
//

Server::Impl::Impl() :
    m_state( init ),
    m_pCtx( 0 )
{
    const char* options[] = {
      "listening_ports", "0",
      NULL
    };
    m_pCtx = mg_create(&Server::Impl::handlerCallback, NULL, options);
    if (!m_pCtx) {
        BPLOG_ERROR( "mg_create failed." );
        return;
    }
    
    m_state = created;
}


Server::Impl::~Impl()
{
    switch (m_state)
    {
        case started:   stop(); // fall through
        case created:   
        case stopped:   if ( m_pCtx != NULL) { mg_destroy( m_pCtx ); m_pCtx = NULL; }
                        break;
        case init:
        default:        break;
    }
}


bool
Server::Impl::bind(unsigned short& port)
{
	if (m_state != created) {
        BPLOG_ERROR_STRM( "Illegal call from state" << m_state );
        return false;
    }

	if (port != 0) {
        BPLOG_ERROR_STRM( "Called with non-ephemeral port " << port );
        //return false;
    }
    port = atoi(mg_get_option( m_pCtx, "listening_ports" ));
    BPLOG_INFO_STRM( "port = " << port );
    
    return true;
}

bool
Server::Impl::mount( const string& uriRegex, IHandler* h )
{
    if (m_state != created) {
        BPLOG_ERROR_STRM( "Illegal call from state" << m_state );
        return false;
    }

	m_callbacks[uriRegex] = h;
    BPLOG_INFO_STRM( h << " mounted for " << uriRegex );
    
    return true;
}


bool
Server::Impl::start()
{
    if (m_state != created) {
        BPLOG_ERROR_STRM( "Illegal call from state" << m_state );
        return false;
    }
    
	// Enable our instance's URI handlers in static callback.
	Server::Impl::s_callbacks.clear();
	for (std::map<const std::string, IHandler*>::const_iterator iter = m_callbacks.begin(); iter != m_callbacks.end(); iter++) {
		Server::Impl::s_callbacks[iter->first] = iter->second;
	}

    if (!mg_start( m_pCtx )) {
        BPLOG_ERROR( "mg_start failed." );
        return false;
    }

    m_state = started;
    return true;
}


bool
Server::Impl::stop()
{
    if (m_state != started)
    {
        BPLOG_ERROR_STRM( "Illegal call from state" << m_state );
        return false;
    }

    mg_stop( m_pCtx );

	// Disable our instance's URI handlers in static callback.
	Server::Impl::s_callbacks.clear();

    m_state = stopped;
    return true;
}


void *
Server::Impl::handlerCallback( enum mg_event event,
                               struct mg_connection *conn,
                               const struct mg_request_info *request_info )
{
    if (event != MG_NEW_REQUEST) {
        return NULL;
    }

	std::map<const std::string, IHandler*>::const_iterator iter = Server::Impl::s_callbacks.find(request_info->uri);
    if (iter == Server::Impl::s_callbacks.end()) {
		iter = Server::Impl::s_callbacks.find("*");
		if (iter == Server::Impl::s_callbacks.end()) {
			return NULL;
		}
    }

    /////////////////////////////////////////////
    // Setup request object for our handler.
    Request req;
    
    req.method = safeStr( request_info->request_method );
    
    req.url.setPath( safeStr( request_info->uri ) );
    req.url.setQuery( safeStr( request_info->query_string ) );

    stringstream ss;
    ss << "HTTP/" << request_info->http_version;
    req.version = ss.str();
    
    BPLOG_DEBUG_STRM( req.method.toString() << " " <<
                      req.url.toString() << " " <<
                      req.version.toString() );

    for (int i=0; i < request_info->num_headers; ++i)
    {
        req.headers.add( safeStr( request_info->http_headers[i].name ),
                         safeStr( request_info->http_headers[i].value ) );
    }

    const char *cl;
    if (strcmp(request_info->request_method, "POST") == 0 &&
        (cl = mg_get_header(conn, "Content-Length")) != NULL) {
        int len = atoi(cl);
        char *buf = (char*)malloc(len);
        if (buf != NULL) {
            mg_read(conn, buf, len);
            //mg_write(conn, buf, len);
            req.body.assign( (unsigned char*)buf, len );
            free(buf);
        }
    }

    /////////////////////////////////
    // Call our mounted handler.
    IHandler* hndlr = iter->second;
    Response resp;
    bool bRet = hndlr->processRequest( req, resp );
    if (!bRet) {
        BPLOG_ERROR( "handler processRequest failed." );
        BPLOG_ERROR( "Returning 500 Internal Error" );
        mg_printf( conn, "HTTP/1.0 500 Internal Error\r\n");
    }

    //////////////////////////
    // Send the response.
    mg_printf( conn, "%s %s\r\n",
               resp.version.toString().c_str(),resp.status.toString().c_str() );

    // Send all headers provided by the handler.
    mg_printf( conn, "%s", resp.headers.toString().c_str() );

    // Add headers we always send.
    ss.str(""); // reset
    ss << "Content-Length: " << resp.body.size() << "\r\n";
    ss << "Server: BrowserPlus embedded webserver" << "\r\n";
    mg_printf( conn, "%s", ss.str().c_str() );
    
    // Add end-of-headers separator.
    mg_printf( conn, "\r\n" );

    // TODO: support IHandler::shapePacket?

    // Send body.
    int nBodySize = (int) resp.body.size();
    if (nBodySize) {
        int nSent = mg_write( conn, resp.body.elementAddr(0), nBodySize );
        if (nSent != nBodySize) {
            BPLOG_ERROR_STRM( "mg_write sent " << nSent << " of " <<
                              nBodySize << " bytes." );
        }
    }

    BPLOG_DEBUG( "Response complete." );
    return conn;
}


///////////////////////////////////////////////////////////////////////////
// Server methods
//
// We're using pimpl idiom, so these are just forwarders.
//

Server::Server() :
    m_pImpl(new Impl())
{

}


Server::~Server()
{
    
}


bool
Server::bind( unsigned short& port )
{
    return m_pImpl->bind( port );
}


bool
Server::mount( const string& uriRegex, IHandler* h )
{
    return m_pImpl->mount( uriRegex, h );
}


bool
Server::start()
{
    return m_pImpl->start();
}


bool
Server::stop()
{
    return m_pImpl->stop();
}


} // server
} // http
} // bp

