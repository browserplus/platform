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

/*
 *  HttpTransaction_Darwin.cpp
 *
 *  Implements the Transaction class and related items.
 *
 *  Created by Gordon Durand on 09/22/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */

#import <sys/errno.h>

#import "BPUtils/BPLog.h"
#import "BPUtils/bpfile.h"
#import "BPUtils/bprunloopthread.h"
#import "BPUtils/bpthreadhopper.h"
#import "HttpTransaction.h"
#import "bpconvert.h"

#import <Foundation/Foundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

static const char* kDefaultUserAgent = "Yahoo! BrowserPlus (osx)";
static const double kDefaultTimeoutSecs = 30.0;

static void streamCB(CFReadStreamRef stream,
                     CFStreamEventType eventType,
                     void* cbInfo);

// This implementation occurs in three levels.  First, there's 
// bp::http::client::Transaction, which contains a opaque pointer
// to a bp::http::client::TransactionImpl.  The TransactionImpl then
// delegates everything to an Objective-C++ TransactionHelper class.
// The extra level is needed since Objective-C++ doesn't support namespaces,
// hence bp::http::client::TransactionImpl must be a C++ class.

//---------------------------------------------------------------------------
// HTTP_TRANS_HELPER is an Objective-C++ class which uses NS/CF to do
// the http heavy lifting.
//
@interface HTTP_TRANS_HELPER : NSObject
{
    NSURLConnection* m_connection;
    NSMutableURLRequest* m_request;
    double m_timeoutSecs;
    bp::http::client::IListener* m_listener;
    
    // async post stuff (needed for progress)
    NSDictionary* m_headers;
    int m_status;
    size_t m_sendTotalBytes;
    size_t m_bytesSent;
    size_t m_receiveTotalBytes;
    size_t m_bytesReceived;
    CFReadStreamRef m_stream;  // only set for async POST
    NSTimer* m_progressTimer;
    bp::time::Stopwatch * m_timeoutStopwatch;
    double m_lastProgressSent;
    bool m_zeroSendProgressSent;
    bool m_hundredSendProgressSent;
    bool m_zeroReceiveProgressSent;
    
    // needed to respect system proxy settings in async CFHTTP case.
    CFDictionaryRef m_proxyDict;

    // When body contents are in a file, we memory map it in. 
    // To prevent it from being deleted out from under us, 
    // we open it and close it when we're done.
    int m_pathFd; 
}

- (id) init;
- (void) initiate: (bp::http::RequestPtr) request
         listener: (bp::http::client::IListener*) listener;
- (void) cancel;
- (double) timeoutSec;
- (void) setTimeoutSec: (double) secs;
- (void) cleanup;

// Delegate methods for async get.  NSURLConnection will use these delegates.
//
- (void) connection: (NSURLConnection*) connection 
   didFailWithError: (NSError*) error;
- (void) connection: (NSURLConnection*) connection 
     didReceiveData: (NSData*) data;
- (void) connection: (NSURLConnection*) connection 
 didReceiveResponse: (NSURLResponse*) response;
- (NSCachedURLResponse*) connection: (NSURLConnection*) connection 
                  willCacheResponse: (NSCachedURLResponse*) cachedResponse;
- (NSURLRequest*) connection: (NSURLConnection*) connection 
             willSendRequest: (NSURLRequest*) request 
            redirectResponse: (NSURLResponse*) redirectResponse;
- (void) connectionDidFinishLoading: (NSURLConnection*) connection;

// For async POST, these are invoked from CFStream event callback
//
- (void) onOpen;
- (void) onResponse;
- (void) onError;
- (void) onComplete;

// timer callbacks
//
- (void) onProgress: (NSTimer*) timer;

// common methods invoked by delegates and callbacks
//
- (void) handleResponseStatus: (NSDictionary*) headers status: (int) status;
- (void) handleResponseData: (const unsigned char*) buf length: (size_t) length;
- (void) handleComplete;
- (void) handleError: (std::string) msg;

@end


@implementation HTTP_TRANS_HELPER

// Initialize 
//
- (id) init
{
    if ((self = [super init])) {
        m_timeoutSecs = kDefaultTimeoutSecs;
        m_listener = nil;
        m_headers = nil;
        m_status = 0;
        m_sendTotalBytes = 0;
        m_bytesSent = 0;
        m_receiveTotalBytes = 0;
        m_bytesReceived = 0;
        m_progressTimer = nil;
        m_pathFd = -1;
        m_proxyDict = nil;
        m_lastProgressSent = 0.0;
        m_zeroSendProgressSent = false;
        m_hundredSendProgressSent = false;
        m_zeroReceiveProgressSent = false;
        m_timeoutStopwatch = new bp::time::Stopwatch;
    }
    return self;
}


// Execute an asynchronous request, notifying a listener of progress
//
- (void) initiate: (bp::http::RequestPtr) request 
         listener: (bp::http::client::IListener*) listener
{
    m_listener = listener;

    NS_DURING
        // set request params
        std::string method = request->method.toString();
        std::string resource = request->url.toString();
        BPLOG_INFO_STRM(self << ": async " << method);
        NSURL* url = [NSURL URLWithString: [NSString stringWithUTF8String: resource.c_str()]];
        m_request = [[NSMutableURLRequest requestWithURL: url] retain];
        [m_request setTimeoutInterval: m_timeoutSecs];
        [m_request setHTTPMethod: [NSString stringWithUTF8String: method.c_str()]];
        
        // set headers
        for (bp::http::Headers::const_iterator it = request->headers.begin();
             it != request->headers.end(); ++it) {
            [m_request setValue: [NSString stringWithUTF8String:it->second.c_str()]
                       forHTTPHeaderField: [NSString stringWithUTF8String:it->first.c_str()]];
        }
        
        // Add body for a POST.  Body can be a buffer or a file.  Try to
        // memory map files.
        m_listener->onConnecting();
        if (method.compare("POST") == 0) {
            // Must use CF streams to get progress 
            CFHTTPMessageRef req = CFHTTPMessageCreateRequest(kCFAllocatorDefault,
                                                              CFSTR("POST"),
                                                              (CFURLRef) url,
                                                              kCFHTTPVersion1_1);
            for (bp::http::Headers::const_iterator it = request->headers.begin();
                 it != request->headers.end(); ++it) {
                NSString* h = [NSString stringWithUTF8String: it->first.c_str()];
                NSString* v = [NSString stringWithUTF8String: it->second.c_str()];
                CFHTTPMessageSetHeaderFieldValue(req, (CFStringRef) h, 
                                                 (CFStringRef) v);
            }
        
            NSData* body = nil;
            NSError* error = nil;
            bp::file::Path path = request->body.path();
            if (!path.empty()) {
                std::string pstr = path.external_file_string();
                m_pathFd = ::open(pstr.c_str(), O_RDONLY);
                body = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String: pstr.c_str()]
                               options: NSUncachedRead | NSMappedRead
                               error: &error];
            } else {
                body = [NSData dataWithBytesNoCopy: (void*) request->body.elementAddr(0)
                               length: request->body.size()
                               freeWhenDone: NO];
            } 
            if (body == nil) {
                NSString* s = [error localizedDescription];
                [self handleError: std::string([s UTF8String])];
                return;
            }
            CFHTTPMessageSetBody(req, (CFDataRef) body);
            m_sendTotalBytes = [body length];
            
            m_stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, req);
    
            // apply system proxy settings, m_proxyDict released in cleanup
            m_proxyDict = SCDynamicStoreCopyProxies(NULL);
            CFReadStreamSetProperty(m_stream, kCFStreamPropertyHTTPProxy, m_proxyDict);
    
            CFRelease(req);
            
            CFStreamClientContext ctx = {0, self, NULL, NULL, NULL};
            CFOptionFlags flags = kCFStreamEventOpenCompleted 
                | kCFStreamEventHasBytesAvailable
                | kCFStreamEventCanAcceptBytes
                | kCFStreamEventEndEncountered
                | kCFStreamEventErrorOccurred;
            
            // set callback on stream
            if (!CFReadStreamSetClient(m_stream, flags, streamCB, &ctx)) {
                [self handleError: std::string("unable to set CFReadStream client")];
                return;
            }
            CFReadStreamScheduleWithRunLoop(m_stream, CFRunLoopGetCurrent(),
                                            kCFRunLoopCommonModes);
    
            // now let's set send buffer size
            CFSocketNativeHandle hand;
            CFDataRef nativeProp = (CFDataRef) CFReadStreamCopyProperty(
                (CFReadStreamRef)m_stream, kCFStreamPropertySocketNativeHandle);
            if (nativeProp != NULL) {
                CFDataGetBytes (nativeProp, 
                                CFRangeMake(0, CFDataGetLength(nativeProp)), 
                                (UInt8 *)&hand);
                CFRelease (nativeProp);
            }
                
#ifdef NOTDEF
            // get SO_NWRITE outta hand
            int nwrite = 65536 * 2;
            socklen_t nwritelen = sizeof(nwrite);
            setsockopt(hand, SOL_SOCKET, SO_SNDBUF,
                       (void *) &nwrite, nwritelen);
#endif
        
            // open stream and set progress timer
            if (!CFReadStreamOpen(m_stream)) {
                [self handleError: std::string("unable to open CFReadStream")];
                return;
            }
        
        } else {
            // Make an async request via NSURLConnection.  GET progress
            // comes via connection:didReceiveData:
            m_connection = [[NSURLConnection connectionWithRequest: m_request
                                             delegate: self] retain];
            if (m_connection == nil) {
                [self handleError: (bp::error::lastErrorString("async http connection failed"))];
                return;
            }
            m_listener->onConnected();
            m_listener->onRequestSent();
        }

        // for post we need a custom timeout timer, we'll always use this timer
        // (YIB-2898787)
        m_timeoutStopwatch->start();
            
        m_progressTimer = [NSTimer scheduledTimerWithTimeInterval: 0.25
                                   target: self
                                   selector: @selector(onProgress:)
                                   userInfo: nil
                                   repeats: YES];
        [m_progressTimer retain];

    NS_HANDLER
        [self handleError: std::string([[localException reason] UTF8String])];
    NS_ENDHANDLER
}


// Cancel a transaction
//
- (void) cancel
{
    BPLOG_INFO_STRM(self << ": cancel");

    [m_progressTimer invalidate];

    if (m_stream) {
        CFReadStreamUnscheduleFromRunLoop(m_stream, CFRunLoopGetCurrent(),
                                          kCFRunLoopCommonModes);
        [self cleanup];
    } else {
        [m_connection cancel];
    }
    m_listener->onCancel();
}


// get timeout
//
- (double) timeoutSec
{
    return m_timeoutSecs;
}


// set timeout
//
- (void) setTimeoutSec: (double) secs
{
    m_timeoutSecs = secs;
}


- (void) cleanup
{
    if (m_pathFd >= 0) {
        ::close(m_pathFd);
        m_pathFd = -1;
    }
    if (m_stream) {
        CFReadStreamClose(m_stream);
        CFRelease(m_stream);
        m_stream = nil;
    }
    if (m_proxyDict) {
        CFRelease(m_proxyDict);
        m_proxyDict = nil;
    }
    
    if (m_connection) {
        [m_connection release];
        m_connection = nil;
    }
    if (m_request) {
        [m_request release];
        m_request = nil;
    }
    if (m_progressTimer) {
        [m_progressTimer invalidate];
        [m_progressTimer release];
        m_progressTimer = nil;
    }
    if (m_timeoutStopwatch) {
        delete m_timeoutStopwatch;
        m_timeoutStopwatch = NULL;
    }
}


// ------------------ delegates

- (NSURLRequest*) connection: (NSURLConnection*) connection 
             willSendRequest: (NSURLRequest*) request 
            redirectResponse: (NSURLResponse*) redirectResponse
{
    bp::url::Url url([[[request URL] absoluteString] UTF8String]);
    m_listener->onRedirect(url);
    
    std::string respUrl = redirectResponse ?
                            [[[redirectResponse URL] absoluteString] UTF8String]
                            : "empty";
    BPLOG_INFO_STRM(self << ": connection gets redirect");
    BPLOG_DEBUG_STRM(self << ": response URL = " << respUrl
                     << ", redirect to " << url.toString());
    return [request copy];
}


- (void) connection: (NSURLConnection*) connection 
     didReceiveData: (NSData*) data 
{
    BPLOG_INFO_STRM(self << ": didReceiveData");
    [self handleResponseData: (const unsigned char*) [data bytes] length: [data length]];
}


- (void) connection: (NSURLConnection*) connection 
 didReceiveResponse: (NSURLResponse*) response
{
    BPLOG_INFO_STRM(self << ": didReceiveResponse");
    NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*) response;
    [self handleResponseStatus: [httpResponse allHeaderFields]
                        status: [httpResponse statusCode]];
}


- (NSCachedURLResponse*) connection: (NSURLConnection*) connection 
                  willCacheResponse: (NSCachedURLResponse*) cachedResponse
{
    // no response caching, to cache "return cachedResponse"
    return nil;
}


- (void) connectionDidFinishLoading: (NSURLConnection*) connection
{
    BPLOG_INFO_STRM(self << ": connectionDidFinishLoading");
    [self handleComplete];
}


- (void) connection: (NSURLConnection*) connection didFailWithError: (NSError*) error
{
    NSString* s = [error localizedDescription];
    std::string msg([s UTF8String]);
    BPLOG_INFO_STRM(self << ": didFailWithError, msg: " << msg);
    [self handleError: msg];
}


// async POST callbacks

- (void) onOpen
{
    m_listener->onConnected();
    m_listener->onRequestSent();
}


- (void) onResponse
{
    BPLOG_INFO_STRM(self << ": onresponse");
    CFHTTPMessageRef response =  (CFHTTPMessageRef) CFReadStreamCopyProperty(
        m_stream, kCFStreamPropertyHTTPResponseHeader); 
    if (response) {
        NSDictionary* headers= (NSDictionary*) CFHTTPMessageCopyAllHeaderFields(response); 
        int status = CFHTTPMessageGetResponseStatusCode(response);
        [self handleResponseStatus: headers status: status];
        [headers release];
        CFRelease(response);
    } 
    
    UInt8 buf[2048];
    CFIndex bytesRead = CFReadStreamRead(m_stream, buf, sizeof(buf));
    if (bytesRead > 0) {
        [self handleResponseData: buf length: bytesRead];
    }
}


- (void) onError
{
    BPLOG_INFO_STRM(self << ": onError");
    std::string msg;
    CFStreamError err = CFReadStreamGetError(m_stream);
    if (err.error == 0) {
        msg = "CFStream error event, but no error code";
    } else {
        if (err.domain == kCFStreamErrorDomainPOSIX) {
            msg = strerror(err.error);
        } else if (err.domain == kCFStreamErrorDomainMacOSStatus) {
            msg = "Mac error: " + bp::conv::lexical_cast<std::string>(err.error);
        }
    }
    [self handleError: msg];
}


- (void) onComplete
{
    BPLOG_INFO_STRM(self << ": onComplete");
    CFHTTPMessageRef response =  (CFHTTPMessageRef) CFReadStreamCopyProperty(
        m_stream, kCFStreamPropertyHTTPResponseHeader); 
    if (response) {
        NSDictionary* headers= (NSDictionary*) CFHTTPMessageCopyAllHeaderFields(response); 
        int status = CFHTTPMessageGetResponseStatusCode(response);
        [self handleResponseStatus: headers status: status];
        [headers release];
        CFRelease(response);
    }
    
    m_stream = NULL;
    [m_progressTimer invalidate];
    [self handleComplete];
}

//#include <iostream>
- (void) onProgress: (NSTimer*) timer
{
    // this .25s polling loop serves two purposes:
    // 1. is the mechanism that delivers client progress for
    //    post transactions (we know it's a post when m_stream is non-null)
    // 2. imposes an application level transaction inactivity timeout
    
    if (m_stream) {
        CFNumberRef numWrittenProperty = (CFNumberRef) CFReadStreamCopyProperty(
            m_stream, kCFStreamPropertyHTTPRequestBytesWrittenCount);
        // under a heavily loaded system (or libgmalloc) it's possible that
        // the timer will fire before the stream has anything written.
        // if numWrittenProperty is null, this functions is a noop
        if (numWrittenProperty != nil) {
            int numWritten;
            CFNumberGetValue(numWrittenProperty, kCFNumberSInt32Type, &numWritten);

            // now let's get the BSD socket to figger out what's left on its buffer
            CFSocketNativeHandle hand = 0;
            CFDataRef nativeProp = (CFDataRef) CFReadStreamCopyProperty(
                (CFReadStreamRef)m_stream, kCFStreamPropertySocketNativeHandle);
            if (nativeProp != NULL) {
                CFDataGetBytes(nativeProp, 
                               CFRangeMake(0, CFDataGetLength(nativeProp)),
                               (UInt8 *)&hand);
                CFRelease(nativeProp);
            }

            if (hand) {
                // get SO_NWRITE outta hand
                int nwrite = 0;
                socklen_t nwritelen = sizeof(nwrite);
                getsockopt(hand, SOL_SOCKET, SO_NWRITE,
                           (void *) &nwrite, &nwritelen);
                numWritten -= nwrite;
            }

            if (numWritten > (int) m_bytesSent) {
                // reset timeout stopwatch if we've made send progress
                m_timeoutStopwatch->reset();
                m_timeoutStopwatch->start();            

                m_bytesSent = numWritten;  
                double percent = m_sendTotalBytes ? 
                    (((double) m_bytesSent / m_sendTotalBytes) * 100) : 0.0;
                // honor our 0% guarantee, 100% will be sent on completion
                if (!m_zeroSendProgressSent) {
                    m_listener->onSendProgress(0, m_sendTotalBytes, 0.0);
                    m_zeroSendProgressSent = true;
                    m_lastProgressSent = 0.0;
                }
                if (percent > m_lastProgressSent) {
                    m_listener->onSendProgress(m_bytesSent, m_sendTotalBytes, percent);
                    m_lastProgressSent = percent;

                    // if percent here is exactly 100%, flip a bit
                    // so we don't double send it.
                    if (percent == 100.0) {
                        m_hundredSendProgressSent = true;
                    }
                }
            }

            CFRelease(numWrittenProperty);
        }
    }
    
    if (m_timeoutStopwatch != NULL &&
        m_timeoutStopwatch->elapsedSec() > m_timeoutSecs) 
    {
        [self handleError: std::string("timed out")];
    }
}


- (void) handleResponseStatus: (NSDictionary*) headers status: (int) status
{
    if (m_headers && [m_headers isEqualToDictionary: headers]
        && m_status == status) {
        // we've seen this before...
        return;
    }
    m_headers = headers;
    m_status = status;
    
    bp::http::Status httpStatus((bp::http::Status::Code) status);
    BPLOG_INFO_STRM(self << ": status " << httpStatus.toString());
    bp::http::Headers httpHeaders;
    NSEnumerator* keyEnumerator = [m_headers keyEnumerator];
    id key;
    while ((key = [keyEnumerator nextObject])) {
        NSString* keyStr = (NSString*) key;
        NSString* valStr = [m_headers objectForKey: key];
        if (std::string([keyStr UTF8String]).compare(bp::http::Headers::ksContentLength) == 0) {
            try {
                m_receiveTotalBytes = bp::conv::lexical_cast<size_t>([valStr UTF8String]);
            } catch(const std::bad_cast&) {
                m_receiveTotalBytes = 0;
            }
        }
        BPLOG_DEBUG_STRM([keyStr UTF8String] << ": " << [valStr UTF8String]);
        httpHeaders.add([keyStr UTF8String], [valStr UTF8String]);
    }
    m_listener->onResponseStatus(httpStatus, httpHeaders);
}


- (void) handleResponseData: (const unsigned char*) buf length: (size_t) length
{
    // honor 0% and 100% guarantees for send progress
    if (!m_zeroSendProgressSent) {
        m_zeroSendProgressSent = true;
        m_listener->onSendProgress(m_sendTotalBytes, m_sendTotalBytes, 0.0);
    }

    // make sure that we send a 100% progress
    if (!m_hundredSendProgressSent) {
        m_hundredSendProgressSent = true;
        m_listener->onSendProgress(m_sendTotalBytes, m_sendTotalBytes, 100.0);
    }
    
    BPLOG_DEBUG_STRM(self << ": gets " << length << " response data bytes");
    m_bytesReceived += length; 
    double percent = m_receiveTotalBytes ? 
        (((double) m_bytesReceived / m_receiveTotalBytes) * 100) : 0.0;

    // honor our 0% guarantee, 100% will be sent on final buffer 
    if (!m_zeroReceiveProgressSent) {
        m_listener->onReceiveProgress(0, m_receiveTotalBytes, 0.0);
        m_lastProgressSent = 0.0;
        m_zeroReceiveProgressSent = true;
    }

    if (percent > m_lastProgressSent) {
        m_listener->onReceiveProgress(m_bytesReceived, m_receiveTotalBytes, percent);
        m_lastProgressSent = percent;
    }

    // reset timeout stopwatch each tme we make recieve progress
    m_timeoutStopwatch->reset();
    m_timeoutStopwatch->start();            

    m_listener->onResponseBodyBytes(buf, length);
}


- (void) handleComplete
{
    m_listener->onComplete();
    m_listener->onClosed();

    [self cleanup];
}


- (void) handleError: (std::string) msg
{
    // if msg contains "timed out", it's a timeout.
    // msg will be different depending on whether our
    // stopwatch triggers the timeout or NSUrl does, but
    // both strings contain "timed out"
    if (msg.find("timed out") != std::string::npos) {
        m_listener->onTimeout();
    } else {
        m_listener->onError(msg);
    }

    [m_progressTimer invalidate];
    
    if (m_stream) {
        CFReadStreamUnscheduleFromRunLoop(m_stream, CFRunLoopGetCurrent(),
                                          kCFRunLoopCommonModes);
        [self cleanup];
    } else {
        [m_connection cancel];
    }
}

@end


// Time for some C++
//
using namespace std;

static void 
streamCB(CFReadStreamRef stream,
         CFStreamEventType eventType,
         void* cbInfo)
{
    switch (eventType) {
        case kCFStreamEventOpenCompleted:
            [(HTTP_TRANS_HELPER*)cbInfo onOpen];
            break;
        case kCFStreamEventHasBytesAvailable:
            [(HTTP_TRANS_HELPER*)cbInfo onResponse];
            break;
        case kCFStreamEventEndEncountered:
            [(HTTP_TRANS_HELPER*)cbInfo onComplete];
            break;
        case kCFStreamEventErrorOccurred:
            [(HTTP_TRANS_HELPER*)cbInfo onError];
            break;
        default:
            break;
    }
}

namespace bp {
namespace http {
namespace client {

// ------------------- TransactionImpl delegates to HTTP_TRANS_HELPER

class Transaction::Impl
{
public:
    Impl(RequestPtr request) 
        : m_request(request), m_userAgent(kDefaultUserAgent), m_helper(nil),
          m_listener(nil) {
        m_helper = [[[HTTP_TRANS_HELPER alloc] init] retain];
    }
    
    ~Impl() {
        [m_helper release];
    }
    
    void initiate(IListener* pListener) { 
        m_listener = pListener;
        [m_helper initiate: m_request
                  listener: m_listener];
    }
    
    void cancel() {
        [m_helper cancel];
    }
    RequestPtr request() const {
        return m_request;
    }
    const string& userAgent() const {
        return m_userAgent;
    }
    void setUserAgent(const string userAgent) {
        m_userAgent = userAgent;
    }
    double timeoutSec() const {
        return [m_helper timeoutSec];
    }
    void setTimeoutSec(double secs) {
        [m_helper setTimeoutSec: secs];
    }
    
    RequestPtr m_request;
    string m_userAgent;
    HTTP_TRANS_HELPER* m_helper;
    IListener* m_listener;
};


// ---------------- Finally, Transaction delegates to TransactionImpl

Transaction::Transaction(RequestPtr request) :
    m_pImpl(new Impl(request))
{
    BPLOG_INFO_STRM(m_pImpl->m_helper << ": transaction created");
    BPLOG_DEBUG_STRM(m_pImpl->m_helper << ": transaction to "
                     <<  request->url.toString());
}

Transaction::~Transaction()
{
    BPLOG_INFO_STRM(m_pImpl->m_helper << ": transaction destroyed");
    // m_pImpl is a boost scoped ptr, no need to delete
}


double Transaction::defaultTimeoutSecs()
{
    return kDefaultTimeoutSecs;
}


void
Transaction::initiate(IListener* listener)

{
    if (listener == NULL) {
        BP_THROW_FATAL("null listener");
    }
    m_pImpl->initiate(listener);
}

void 
Transaction::cancel()
{
    m_pImpl->cancel();
}

RequestPtr
Transaction::request() const
{
    return m_pImpl->request();
}

const string&
Transaction::userAgent() const 
{ 
    return m_pImpl->userAgent();
}

void 
Transaction::setUserAgent(const string& userAgent) 
{ 
    m_pImpl->setUserAgent(userAgent);
}

double 
Transaction::timeoutSec() const 
{ 
    return m_pImpl->timeoutSec();
}

void 
Transaction::setTimeoutSec(double secs) 
{ 
    m_pImpl->setTimeoutSec(secs);
}


} // namespace client
} // namespace http
} // namespace bp
