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
 * HttpStressTest.cpp
 * Stresses the snot out of the HTTP client implementation.
 *
 * Created by Lloyd Hilaiel on 03/26/2010.
 */

#include "HttpStressTest.h"
#include <math.h>
#include "bphttp/HttpQueryString.h"
#include "bphttp/HttpSyncTransaction.h"
#include "bphttp/HttpTransaction.h"
#include "BPUtils/bpconvert.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpmd5.h"
#include "BPUtils/bprandom.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/OS.h"

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
#define AMOUNT_OF_CONTENT 4
#define SIZE_OF_CONTENT (1024 * 64)
#define RUNLOOP_THREADS 5
#define TRANS_PER_THREAD 40
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
        // for POST requests we'll calculate the md5 and return it
        // in the response body.
        if (request.method.code() == bp::http::Method::HTTP_POST) {
            // calculate md5..
            std::string md5 = bp::md5::hash(request.body.toString());

            // return md5...
            response.headers.add(bp::http::Headers::ksContentType, "text/plain");
            response.body.append(md5);
        }
        // for GET requests we'll return the content associated with the MD5
        // from the request URI
        else {
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
    std::map<std::string, std::string> * contents;
    std::list<std::tr1::shared_ptr<class StressHttpClient> > clients;
    unsigned short port;
};

void addTransactions(HTRunLoopContext * context);

class StressHttpClient : virtual public bp::http::client::IListener,
                         virtual public std::tr1::enable_shared_from_this<StressHttpClient>
{
public:
    void startTransaction() 
    {
        checkThreadId();
        m_transaction->initiate(shared_from_this());
    }
    
    StressHttpClient(HTRunLoopContext * context) 
        : m_body(), m_context(context), m_tid(0),
          m_gotSendZero(false), m_gotSendHundred(false),
          m_gotReceiveZero(false), m_gotReceiveHundred(false)
    {
        // capture the current thread id for sanity checks later
        m_tid = bp::thread::Thread::currentThreadID();
        
        m_chosenMD5 = (*(m_context->contentMD5s))[bp::random::generate() % AMOUNT_OF_CONTENT];
        m_chosenBody = (*(m_context->contents))[m_chosenMD5];

        // create the url picking one of the available content bodies (md5s)
        std::stringstream urlss;
        urlss << "http://127.0.0.1:" << m_context->port << "/" << m_chosenMD5;

        // lets build up the request.  1/2 the time we'll do a get,
        // 1/2 we'll do a post
        m_isget = (1 == (bp::random::generate() % 2));

        if (m_isget) {
            m_request.reset(new bp::http::Request(bp::http::Method::HTTP_GET, urlss.str()));
            m_transaction.reset(new bp::http::client::Transaction(m_request));
            m_transaction->setTimeoutSec(5.0);
        } else {
            m_request.reset(new bp::http::Request(bp::http::Method::HTTP_POST, urlss.str()));
            m_request->body.append(m_chosenBody);
            m_transaction.reset(new bp::http::client::Transaction(m_request));
            m_transaction->setTimeoutSec(5.0);
        }
    }
    
    virtual ~StressHttpClient() 
    {
        checkThreadId();
    }

    virtual void onResponseBodyBytes(const unsigned char* pBytes, 
                                     unsigned int size) 
    {
        checkThreadId();
        m_body.append(pBytes, size);
    }

    virtual void onClosed() 
    {
        checkThreadId();
        // test response body
        if (m_isget) {
            std::string md5 = bp::md5::hash(m_body.toString());
            die(0 == md5.compare(m_chosenMD5));
        } else {
            die(0 == m_body.toString().compare(m_chosenMD5));
        }
    }
    
    virtual void onTimeout() { checkThreadId(); die(false); }
    virtual void onCancel() { checkThreadId(); die(false); }
    virtual void onError(const std::string&) { checkThreadId(); die(false); }

    void checkThreadId() 
    {
        BPASSERT (m_tid == bp::thread::Thread::currentThreadID() );
    }
    
    virtual void die(bool success) 
    {
        checkThreadId();
        
        // if we didn't get 0 or 100 progress at any point, this
        // test is actually a failure
        if (!m_gotSendZero || !m_gotSendHundred ||
            !m_gotReceiveZero || !m_gotReceiveHundred) 
        {
            success = false;
        }

        if (success) m_context->successes++;
        else m_context->failures++;

        // *delete ourselves*, kinda nuts!!  
        std::list<std::tr1::shared_ptr<class StressHttpClient> >::iterator it;
        HTRunLoopContext * context = m_context;
        for (it = context->clients.begin(); it != context->clients.end(); it++)
        {
            if (it->get() == this) break;
        }
        if (it == context->clients.end()) {
            // FATAL!  we can't find a reference to ourselves.
            abort(); // XXX: we need to fail better here.
        }
        it->reset();  // delete self!
        context->clients.erase(it);

        // allocate more clients as neccesary
        addTransactions(context);

        // if there are no more clients, then we must be done
        if (context->clients.size() == 0) {
            context->rlt->stop();
        }
    }

    virtual void onConnected() { checkThreadId(); }
    virtual void onConnecting() { checkThreadId(); }
    virtual void onComplete() { checkThreadId(); }
    virtual void onRedirect(const bp::url::Url&) { checkThreadId(); }
    virtual void onRequestSent() { checkThreadId(); }
    virtual void onResponseStatus(const bp::http::Status&, const bp::http::Headers&) { checkThreadId(); }
    virtual void onSendProgress(size_t, size_t, double p) {
        checkThreadId();
        if (p == 0.0) {
            if (m_gotSendZero) die(false);
            m_gotSendZero = true;
        } else if (p == 100.0) {
            if (m_gotSendHundred) die(false);
            m_gotSendHundred = true;
        }
    }
    virtual void onReceiveProgress(size_t, size_t, double p) {
        checkThreadId();
        if (p == 0.0) {
            if (m_gotReceiveZero) die(false);
            m_gotReceiveZero = true;
        } else if (p == 100.0) {
            if (m_gotReceiveHundred) die(false);
            m_gotReceiveHundred = true;
        }
    }
    
    bp::http::client::TransactionPtr m_transaction;
    bp::http::RequestPtr m_request;
    bp::http::Body m_body;
    std::string m_chosenMD5;
    std::string m_chosenBody;
    
    // when the test is complete, this class will stop the runloop,
    // returning control to the testcase
    HTRunLoopContext * m_context;
    unsigned int m_tid;

    // if true, we're getting a 100k buffer by md5.  if false, we're
    // posting a 100k buffer
    bool m_isget;

    // flags to test that 0% and 100% are always implemented from the HTTP implementation
    bool m_gotSendZero;
    bool m_gotSendHundred;
    bool m_gotReceiveZero;
    bool m_gotReceiveHundred;
};

void addTransactions(HTRunLoopContext * context) 
{
    while ((context->clients.size() + context->successes + context->failures) < TRANS_PER_THREAD &&
           context->clients.size() < SIMUL_TRANS)
    {
        std::tr1::shared_ptr<StressHttpClient> shc(new StressHttpClient(context));
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
            contexts[i].contents = &content;
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
