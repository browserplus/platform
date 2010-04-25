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
 * HttpClientTest.cpp
 * Tests of the basic functionality of the BPUtils HTTP client
 *
 * Created by Lloyd Hilaiel on 7/18/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "HttpClientTest.h"
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

using namespace std;
using namespace bp;
using namespace bp::conv;
using namespace bp::http;
using namespace bp::http::client;

CPPUNIT_TEST_SUITE_REGISTRATION(HttpClientTest);


void HttpClientTest::setUp()
{
    m_testServer.run();
}


void HttpClientTest::tearDown()
{
    m_testServer.stop();
}


// Test Ideas
// * Verify Request Version and Headers are sent
// * Verify Response Version and Headers
// * Simultaneous transactions
// * Sending a request before the entire request body is available.
// * Extracting from a response before the entire response body is
//   available (use case: playing music before track download complete).
// * Cancel


// A helper class for async operations.  Runs in a separate RunLoopThread
//
class AsyncHttp : virtual public IListener
{
public:
    
    void startTransaction() {
        m_transaction->initiate(this);
    }
    
    AsyncHttp(RequestPtr request, bp::runloop::RunLoop *rl) 
        : m_request(request), m_connecting(false), m_connected(false),
          m_redirectUrl(), m_requestSent(false), m_status(), m_headers(), 
          m_body(), m_complete(false), m_closed(false),
          m_percentSent(0.0), m_percentReceived(0.0), m_timedOut(false),
          m_cancelled(false), m_errorMsg(), m_rl(rl)
    {
        m_transaction = new Transaction(m_request);
    }
    
    virtual ~AsyncHttp() {
        delete m_transaction;
    }

    void setTimeoutSec(double fSecs) {
        m_transaction->setTimeoutSec(fSecs);
    }
    
    // -----------------  IListener callbacks
    
    virtual void onConnecting() {
        m_connecting = true;
    }
    
    virtual void onConnected() {
        m_connected = true;
    } 
    
    virtual void onRedirect(const bp::url::Url& newUrl) {
        m_redirectUrl = newUrl;
    }
    
    virtual void onRequestSent() {
        m_requestSent = true;
    }
    
    virtual void onResponseStatus(const Status& status,
                                  const Headers& headers) {
        m_status = status;
        m_headers = headers;
    }
    
    virtual void onResponseBodyBytes(const unsigned char* pBytes, 
                                     unsigned int size) {
        m_body.append(pBytes, size);
    }
    
    virtual void onSendProgress(size_t /*bytesSent*/,
                                size_t /*totalBytes*/,
                                double percent) {
        m_percentSent = percent;
    }
    
    virtual void onReceiveProgress(size_t /*bytesReceived*/,
                                   size_t /*totalBytes*/,
                                   double percent) {
        m_percentReceived = percent;
    }
    
    virtual void onComplete() {
        m_complete = true;
    }
    
    virtual void onClosed() {
        die();
    }
    
    virtual void onTimeout() {
        m_timedOut = true;
        die();
    }
    
    virtual void onCancel() {
        m_cancelled = true;
        die();
    }
    
    virtual void onError(const std::string& msg) {
        m_errorMsg = msg;
        die();
    }
    
    virtual void die() {
        m_closed = true;
        m_rl->stop();
    }
    
    Transaction* m_transaction;
    RequestPtr m_request;
    bool m_connecting;
    bool m_connected;
    bp::url::Url m_redirectUrl;
    bool m_requestSent;
    Status m_status;
    Headers m_headers;
    Body m_body;
    bool m_complete;
    bool m_closed;
    double m_percentSent;
    double m_percentReceived;
    bool m_timedOut;
    bool m_cancelled;
    std::string m_errorMsg;
    
    bool ok() const {
        return !m_timedOut && !m_cancelled && m_errorMsg.empty();
    }
    
    // when the test is complete, this class will stop the runloop,
    // returning control to the testcase
    bp::runloop::RunLoop * m_rl;
};


void HttpClientTest::testTextGet()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("textGet.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);
    
    const Response* prespExpected;
    string sUrl = m_testServer.simpleTransaction(prespExpected);
    string sExpBody = prespExpected->body.toString();
    
    SyncTransaction tran(RequestPtr(new Request(Method::HTTP_GET, sUrl)));
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->version.isHttp10() || ptrResp->version.isHttp11());
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    
    string sContentLength = ptrResp->headers.get(Headers::ksContentLength);
    CPPUNIT_ASSERT(atoi(sContentLength.c_str()) == (int) ptrResp->body.size());
    
    string sRcvdBody = ptrResp->body.toString();
    CPPUNIT_ASSERT(sRcvdBody.length() > 0);
    CPPUNIT_ASSERT(sRcvdBody == sExpBody);
}

void HttpClientTest::testRedirect()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("redirect.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);
    
    const Response* prespExpected;
    string sUrl = m_testServer.redirectTransaction(prespExpected);
    string sExpBody = prespExpected->body.toString();
    
    SyncTransaction tran(RequestPtr(new Request(Method::HTTP_GET, sUrl)));
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->version.isHttp10() || ptrResp->version.isHttp11());
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    
    string sContentLength = ptrResp->headers.get(Headers::ksContentLength);
    CPPUNIT_ASSERT(atoi(sContentLength.c_str()) == (int) ptrResp->body.size());
    
    string sRcvdBody = ptrResp->body.toString();
    CPPUNIT_ASSERT(sRcvdBody.length() > 0);
    CPPUNIT_ASSERT(sRcvdBody == sExpBody);
}


void HttpClientTest::testTextGetAsync()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("textGetAsync.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const Response* prespExpected;
    string sUrl = m_testServer.simpleTransaction(prespExpected);
    string sExpBody = prespExpected->body.toString();
    
    // allocate a runloop thread and initialize it on this thread of
    // execution
    bp::runloop::RunLoop rl;
    rl.init();
    
    RequestPtr request(new Request(Method::HTTP_GET, sUrl));
    AsyncHttp async(request, &rl);
    async.startTransaction();
    CPPUNIT_ASSERT(async.ok());
    
    rl.run();
    CPPUNIT_ASSERT(async.ok());
    CPPUNIT_ASSERT(async.m_status.code() == Status::OK);
    
    string sContentLength = async.m_headers.get(Headers::ksContentLength);
    CPPUNIT_ASSERT(atoi(sContentLength.c_str()) == (int) async.m_body.size());
    
    string sRcvdBody = async.m_body.toString();
    CPPUNIT_ASSERT(sRcvdBody.length() > 0);
    CPPUNIT_ASSERT(sRcvdBody == sExpBody);
    
    // make sure all of our listener callbacks were hit
    CPPUNIT_ASSERT(async.m_connecting);
    CPPUNIT_ASSERT(async.m_connected);
    CPPUNIT_ASSERT(async.m_requestSent);
    CPPUNIT_ASSERT(async.m_complete);
    CPPUNIT_ASSERT(async.m_closed);
    CPPUNIT_ASSERT(async.m_percentReceived == 100.0);
}


void HttpClientTest::testSlowGetAsync()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("slowGetAsync.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    // allocate a runloop thread and initialize it on this thread of
    // execution
    bp::runloop::RunLoop rl;
    rl.init();
    
    // Setup the characteristics of the response we want from the server.
    const int    kRespLenKB     = 100;
    const double kPacketDelaySec= 0.5;
    const double kTimeoutSec    = 1.0;

    // Log the test conditions.
    BPLOG_INFO_STRM( "RespLenKB = " << kRespLenKB );
    BPLOG_INFO_STRM( "PacketDelaySec = " << kPacketDelaySec );
    BPLOG_INFO_STRM( "TimeoutSec = " << kTimeoutSec );
    
    // Setup the url, which will determine the response behavior of the
    // server's shaping handler.
    list<pair<string,string> > lpsFields;
    lpsFields.push_back(make_pair("respLenKB",toString(kRespLenKB)));
    lpsFields.push_back(make_pair("packetDelaySec",toString(kPacketDelaySec)));
    string sUrl = m_testServer.getShapingUrl() +
                  bp::url::makeQueryString(lpsFields);
    
    RequestPtr request(new Request(Method::HTTP_GET, sUrl));
    AsyncHttp async(request, &rl);
    async.setTimeoutSec(kTimeoutSec);

    bp::time::Stopwatch sw;
    sw.start();
    async.startTransaction();
    CPPUNIT_ASSERT(async.ok());

    rl.run();
    
    BPLOG_INFO_STRM( "transaction time (sec): " << sw.elapsedSec() );
    CPPUNIT_ASSERT(async.ok());
    CPPUNIT_ASSERT(async.m_status.code() == Status::OK);

    string sContentLength = async.m_headers.get(Headers::ksContentLength);
    CPPUNIT_ASSERT(atoi(sContentLength.c_str()) == (int) async.m_body.size());
    CPPUNIT_ASSERT(async.m_body.size() == kRespLenKB*1000);

    // make sure all of our listener callbacks were hit
    CPPUNIT_ASSERT(async.m_connecting);
    CPPUNIT_ASSERT(async.m_connected);
    CPPUNIT_ASSERT(async.m_requestSent);
    CPPUNIT_ASSERT(async.m_complete);
    CPPUNIT_ASSERT(async.m_closed);
    CPPUNIT_ASSERT(async.m_percentReceived == 100.0);

    BPLOG_INFO( "Unit test passed." );
}


void HttpClientTest::testNotFound()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("notFound.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const Response* prespExpected;
    string sUrl = m_testServer.notFoundTransaction(prespExpected);
    string sExpBody = prespExpected->body.toString();
    
    SyncTransaction tran(RequestPtr(new Request(Method::HTTP_GET, sUrl)));
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::NOT_FOUND);
    
    string sRcvdBody = ptrResp->body.toString();
    CPPUNIT_ASSERT(sRcvdBody.length() > 0);
    CPPUNIT_ASSERT(sRcvdBody == sExpBody);
}


void HttpClientTest::saveBodyToBinaryFile(const bp::file::Path& path,
                                          const Body& body)
{
    ofstream ofs;
	if (!bp::file::openWritableStream(ofs, path, ios::binary | ios::trunc)) {
        BP_THROW("Couldn't save body to binary file");
    }
    
    copy(body.begin(), body.end(), ostreambuf_iterator<char>(ofs));
}


void HttpClientTest::testBinaryGet()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("binaryGet.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const Response* prespExpected;
    string sUrl = m_testServer.binaryTransaction(prespExpected);
    
    // TODO: set accept headers?
    SyncTransaction tran(RequestPtr(new Request(Method::HTTP_GET, sUrl)));
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    
    CPPUNIT_ASSERT(ptrResp->body.size() > 0);
    CPPUNIT_ASSERT(ptrResp->body.size() == prespExpected->body.size());
    CPPUNIT_ASSERT(equal(prespExpected->body.begin(),
                         prespExpected->body.end(),
                         ptrResp->body.begin()));
    
    // Save to an output file for fun.
    //  saveBodyToBinaryFile("sophie.jpg", ptrResp->body);
}


void HttpClientTest::testBinaryGetAsync()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("binaryGetAsync.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const Response* prespExpected;
    string sUrl = m_testServer.binaryTransaction(prespExpected);
    
    // allocate a runloop thread and initialize it on this thread of
    // execution
    bp::runloop::RunLoop rl;
    rl.init();
    
    RequestPtr request(new Request(Method::HTTP_GET, sUrl));
    AsyncHttp async(request, &rl);
    async.startTransaction();
    CPPUNIT_ASSERT(async.ok());
    
    rl.run();
    CPPUNIT_ASSERT(async.ok());
    CPPUNIT_ASSERT(async.m_status.code() == Status::OK);
    CPPUNIT_ASSERT(async.m_body.size() > 0);
    CPPUNIT_ASSERT(async.m_body.size() == prespExpected->body.size());
    CPPUNIT_ASSERT(equal(prespExpected->body.begin(),
                         prespExpected->body.end(),
                         async.m_body.begin()));
    
    // Save to an output file for fun.
    //  saveBodyToBinaryFile("sophie.jpg", ptrResp->body);
}


void HttpClientTest::testPost()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("post.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const string ksBody = "A, B, C, it's easy as 1, 2, 3";
    
    bp::url::Url url(m_testServer.getEchoUrl());
    
    RequestPtr ptrReq(new Request(Method::HTTP_POST, url));
    ptrReq->body.assign(ksBody);
    SyncTransaction tran(ptrReq);
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    CPPUNIT_ASSERT(ptrResp->body.toString() == ksBody);
}


void HttpClientTest::testPostAsync()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("postAsync.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const string ksBody = "A, B, C, it's easy as 1, 2, 3";
    
    bp::url::Url url(m_testServer.getEchoUrl());
    
    RequestPtr request(new Request(Method::HTTP_POST, url));
    request->body.assign(ksBody);
    
    // allocate a runloop thread and initialize it on this thread of
    // execution
    bp::runloop::RunLoop rl;
    rl.init();
    
    AsyncHttp async(request, &rl);
    async.startTransaction();
    CPPUNIT_ASSERT(async.m_errorMsg.empty());
    
    rl.run();
    CPPUNIT_ASSERT(async.ok());
    CPPUNIT_ASSERT(async.m_status.code() == Status::OK);
    CPPUNIT_ASSERT(async.m_body.toString() == ksBody);
    
    // make sure all of our listener callbacks were hit
    CPPUNIT_ASSERT(async.m_connecting);
    CPPUNIT_ASSERT(async.m_connected);
    CPPUNIT_ASSERT(async.m_requestSent);
    CPPUNIT_ASSERT(async.m_complete);
    CPPUNIT_ASSERT(async.m_closed);
    CPPUNIT_ASSERT(async.m_percentSent == 100.0);
}


void HttpClientTest::testPostCRLF()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("postCRLF.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    const string ksBody = "abc\r\nabc";
    
    bp::url::Url url(m_testServer.getEchoUrl());
    
    RequestPtr ptrReq(new Request(Method::HTTP_POST, url));
    ptrReq->body.assign(ksBody);
    SyncTransaction tran(ptrReq);
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    CPPUNIT_ASSERT(ptrResp->body.toString() == ksBody);
}


void HttpClientTest::testServerDelay()
{    
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("serverDelay.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    bp::url::Url url(m_testServer.getEchoUrl());
    RequestPtr ptrReq(new Request(Method::HTTP_POST, url));
    
    const string ksBody = "Merwin is a manly poet";
    ptrReq->body.assign(ksBody);
    
    // Do a normal transaction.
    SyncTransaction tran(ptrReq);
    bp::time::Stopwatch sw;
    sw.start();
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    
    double fTime1 = sw.elapsedSec();
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    CPPUNIT_ASSERT(ptrResp->body.toString() == ksBody);
    
    // Now do it with a requested delay.
    const float kfDelaySec = 2;
    QueryString qs;
    qs.add("DelaySec", bp::conv::toString(kfDelaySec));
    ptrReq->url.setQuery(qs.toString());
    
    SyncTransaction tran2(ptrReq);
    sw.reset();
    sw.start();
    ptrResp = tran2.execute(results);
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    double fTime2 = sw.elapsedSec();
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);
    CPPUNIT_ASSERT(ptrResp->body.toString() == ksBody);
    
    // Verify delay was about what we expected.
    const double kfDelayTolerance = 0.5;
    double fObservedDelay = fTime2 - fTime1;
    CPPUNIT_ASSERT(fObservedDelay > 0);
    CPPUNIT_ASSERT(fabs(fObservedDelay-kfDelaySec) < kfDelayTolerance);
}


void HttpClientTest::testTimeout()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("timeout.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    bp::time::Stopwatch sw;
    
    bp::url::Url url(m_testServer.getEchoUrl());
    url.setQuery("DelaySec=5");
    
    // Set our timeout to something less than our requested delay.
    double fTimeout = 2;
    RequestPtr ptrReq(new Request(Method::HTTP_GET, url));
    SyncTransaction tran(ptrReq);
    tran.setTimeoutSec(fTimeout);
    
    sw.start();
    
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eTimedOut);
    
    // Verify timeout was about what we expected.
    double fTolerance = 0.5;
    double fTime = sw.elapsedSec();
    CPPUNIT_ASSERT(fabs(fTime-fTimeout) < fTolerance);
}


void HttpClientTest::testTimeoutAsync()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("timeoutAsync.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    bp::time::Stopwatch sw;
    
    bp::url::Url url(m_testServer.getEchoUrl());
    url.setQuery("DelaySec=5");
    
    // Set our timeout to something less than our requested delay.
    double fTimeout = 2;
    RequestPtr ptrReq(new Request(Method::HTTP_GET, url));
    
    // allocate a runloop thread and initialize it on this thread of
    // execution
    bp::runloop::RunLoop rl;
    rl.init();
    
    AsyncHttp async(ptrReq, &rl);
    async.m_transaction->setTimeoutSec(fTimeout);
    
    sw.start();
    async.startTransaction();
    CPPUNIT_ASSERT(async.m_errorMsg.empty());
    
    rl.run();
    CPPUNIT_ASSERT(async.m_errorMsg.empty());
    CPPUNIT_ASSERT(async.m_timedOut);
    
    // Verify timeout was about what we expected.
    double fTolerance = 0.5;
    double fTime = sw.elapsedSec();
    CPPUNIT_ASSERT(fabs(fTime-fTimeout) < fTolerance);
}


void HttpClientTest::testCancelAsync()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("cancelAsync.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    // an async helper who cancels the transaction at the
    // first sign of progress
    class MyAsync : public virtual AsyncHttp {
    public:
        MyAsync(RequestPtr request, bp::runloop::RunLoop *rl) 
        : AsyncHttp(request, rl) {
        }
        virtual void onResponseStatus(const Status&, const Headers&) {
            m_transaction->cancel();
        }
    };
    
    const Response* prespExpected;
    bp::url::Url url(m_testServer.binaryTransaction(prespExpected));
    
    // allocate a runloop thread and initialize it on this thread of
    // execution
    bp::runloop::RunLoop rl;
    rl.init();
    
    RequestPtr request(new Request(Method::HTTP_GET, url));
    MyAsync async(request, &rl);
    async.startTransaction();
    CPPUNIT_ASSERT(async.m_errorMsg.empty());
    
    rl.run();
    CPPUNIT_ASSERT(async.m_errorMsg.empty());
    CPPUNIT_ASSERT(async.m_cancelled);
}


// Test that only cookies we manually add to the header are sent.
void HttpClientTest::testCookies()
{
    log::removeAllAppenders();
    log::setupLogToFile(file::Path("cookies.log"),
                        log::LEVEL_DEBUG, log::kTruncate, log::TIME_MSEC);

    bp::url::Url url(m_testServer.getEchoUrl());
    RequestPtr ptrReq(new Request(Method::HTTP_POST, url));

    // Tell the test server to echo request headers in response body.
    QueryString qs;
    qs.add("EchoHeaders", bp::conv::toString(1));
    ptrReq->url.setQuery(qs.toString());

    // Add some cookie headers.
    string sCookie = "kind=oatmeal";
    Headers hdrs;
    hdrs.add( Headers::ksCookie, "kind=oatmeal" );
    ptrReq->headers = hdrs;
    
    // Dummy body.
    const string ksBody = "Cookies are delicious";
    ptrReq->body.assign(ksBody);

    // Execute the transaction.
    SyncTransaction tran(ptrReq);
    SyncTransaction::FinalStatus results;
    ResponsePtr ptrResp = tran.execute(results);
    CPPUNIT_ASSERT(results.code == SyncTransaction::FinalStatus::eOk);
    CPPUNIT_ASSERT(ptrResp->status.code() == Status::OK);

    string sBody = ptrResp->body.toString();
    string sCookieHdr = "Cookie: " + sCookie;
    string::size_type nIdx = sBody.find( sCookieHdr );
    CPPUNIT_ASSERT( nIdx != string::npos);
}
