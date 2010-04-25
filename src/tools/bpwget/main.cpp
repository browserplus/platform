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
 * bpwget - a command line HTTP client built on bputils - used for testing
 *
 * Author: Lloyd Hilaiel
 * (c) Yahoo! Inc 2008
 */

#include <iostream>
#include "BPUtils/APTArgParse.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/HttpSyncTransaction.h"

using namespace std;
using namespace std::tr1;


// definition of program arguments
static APTArgDefinition s_args[] =
{
    {
        "async", false, "", false, false, false,
        "Asynchronously execute outputting progress to stderr"
    },
    {
        "headers", false, "", false, false, false,
        "Dump headers as part of output"
    },
    {
        "post", true, "", false, false, false,
        "a file contaning body to post"
    },
    {
        "timeout", APT::TAKES_ARG, "30", APT::NOT_REQUIRED,
        APT::IS_INTEGER, APT::MAY_NOT_RECUR, "The inactivity timeout."
    },
    { "log", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "enable console logging, argument is level (debug, info, etc)"
    },
    { "logfile", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "Enable file logging, argument is a path, when combined with '-log' "
      "logging will occur to a file at the level specified."
    }
};

static void 
setupLogging(const APTArgParse& argParser)
{
    bp::log::removeAllAppenders();
    
    string level = argParser.argument("log");
    bp::file::Path path(argParser.argument("logfile"));

    if (level.empty() && path.empty())
        return;
    if (level.empty())
        level = "info";

    bp::log::Level logLevel = bp::log::levelFromString(level);
    bp::log::setLogLevel(logLevel);
    
    if (path.empty())
        bp::log::setupLogToConsole(logLevel);
    else
        bp::log::setupLogToFile(path, logLevel);
}


// utility function to output headers
static void
dumpHeaders(const bp::http::Version & version,
            const bp::http::Status& status,
            const bp::http::Headers & headers)
{
    cout << version.toString() << " " << status.toString() << endl;

    bp::http::Headers::const_iterator i;
    
    for (i = headers.begin(); i != headers.end(); i++)
    {
        cout << i->first << ": " << i->second << endl;
    }
    cout << endl;
}


// A helper class for async operations.  Runs in a separate RunLoopThread
//
class AsyncHttp : virtual public bp::http::client::Listener
{
public:

    AsyncHttp(shared_ptr<bp::http::Request> request,
              bp::runloop::RunLoop *rl,
              bool dumpHeaders) 
        : bp::http::client::Listener(),
          m_dumpHeaders(dumpHeaders), m_timedOut(false),
          m_cancelled(false), m_errorMsg(), m_rl(rl)
    {
        m_transaction = new bp::http::client::Transaction(request);
    }
    
    virtual ~AsyncHttp() {
        delete m_transaction;
    }

    void startTransaction(double timeo) {
        m_transaction->setTimeoutSec(timeo);
        m_transaction->initiate(this);
    }
    
    
    // -----------------  Listener overrides
    void onResponseStatus(const bp::http::Status& status,
                          const bp::http::Headers& headers) {
        bp::http::client::Listener::onResponseStatus(status, headers);
        if (m_dumpHeaders) dumpHeaders(bp::http::Version(), status, headers);
    }
    
    void onResponseBodyBytes(const unsigned char* pBytes, 
                             unsigned int size) {
        bp::http::client::Listener::onResponseBodyBytes(pBytes, size);
        cout.write((const char*)pBytes, size);
    }
    
    void onSendProgress( size_t bytesProcessed,
                         size_t totalBytes,
                         double percent ) {
        cerr << "sendProgress: " << bytesProcessed
             << " of " << totalBytes << " (" << percent 
             << "%)" << endl;
        bp::http::client::Listener::onSendProgress(bytesProcessed, totalBytes, percent);
    }
    
    void onReceiveProgress( size_t bytesProcessed,
                            size_t totalBytes,
                            double percent ) {
        cerr << "receiveProgress: " << bytesProcessed
             << " of " << totalBytes << " (" << percent << "%)" 
             << endl;
        bp::http::client::Listener::onReceiveProgress(bytesProcessed, totalBytes, percent);
    }
    
    void onClosed() {
        bp::http::client::Listener::onClosed();
        m_rl->stop();
    }
    void onTimeout() {
        cerr << "timeout" << endl;
        bp::http::client::Listener::onTimeout();
        m_rl->stop();
    }
    void onError(const string& msg) {
        cerr << "error: " << msg << endl;
        bp::http::client::Listener::onError(msg);
        m_rl->stop();
    }
    void onCancel() {
        cerr << "cancelled" << endl;
        bp::http::client::Listener::onCancel();
        m_rl->stop();
    }
    
    bp::http::client::Transaction* m_transaction;
    
    bool m_dumpHeaders;

    bool m_timedOut;
    bool m_cancelled;
    string m_errorMsg;
    bool ok() const {
        return !m_timedOut && !m_cancelled && m_errorMsg.empty();
    }
    
    // when complete, this class will stop the runloop,
    // returning control to main
    bp::runloop::RunLoop * m_rl;
};


int
main(int argc, const char ** argv)
{
    bp::log::setupLogToFile(bp::file::Path("bpwget.log"), bp::log::LEVEL_DEBUG);

    // process command line arguments
    APTArgParse ap(" <options> <url>\n  execute HTTP requests from the command line");
    int nargs;
    nargs = ap.parse(sizeof(s_args) / sizeof(s_args[0]), s_args, argc, argv);
    if (nargs < 0) {
        cerr << ap.error();
        return 1;
    }
    if (nargs != (argc - 1)) {
        cerr << argv[0] << ": " << "missing required url argument"
                  << endl;
        return 1;
    }
    string url(argv[nargs]);
    bool async = ap.argumentPresent("async");

    double timeout = 30.0;
    if (ap.argumentPresent("timeout")) {
        timeout = ap.argumentAsInteger("timeout");
    }
    
    setupLogging(ap);
    
    shared_ptr<bp::http::Request> req(new bp::http::Request);
    if (!req->url.parse(url)) {
        cerr << "invalid url: " << url << endl;
        return 1;
    }

    // check for post body
    if (ap.argumentPresent("post")) {
        bp::file::Path path(ap.argument("post"));
        if (!bp::file::exists(path)) {
            cerr << "no such file: " << path << endl;
            return 1;
        }
        string body;

        if (!bp::strutil::loadFromFile(path, body)) {
            cerr << "couldn't read file: " << path << endl;
            return 1;
        }
        
        // set the request type to POST
        req->method = bp::http::Method("POST");
        req->body.assign(body);
    }
    
    if (async) {

        // allocate a runloop thread and initialize it on this thread of
        // execution
        bp::runloop::RunLoop rl;
        rl.init();

        AsyncHttp async(req, &rl, ap.argumentPresent("headers"));
        async.startTransaction(timeout);
        if (async.ok()) {
            rl.run();
        } 

        if (!async.ok()) {
            exit(1);
        } 
       
    } else {
        using bp::http::client::SyncTransaction;
        
        SyncTransaction t(req);
        t.setTimeoutSec(timeout);
        SyncTransaction::FinalStatus results;
        bp::http::ResponsePtr resp = t.execute(results);
        switch (results.code) {
            case SyncTransaction::FinalStatus::eOk:
                if (ap.argumentPresent("headers")) {
                    dumpHeaders(resp->version, resp->status, resp->headers);
                }
                
                if (!resp->body.empty()) {
                    cout.write((const char *) resp->body.elementAddr(0),
                               resp->body.size());
                }
                break;
            case SyncTransaction::FinalStatus::eError:
                cout << "error: " << results.message << endl;
                break;
            case SyncTransaction::FinalStatus::eTimedOut:
                cout << "timed out" << endl;
                break;
            case SyncTransaction::FinalStatus::eCancelled:
                cout << "cancelled" << endl;
                break;
        }
    }
    
    return 0;
}
