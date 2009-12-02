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
 * QueryCache - A class responsible for generating a list of available
 *              corelets by querying multiple distribution servers.
 *              This class is also responsible for short term caching
 *              responses to minimize network traffic without requiring
 *              the client keep any sort of context.    
 */

#ifndef __QUERYCACHE_H__
#define __QUERYCACHE_H__

#include <string>

#include "api/DistQueryTypes.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpthreadhopper.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/HttpListener.h"
#include "BPUtils/HttpTransaction.h"

class LatestPlatformServerAndVersion
{
  public:
    LatestPlatformServerAndVersion() : version(), size(0), serverURL()  {}
    bp::ServiceVersion version;    
    int size;
    std::string serverURL;    
};

class IQueryCacheListener
{
  public:
    virtual void onCoreletList(const AvailableCoreletList & list) = 0;
    virtual void onCoreletListFailure() = 0;    
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
    
    void coreletList(std::string plat);

    void latestPlatformVersion(std::string plat);

  private:
    virtual void onHop(void * context);

    IQueryCacheListener * m_listener;

    enum { T_None, T_CoreletList, T_PlatformVersion } m_qType;

    class MyListener : public bp::http::client::Listener
    {
      public:
        MyListener(QueryCache& owner,
                   bp::http::client::Transaction* transaction)
        : bp::http::client::Listener(), 
          m_owner(owner), m_transaction(transaction)
        {
            BPLOG_DEBUG_STRM("create MyListener, this = " << this);
        }

        virtual ~MyListener() 
        {
            BPLOG_DEBUG_STRM("delete MyListener, this = " << this);
            delete m_transaction;
        }
            
        // overrides from Listener
        virtual void onClosed();
        virtual void onTimeout();
        virtual void onCancel();
        virtual void onError(const std::string& msg);

        QueryCache& m_owner;
        bp::http::client::Transaction* m_transaction;

      private:
        // no copy/assignment semantics
        MyListener(const Listener&);
        MyListener& operator=(const Listener&);
    };

    void listenerCompleted(MyListener* l,
                           const std::string& error);
    
    std::list<std::string> m_serverURLs;
    std::map<std::string, MyListener*> m_listeners;
    std::set<MyListener*> m_listenersToReap;
    unsigned int m_numComplete;

    // used in T_CoreletList case
    AvailableCoreletList mergeResponses();
    void pruneBlacklisted(AvailableCoreletList & oList);

    // used in T_PlatformVersion case
    bool parsePlatformVersionResponses(
        LatestPlatformServerAndVersion & oLatest);
    
    bp::time::Stopwatch m_sw;

    // the platform for which we're querying corelets
    std::string m_plat;

    // a filter which prunes services from the server returned list
    const class IServiceFilter * m_serviceFilter;

    // the client to notify of events
    IQueryCacheListener * m_clientListener;

    LatestPlatformServerAndVersion m_latest;
    AvailableCoreletList m_listToReturn;
};

#endif
