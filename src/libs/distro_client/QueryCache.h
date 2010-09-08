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
 * QueryCache - A class responsible for generating a list of available
 *              services by querying multiple distribution servers.
 *              This class is also responsible for short term caching
 *              responses to minimize network traffic without requiring
 *              the client keep any sort of context.    
 */

#ifndef __QUERYCACHE_H__
#define __QUERYCACHE_H__

#include <string>

#include "api/DistQueryTypes.h"
#include "bphttp/HttpListener.h"
#include "bphttp/HttpTransaction.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpthreadhopper.h"
#include "BPUtils/bpstopwatch.h"


class LatestPlatformServerAndVersion
{
  public:
    LatestPlatformServerAndVersion() : version(), size(0), serverURL()  {}
    bp::SemanticVersion version;
    int size;
    std::string serverURL;    
};

class IQueryCacheListener
{
  public:
    virtual void onServiceList(const AvailableServiceList & list) = 0;
    virtual void onServiceListFailure() = 0;    
    virtual void onLatestPlatform(
        const LatestPlatformServerAndVersion & latest) = 0;
    virtual void onLatestPlatformFailure() = 0;    
    virtual ~IQueryCacheListener() { }
};

class QueryCache : public bp::thread::HoppingClass
{
  public:
    QueryCache(std::list<std::string> serverURLs,
               const class IServiceFilter * serviceFilter);
    ~QueryCache();

    void setListener(IQueryCacheListener * l);
    
    void serviceList(std::string plat);

    void latestPlatformVersion(std::string plat);

  private:
    virtual void onHop(void * context);

    IQueryCacheListener * m_listener;

    enum { T_None, T_ServiceList, T_PlatformVersion } m_qType;

    class MyListener : public bp::http::client::Listener,
                       public std::tr1::enable_shared_from_this<MyListener>
    {
      public:
        static std::tr1::shared_ptr<MyListener> alloc(
                QueryCache& owner,
                bp::http::client::TransactionPtr transaction)
        {
            std::tr1::shared_ptr<MyListener> rval(new MyListener(owner,
                                                                 transaction));
            return rval;
        }
        virtual ~MyListener() 
        {
            m_listening = false;
            BPLOG_DEBUG_STRM("delete MyListener, this = " << this);
        }
            
        // overrides from Listener
        virtual void onClosed();
        virtual void onTimeout();
        virtual void onCancel();
        virtual void onError(const std::string& msg);

        QueryCache& m_owner;
        bp::http::client::TransactionPtr m_transaction;

      private:
        MyListener(QueryCache& owner,
                   bp::http::client::TransactionPtr transaction)
        : bp::http::client::Listener(),
          m_owner(owner), m_transaction(transaction), m_listening(true)
        {
            BPLOG_DEBUG_STRM("create MyListener, this = " << this);
        }

        // no copy/assignment semantics
        MyListener(const Listener&);
        MyListener& operator=(const Listener&);

        bool m_listening;
    };
    typedef std::tr1::shared_ptr<MyListener> MyListenerPtr;

    void listenerCompleted(MyListenerPtr l,
                           const std::string& error);
    
    std::list<std::string> m_serverURLs;
    std::map<std::string, MyListenerPtr> m_listeners;
    std::set<MyListenerPtr> m_listenersToReap;
    unsigned int m_numComplete;

    // used in T_ServiceList case
    AvailableServiceList mergeResponses();
    // a utility function to remove blacklisted services and services
    // with an unsupported service API version from the list of returned
    // services
    void pruneBlacklistedAndUnsupported(AvailableServiceList & oList);

    // used in T_PlatformVersion case
    bool parsePlatformVersionResponses(
        LatestPlatformServerAndVersion & oLatest);
    
    bp::time::Stopwatch m_sw;

    // the platform for which we're querying services
    std::string m_plat;

    // a filter which prunes services from the server returned list
    const class IServiceFilter * m_serviceFilter;

    // the client to notify of events
    IQueryCacheListener * m_clientListener;

    LatestPlatformServerAndVersion m_latest;
    AvailableServiceList m_listToReturn;
};

#endif
