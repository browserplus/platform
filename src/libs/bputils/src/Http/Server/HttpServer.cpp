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
 * HttpServer.cpp  - An abstraction around an embedded http server
 */

#include "HttpServer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "BPLog.h"
#include "bpstopwatch.h"
#include "bptime.h"
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
    static void handlerCallback( struct mg_connection*,
                                 const struct mg_request_info* info,
                                 void* user_data );

    State       m_state;
    mg_context* m_pCtx;
};



///////////////////////////////////////////////////////////////////////////
// Server::Impl Methods
//

Server::Impl::Impl() :
    m_state( init ),
    m_pCtx( 0 )
{
    m_pCtx = mg_create();
    if (!m_pCtx) {
        BPLOG_ERROR( "mg_create failed." );
        return;
    }
    
    m_state = created;

    if (mg_set_option( m_pCtx, "idle_time", "0" ) != 1) {
        BPLOG_ERROR( "Unable to set idle time." );
        return;
    }
}


Server::Impl::~Impl()
{
    switch (m_state)
    {
        case started:   stop(); // fall through
        case created:   
        case stopped:   mg_destroy( m_pCtx );
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

    stringstream ssPort;
    ssPort << port;
    int nRet = mg_set_option( m_pCtx, "ports", ssPort.str().c_str() );
    if (!nRet) {
        BPLOG_ERROR( "mg_set_option failed." );
        return false;
    }
        
    port = (unsigned short) nRet;
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

    mg_set_uri_callback( m_pCtx, uriRegex.c_str(), handlerCallback, (void*)h );
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

    m_state = stopped;
    return true;
}


void
Server::Impl::handlerCallback( struct mg_connection* conn,
                               const struct mg_request_info* req_info,
                               void* user_data )
{
    /////////////////////////////////////////////
    // Setup request object for our handler.
    Request req;
    
    req.method = safeStr( req_info->request_method );
    
    req.url.setPath( safeStr( req_info->uri ) );
    req.url.setQuery( safeStr( req_info->query_string ) );

    stringstream ss;
    ss << "HTTP/"
       << req_info->http_version_major << "."
       << req_info->http_version_minor;
    req.version = ss.str();
    
    BPLOG_DEBUG_STRM( req.method.toString() << " " <<
                      req.url.toString() << " " <<
                      req.version.toString() );

    for (int i=0; i < req_info->num_headers; ++i)
    {
        req.headers.add( safeStr( req_info->http_headers[i].name ),
                         safeStr( req_info->http_headers[i].value ) );
    }

    if (req_info->post_data_len > 0) {
        req.body.assign( (unsigned char*)req_info->post_data,
                         req_info->post_data_len );
    }


    /////////////////////////////////
    // Call our mounted handler.
    IHandler* hndlr = (IHandler*) user_data;
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

