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
 * HttpServerTest.cpp
 *
 * Created by Lloyd Hilaiel on 7/17/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "HttpServerTest.h"
#include "BPUtils/HttpServer.h"


CPPUNIT_TEST_SUITE_REGISTRATION(HttpServerTest);

void HttpServerTest::startupShutdownTest()
{
    unsigned short int port = 0;
    
    // first let's run and stop a server
    {
        bp::http::server::Server s;
        CPPUNIT_ASSERT(s.bind(port));
        s.start();
        s.stop();
    }

    // now let's try to bind that same port again
    // testing REUSEPORT and correct shutdown
    {
        bp::http::server::Server s;
        CPPUNIT_ASSERT(s.bind(port));
        s.start();
        s.stop();
    }

    // ensure that shutdown happens automatically in the destructor
    // (crash otherwise) 
    {
        bp::http::server::Server s;
        CPPUNIT_ASSERT(s.bind(port));
        s.start();
    }
}

void HttpServerTest::bindingTest()
{

#if 0
// Windows assumes that REUSEADDR implies REUSEPORT and doesn't fail
// when you try to bind a port with REUSEADDR set on it.  
// because the behavior is different, we cannot include this test.
    
    // try double binding a port
    unsigned short int port = 0;    

    // first let's run and stop a server
    {
        bp::http::server::Server s;
        CPPUNIT_ASSERT(s.bind(port));
        s.start();

        bp::http::server::Server s1;
        CPPUNIT_ASSERT(!s1.bind(port));
    }
#endif 

}
