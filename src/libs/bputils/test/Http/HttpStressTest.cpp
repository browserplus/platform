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
 * HttpStressTest.cpp
 * Stresses the snot out of the HTTP client implementation.
 *
 * Created by Lloyd Hilaiel on 03/26/2010.
 */

#include "HttpStressTest.h"
#include <math.h>
#include "BPUtils/bpconvert.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/HttpQueryString.h"
#include "BPUtils/HttpSyncTransaction.h"
#include "BPUtils/HttpTransaction.h"
#include "BPUtils/OS.h"
#include "BPUtils/bprandom.h"
#include "BPUtils/bpmd5.h"

CPPUNIT_TEST_SUITE_REGISTRATION(HttpStressTest);

// Test overview:
// * ten pieces of content are generated, they're random data referenced by
//   a url that includes their md5
// * a server is allocated on the main thread that will serve this content
// * X runloop threads are spun up 
// * each runloop thread runs a total of Y requests before exiting with
//   Z concurrent HTTP transactions
// * all runloop threads collect statistics
// * upon completion all runloop threads are joined and we tally up how many
//   transactions were completed successfully

// parameters of the test
#define AMOUNT_OF_CONTENT 10
#define SIZE_OF_CONTENT (1024 * 100)
#define RUNLOOP_THREADS 5
#define TRANS_PER_THREAD 50
#define SIMUL_TRANS 5

class HttpStressHandler : public bp::http::server::IHandler 
{
public:
    HttpStressHandler(std::map<std::string, std::string> * content) 
        : m_content(content)
    {
    }

private:
    virtual bool processRequest(const bp::http::Request & request,
                                bp::http::Response & response)
    {
        std::string path = request.url.path().substr(1);        
        std::map<std::string, std::string>::const_iterator i;
        i = m_content->find(path);
        if (i == m_content->end()) {
            response.status.setCode(bp::http::Status::NOT_FOUND);
            response.body.append("Hey dude.  I can't find what yer lookin' for.");
            response.headers.add(bp::http::Headers::ksContentType, "text/plain");
        } else {
            response.headers.add(bp::http::Headers::ksContentType, "text/plain");
            response.body.append(i->second);
        }
        return true;
    }
    
    std::map<std::string, std::string> * m_content;
};

//////////////////////////////////////////////////////////////////////
// client implementation
struct HTRunLoopContext {
    bp::runloop::RunLoopThread * rlt;
    unsigned int successes;
    unsigned int failures;
    std::vector<std::string> * contentMD5s;    
    std::list<class StressHttpClient *> clients;
    unsigned short port;
};

void addTransactions(HTRunLoopContext * context);

class StressHttpClient : virtual public bp::http::client::IListener
{
public:
    void startTransaction() 
    {
        m_transaction->initiate(this);
    }
    
    StressHttpClient(HTRunLoopContext * context) 
        : m_body(), m_context(context)
    {
        m_chosenMD5 = (*(m_context->contentMD5s))[bp::random::generate() % AMOUNT_OF_CONTENT];
        // create the url picking one of the available content bodies (md5s)
        std::stringstream urlss;
        urlss << "http://127.0.0.1:" << m_context->port << "/" << m_chosenMD5;

        // lets build up the request
        m_request.reset(new bp::http::Request(bp::http::Method::HTTP_GET, urlss.str()));
        m_transaction = new bp::http::client::Transaction(m_request);
        m_transaction->setTimeoutSec(3.0);
    }
    
    virtual ~StressHttpClient() 
    {
        delete m_transaction;
    }

    virtual void onResponseBodyBytes(const unsigned char* pBytes, 
                                     unsigned int size) 
    {
        m_body.append(pBytes, size);
    }

    virtual void onClosed() 
    {
        // test response body
        std::string md5 = bp::md5::hash(m_body.toString());
        die(0 == md5.compare(m_chosenMD5));
    }
    
    virtual void onTimeout() { die(false); }
    virtual void onCancel() { die(false); }
    virtual void onError(const std::string&) { die(false); }
    
    virtual void die(bool success) 
    {
        if (success) m_context->successes++;
        else m_context->failures++;

        // *delete ourselves*, kinda nuts!!  
        std::list<class StressHttpClient *>::iterator it;
        HTRunLoopContext * context = m_context;
        for (it = context->clients.begin(); it != context->clients.end(); it++)
        {
            if (*it == this) break;
        }
        if (it == context->clients.end()) {
            // FATAL!  we can't find a reference to ourselves.
            abort(); // XXX: we need to fail better here.
        } 
        delete *it; // delete self!
        context->clients.erase(it);

        // allocate more clients as neccesary
        addTransactions(context);

        // if there are no more clients, then we must be done
        if (context->clients.size() == 0) {
            context->rlt->stop();
        }
    }

    // stupid I have to manually define these
    virtual void onConnected() { }
    virtual void onConnecting() { }
    virtual void onComplete() { }
    virtual void onRedirect(const bp::url::Url&) { }
    virtual void onRequestSent() { }
    virtual void onResponseStatus(const bp::http::Status&, const bp::http::Headers&) { }
    virtual void onSendProgress(size_t, size_t, double) { }
    virtual void onReceiveProgress(size_t, size_t, double) { }
    
    bp::http::client::Transaction* m_transaction;
    bp::http::RequestPtr m_request;
    bp::http::Body m_body;
    std::string m_chosenMD5;
    
    // when the test is complete, this class will stop the runloop,
    // returning control to the testcase
    HTRunLoopContext * m_context;
};

void addTransactions(HTRunLoopContext * context) 
{
    while ((context->clients.size() + context->successes + context->failures) < TRANS_PER_THREAD &&
           context->clients.size() < SIMUL_TRANS)
    {
        StressHttpClient * shc = new StressHttpClient(context);
        context->clients.push_back(shc);
        shc->startTransaction();
    }
}

void
runLoopStart(void * cookie, bp::runloop::Event e)
{
    HTRunLoopContext * context = (HTRunLoopContext *) e.payload();
    // allocate the number of simul clients
    addTransactions(context);
}

void
runLoopEnd(void * cookie)
{
}

//////////////////////////////////////////////////////////////////////    

void HttpStressTest::beatTheSnotOutOfIt()
{
    // first let's generate random content key'd by its md5 value
    std::map<std::string, std::string> content;
    std::vector<std::string> contentMD5s;

    for (unsigned int i = 0; i < AMOUNT_OF_CONTENT; i++) {
        std::string c;
        while (c.length() < SIZE_OF_CONTENT) {
            char ch = (bp::random::generate() % 26) + 'a';
            c.push_back(ch);
        }
        // now md5
        std::string md5 = bp::md5::hash(c);
        
        // and add
        contentMD5s.push_back(md5);
        content[md5] = c;
    }

    // now we need a lil' webserver that will serve this conent
    {
        bp::http::server::Server server;
        HttpStressHandler handler(&content);
        
        unsigned short port = 0;
        CPPUNIT_ASSERT( server.bind(port) );

        // now mount the little handler that serves content.
        CPPUNIT_ASSERT( server.mount(std::string("*"), &handler) );
        
        // start our webserver
        CPPUNIT_ASSERT( server.start() );

        // spawn the appropriate number of runloop threads here
        bp::runloop::RunLoopThread runLoopThreads[RUNLOOP_THREADS];
        HTRunLoopContext contexts[RUNLOOP_THREADS];

        // spawn all of our runloops that themselves will run some
        // number of HTTP clients
        for (unsigned int i = 0; i < RUNLOOP_THREADS; i++) {
            contexts[i].rlt = runLoopThreads + i;
            contexts[i].successes = contexts[i].failures = 0;
            contexts[i].contentMD5s = &contentMD5s;
            contexts[i].port = port;
            runLoopThreads[i].setCallBacks(NULL, NULL, 
                                           runLoopEnd, contexts + i,
                                           runLoopStart, contexts + i);
            CPPUNIT_ASSERT( runLoopThreads[i].run() );

            CPPUNIT_ASSERT( runLoopThreads[i].sendEvent( bp::runloop::Event(contexts + i) ) );
        }

        // now wait for all the runloops to complete
        for (unsigned int i = 0; i < RUNLOOP_THREADS; i++) {
            CPPUNIT_ASSERT( runLoopThreads[i].join() );
        }
        
        // stop our webserver
        CPPUNIT_ASSERT( server.stop() );        

        // validate the data in runloop contexts
        for (unsigned int i = 0; i < RUNLOOP_THREADS; i++) {
            CPPUNIT_ASSERT_EQUAL( (unsigned int) TRANS_PER_THREAD, contexts[i].successes );
            CPPUNIT_ASSERT_EQUAL( (unsigned int) 0, contexts[i].failures );            
        }
    }
}
