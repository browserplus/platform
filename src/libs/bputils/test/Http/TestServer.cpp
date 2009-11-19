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
 * TestServer.cpp
 * A HTTP server with associated handlers for testing.
 *
 * Created by Lloyd Hilaiel on 7/18/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "TestServer.h"
#include <assert.h>
#include <iostream>
#ifdef WIN32
#include <Windows.h>
#endif
#include "BPUtils/bpconvert.h"
#include "BPUtils/HttpQueryString.h"
#include "TestImageData.h"


using namespace bp::http;
using namespace std;

#define SIMPLE_PATH     "/simple"
#define NOTFOUND_PATH   "/this/path/dont/exist"
#define BINARY_PATH     "/gimme/an/image"
#define ECHO_PATH       "/echo" 
#define REDIRECT_PATH   "/redirect"
#define SHAPING_PATH    "/shapetest"


//////////////////////////////////////////////////////////////////////
// TestServerHandler
// Currently handles all paths except "echo", which has its own
// handler.
//
class TestServerHandler : public bp::http::server::IHandler
{
public:
    TestServerHandler(TestServer * ts) : m_ts(ts) {  }
    ~TestServerHandler() { }    
    
    bool processRequest(const bp::http::Request & request,
                        bp::http::Response & response)
    {
        std::string path = request.url.path();
        if (!path.compare(SIMPLE_PATH)) {
            response = m_ts->m_simpleResponse;
        } else if (!path.compare(BINARY_PATH)) {
            response = m_ts->m_binaryResponse;
        } else if (!path.compare(REDIRECT_PATH)) {
            response.status.setCode(Status::FOUND);    
            response.headers.add(Headers::ksLocation, m_ts->getSimpleUrl());
        } else {
            response = m_ts->m_notFoundResponse;            
        }
        return true;
    }
private:
    TestServer * m_ts;
};



//////////////////////////////////////////////////////////////////////
// EchoHandler
//
class EchoHandler : public bp::http::server::IHandler
{
public:
    EchoHandler() {}
    ~EchoHandler() {}    

    bool processRequest(const bp::http::Request& request,
                        bp::http::Response& response);
};


bool EchoHandler::processRequest(const bp::http::Request& request,
                                 bp::http::Response& response)
{
    try
    {
        // If there is a "DelaySec" field in the query string,
        // delay that many seconds.
        QueryString qs( request.url.query() );
        std::string sDelaySec;
        if (qs.find( "DelaySec", sDelaySec ))
        {
#ifdef WIN32
            double fDelaySec = bp::conv::lexical_cast<double>( sDelaySec );
            Sleep( int( fDelaySec * 1000 ) );
#else
            unsigned int delay = bp::conv::lexical_cast<unsigned int>( sDelaySec );
            sleep( delay );
#endif
        }

        string sDummy;
        if (qs.find( "EchoHeaders", sDummy ))
        {
            // Echo the request headers in the response body.
            response.body.assign( request.headers.toString() );
        }
        else
        {
            // Echo the request body in the response body.
            response.body = request.body;
        }
        return true;
    }
    catch( std::exception& exc )
    {
        cout << bp::error::makeCatchReportString( exc );
        return false;
    }
}


//////////////////////////////////////////////////////////////////////
// ShapingHandler
// This handler currently returns very boring (all 0's) responses,
// but the response size and aspects of the response packets can be
// configured by the http client, so that the client can test its
// response to various network performance scenarios.

class ShapingHandler : public bp::http::server::IHandler
{
public:
    ShapingHandler() {}
    ~ShapingHandler() {}    

    bool processRequest(const bp::http::Request& request,
                        bp::http::Response& response);

    bool shapePacket(unsigned int packetsSent,
                     unsigned int bytesSent,
                     double elapsedSec,
                     unsigned int& packetBytes,
                     double& packetDelaySec);
    
private:
    bool    m_bShapePackets;
    double  m_fPacketDelaySec;
    int     m_nMaxRateKBs;
};


bool ShapingHandler::processRequest(const bp::http::Request& request,
                                    bp::http::Response& response)
{
    QueryString qs( request.url.query() );
    int nLenKB = 0;
    string s;
    if (qs.find( "respLenKB", s )) {
        nLenKB = bp::conv::lexical_cast<int>( s );
    }

    m_bShapePackets = false;
    if (qs.find( "packetDelaySec", s )) {
        m_bShapePackets = true;
        m_fPacketDelaySec = bp::conv::lexical_cast<double>( s );
    }

    if (qs.find( "maxRateKBs", s )) {
        m_bShapePackets = true;
        m_nMaxRateKBs = bp::conv::lexical_cast<int>( s );
    }

    vector<unsigned char> v( nLenKB*1000 );
    response.body.assign( v.begin(), v.end() );

    return true;
}


bool ShapingHandler::shapePacket(unsigned int packetsSent,
                                 unsigned int bytesSent,
                                 double elapsedSec,
                                 unsigned int& packetBytes,
                                 double& packetDelaySec)
{
    if (m_bShapePackets)
    {
        // TODO: set packetBytes to support maxRateKBs.

        if (m_fPacketDelaySec > 0) {
            packetDelaySec = m_fPacketDelaySec;
        }

        return true;
    }
    else
    {
        return false;
    }
}



//////////////////////////////////////////////////////////////////////
// TestServer
//

TestServer::TestServer() : m_port(0)
{
    // allocate our handlers
    m_handler = new TestServerHandler(this);
    m_echoHandler = new EchoHandler();
    m_shapingHandler = new ShapingHandler();
    
    // populate the various responses
    m_notFoundResponse.status.setCode(Status::NOT_FOUND);
    m_notFoundResponse.body.append("Hey dude.  "
                                   "I can't find what yer lookin' for.");
    m_notFoundResponse.headers.add(Headers::ksContentType,
                                   "text/plain");

    m_simpleResponse.body.append("hello world!\n");
    m_simpleResponse.headers.add(Headers::ksContentType,
                                 "text/plain");

    m_binaryResponse.headers.add(Headers::ksContentType,
                                 "image/jpeg");
    m_binaryResponse.body.assign(TestSampleImage, sizeof(TestSampleImage));

    // mount our handlers
    bool mounted = m_server.mount(ECHO_PATH, m_echoHandler);
    assert(mounted);
    mounted = m_server.mount(SHAPING_PATH, m_shapingHandler);
    assert(mounted);
    mounted = m_server.mount("*", m_handler);
    assert(mounted);
    
    mounted = false; // fix compiler warning in -DNDEBUG
}

TestServer::~TestServer()
{
    // stop the server so we don't delete our handler out from
    // underneath it
    if (m_port) m_server.stop();
    m_port = 0;

    delete m_handler;
    m_handler = NULL;

    delete m_echoHandler;
    m_echoHandler = NULL;

    delete m_shapingHandler;
    m_shapingHandler = NULL;
}

void
TestServer::run()
{
    if (!m_port) {
        bool bound = m_server.bind(m_port);
        assert(bound);
        assert(m_port != 0);
        bound = false; // fix compiler warning in -DNDEBUG

        bool started = m_server.start();
        assert(started);
        started = false; // fix compiler warning in -DNDEBUG
    }
}

void
TestServer::stop()
{
    if (m_port) {
        m_server.stop();
    }
    m_port = 0;
}

std::string
TestServer::getMyURL(const char * path)
{
    std::stringstream url;

    // Use 127.0.0.1 instead of localhost.  The latter has poor
    // performance on win7, evidently due to IPv6 issues.
    // (YIB-2875425)
//  url << "http://localhost:" << m_port << path;
    url << "http://127.0.0.1:" << m_port << path;
    
    return url.str();
}

std::string
TestServer::simpleTransaction(const bp::http::Response * &response)
{
    response = &m_simpleResponse;
    return getMyURL(SIMPLE_PATH);
}

std::string
TestServer::notFoundTransaction(const bp::http::Response * &response)
{
    response = &m_notFoundResponse;
    return getMyURL(NOTFOUND_PATH);
}

std::string
TestServer::binaryTransaction(const bp::http::Response * &response)
{
    response = &m_binaryResponse;
    return getMyURL(BINARY_PATH);
}

std::string
TestServer::redirectTransaction(const bp::http::Response * &response)
{
    response = &m_simpleResponse;
    return getMyURL(REDIRECT_PATH);
}

std::string
TestServer::getEchoUrl()
{
    return getMyURL(ECHO_PATH);
}

std::string
TestServer::getSimpleUrl()
{
    return getMyURL(SIMPLE_PATH);
}

std::string
TestServer::getShapingUrl()
{
    return getMyURL(SHAPING_PATH);
}
