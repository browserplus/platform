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

#include "QueryCache.h"
#include "WSProtocol.h"
#include "DistQuery.h"

#include <boost/scoped_ptr.hpp>

bool operator<(const AvailableService& lhs, const AvailableService& rhs)
{
    if (lhs.name.compare(rhs.name) < 0) return true;
    if (lhs.name.compare(rhs.name) > 0) return false;
    if (lhs.version.compare(rhs.version) < 0) return true;
    if (lhs.version.compare(rhs.version) > 0) return false;
    return false;
}

QueryCache::QueryCache(std::list<std::string> serverURLs,
                       const IServiceFilter * serviceFilter)
    : m_listener(NULL),
      m_qType(T_None),
      m_serverURLs(serverURLs),
      m_numComplete(0),
      m_serviceFilter(serviceFilter),
      m_clientListener(NULL)
{
    BPLOG_DEBUG_STRM("create QueryCache, this = " << this);
    m_sw.reset();
}

QueryCache::~QueryCache()
{
    BPLOG_DEBUG_STRM("delete QueryCache, this = " << this);
}

void
QueryCache::setListener(IQueryCacheListener * l)
{
    m_listener = l;
}

#define MAX_CACHE_AGE 60.0
// cache is scoped by platform
static std::map<std::string, AvailableServiceList> s_cache;
static bp::time::Stopwatch s_cacheAge;

static const AvailableServiceList * getPlatCache(std::string plat)
{
    std::map<std::string, AvailableServiceList>::iterator it;
    it = s_cache.find(plat);
    if (it != s_cache.end()) return &(it->second);
    return NULL;
}

static void clearPlatCache(std::string plat)
{
    std::map<std::string, AvailableServiceList>::iterator it;
    it = s_cache.find(plat);
    if (it != s_cache.end()) s_cache.erase(it);
}

static void setPlatCache(std::string plat, const AvailableServiceList & clp)
{
    std::map<std::string, AvailableServiceList>::iterator it;
    it = s_cache.find(plat);
    if (it != s_cache.end()) s_cache.erase(it);
    s_cache[plat] = clp;
}

void
QueryCache::serviceList(std::string plat)
{
    m_qType = T_ServiceList;

    if (m_serverURLs.size() == 0) {
        BPLOG_ERROR("QueryCache::ServiceList called without any distribution servers");
        // asynchronous failure return
        hop((void *) false);
        return;
    }

    if (plat.empty()) plat.append("none");
    m_plat = plat;
    
    const AvailableServiceList * cache = getPlatCache(plat);
    
    // simple cache implementation
    if (s_cacheAge.elapsedSec() < MAX_CACHE_AGE && cache != NULL) {
        // cache hit!
        BPLOG_INFO_STRM("Returning service list from cache ("
                        << s_cacheAge.elapsedSec() << "s old");
        m_listToReturn = *cache;
        // asynchronous success return
        hop((void *) true);
        return;
    } else {
        clearPlatCache(plat);
    }

    m_sw.reset(); m_sw.start();
    std::list<std::string>::iterator it;
    
    for (it = m_serverURLs.begin(); it != m_serverURLs.end(); it++) {
        std::string url =
            WSProtocol::buildURL(*it, WSProtocol::AVAILABLE_SERVICES_PATH);
        if (plat.compare("none") != 0) url += "/" + plat;

        bp::http::RequestPtr myReq = WSProtocol::buildRequest(url);    
        bp::http::client::TransactionPtr tran(new bp::http::client::Transaction(myReq));
        MyListenerPtr l = MyListener::alloc(*this, tran);
        m_listeners[*it] = l;
        BPLOG_INFO_STRM(l << ": initiate GET of available services for "
                        << plat);
        l->m_transaction->initiate(l);
    }

    // if we couldn't successfully kick off a single query, then we're
    // sunk
    if (m_numComplete == m_serverURLs.size()) {
        BPLOG_ERROR("Couldn't query any distribution servers for available services.");
        // asynchronous failure return
        hop((void *) false);
    }
}

void
QueryCache::listenerCompleted(MyListenerPtr l,
                              const std::string& error)
{
    std::string prError;
    if (!error.empty()) {
        prError.append(", error = ");
        prError.append(error);
    }

    BPLOG_INFO_STRM(l << ": completed request");

    m_numComplete++;
    
    if (!error.empty()) {
        BPLOG_ERROR_STRM("HTTP error (" << error << ")");
        // remove from m_listeners
        std::map<std::string, MyListenerPtr>::iterator it;
        for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
            if (it->second == l) {
                break;
            }
        }
        BPASSERT(it != m_listeners.end());
        m_listenersToReap.insert(it->second);
        m_listeners.erase(it);
    }
    
    // are we done yet?
    if (m_numComplete == m_serverURLs.size()) {
        // now let's prune any queries that failed
        std::map<std::string, MyListenerPtr>::iterator it = m_listeners.begin();
        while (it != m_listeners.end()) {
            MyListenerPtr l2 = it->second;
            if (l2->response()->status.code() != bp::http::Status::OK) {
                BPLOG_ERROR_STRM("HTTP error " << l2->response()->status.code() 
                                 << "(" << l2->response()->status.toString()
                                 << ")");
                m_listenersToReap.insert(it->second);
                m_listeners.erase(it);
                it = m_listeners.begin();
            } else {
                it++;
            }
        }

        if (m_qType == T_PlatformVersion) {
            // each response should be a version number, i.e. "1.1.6"
            // parse them all to find the newest available version,
            // respecting priority order of the servers (favor primary)
            BPLOG_INFO_STRM("Platform version queried " << m_listeners.size()
                            << " distro servers in "
                            << m_sw.elapsedSec() << "s");
            
            if (!parsePlatformVersionResponses(m_latest)) {
                BPLOG_ERROR("No valid server responses for latest platform version");
                hop((void *) false);
            } else {
                hop((void *) true);
            }
        } else if (m_qType == T_ServiceList) {
            // now let's parse the server responses into one list
            AvailableServiceList cl;
            cl = mergeResponses();
            pruneBlacklisted(cl);

            s_cacheAge.reset();
            s_cacheAge.start();
            setPlatCache(m_plat, cl);
            m_listToReturn = cl;
            
            BPLOG_INFO_STRM("Queried " << m_listeners.size()
                            << " distro servers in "
                            << m_sw.elapsedSec() << "s");

            // deliver the correct callback
            hop((void *) true);
        } else {
            BPLOG_ERROR_STRM("m_qType has unexpected value: " << m_qType);
        }

        // release shared ptrs in m_listeners
        for (it = m_listeners.begin(); it != m_listeners.end(); ++it) {
            m_listenersToReap.insert(it->second);
        }
        m_listeners.clear();
    }
}

void
QueryCache::onHop(void * boolAsPtr)
{
    bool success = (bool) boolAsPtr;
    
    if (!m_listener) return;
    
    if (m_qType == T_PlatformVersion) {
        if (!success) m_listener->onLatestPlatformFailure();
        else m_listener->onLatestPlatform(m_latest);
    } else if (m_qType == T_ServiceList) {
        if (!success) m_listener->onServiceListFailure();
        else m_listener->onServiceList(m_listToReturn);
    } else {
        BPLOG_ERROR_STRM("m_qType has unexpected value: " << m_qType);        
    }
    
}

bool
QueryCache::parsePlatformVersionResponses(LatestPlatformServerAndVersion & latest)
{
    // iterate through all of the responses in priority order
    std::list<std::string>::iterator url_it;
    
    bool foundOne = false;
    
    for (url_it = m_serverURLs.begin();
         url_it != m_serverURLs.end();
         url_it++) 
    {
        using namespace bp;
        std::map<std::string, MyListenerPtr>::iterator it;

        it = m_listeners.find(*url_it);
        if (it == m_listeners.end()) continue;

        MyListenerPtr l = it->second;
        if (l->response()->body.size() == 0) 
        {
            BPLOG_WARN_STRM("missing response body for HTTP request");
            continue;
        }

        std::string json = l->response()->body.toString();
        BPASSERT(!json.empty());

        boost::scoped_ptr<bp::Object> pload(Object::fromPlainJsonString(json));

        if (pload == NULL) {
            BPLOG_WARN_STRM("syntax error in HTTP response");
            continue;
        }
        
        const Map* m = dynamic_cast<const Map*>(pload.get());
        if (m != NULL) {
            std::string version;
            std::string server;
            int size = 0;
            if (m->getString("version", version)
                && m->getInteger("size", size))
            {
                server = *url_it;
                bp::ServiceVersion cur;
                if (cur.parse(version)) {
                    if (cur.compare(latest.version) > 0) {
                        latest.version = cur;
                        latest.size = size;
                        latest.serverURL = server;
                        foundOne = true;
                    }
                }
            }
        }
    }

    return foundOne;
}

std::list<AvailableService>
QueryCache::mergeResponses()
{
    // a set of the services that we've already seen
    std::set<AvailableService> seenServices;
    // the full list that we will return
    std::list<AvailableService> returnList;

    // iterate through all of the responses in priority order
    std::list<std::string>::iterator url_it;
    
    for (url_it = m_serverURLs.begin();
         url_it != m_serverURLs.end();
         url_it++)
    {
        using namespace bp;
        std::map<std::string, MyListenerPtr>::iterator it;

        it = m_listeners.find(*url_it);
        if (it == m_listeners.end()) continue;

        MyListenerPtr listener = it->second;
        if (listener->response()->body.size() == 0) 
        {
            BPLOG_WARN_STRM("missing response body for HTTP request");
            continue;
        }

        std::string json = listener->response()->body.toString();
        BPASSERT(!json.empty());
        boost::scoped_ptr<bp::Object> pload(Object::fromPlainJsonString(json));

        if (pload == NULL) {
            BPLOG_WARN_STRM("syntax error in HTTP response");
            continue;
        }
        
        const List* l = dynamic_cast<const List*>(pload.get());
        if (l != NULL) {
            // iterate through the list.  children should be maps.  each
            // must have a name and a versionString
            unsigned int i;
            for (i=0; i < l->size() ; i++) {
                const Map* m = dynamic_cast<const Map*>(l->value(i));
                if (m == NULL || 
                    !m->has("name", BPTString) ||
                    !m->has("versionString", BPTString) ||
                    !m->has("size", BPTInteger))
                {
                    BPLOG_WARN_STRM("malformed server response for "
                                    << "available services");
                    continue;
                }

                std::string name =
                    (dynamic_cast<const String*>(m->get("name")))->value();

                std::string verStr =
                    (dynamic_cast<const String*>(
                        m->get("versionString")))->value();

                unsigned int size =
                    (unsigned int) (dynamic_cast<const Integer*>(m->get("size")))->value();

                AvailableService ac;
                ac.dependentService = false;
                ac.serverURL = it->first;
                ac.name = name;
                ac.sizeBytes = size;

                if (!ac.version.parse(verStr)) {
                    BPLOG_WARN_STRM("malformed version string in "
                                    << "available services");
                    continue;
                }

                // now extract provider information
                // now if thist is a dependent service, let's report that
                if (m->has("CoreletType", BPTString)) {
                    std::string ctype = dynamic_cast<const String*>(m->get("CoreletType"))->value();
                    if (!ctype.compare("dependent"))
                    {
                        ac.dependentService = true;

                        // now we require a name key
                        if (!m->has("CoreletRequires/Name", BPTString)) {
                            continue;
                        }

                        // extract service name
                        ac.providerName =
                            (dynamic_cast<const String*>
                             (m->get("CoreletRequires/Name")))->value();

                        if (m->has("CoreletRequires/Minversion", BPTString))
                        {
                            ac.providerMinversion = 
                                (dynamic_cast<const String*>
                                 (m->get("CoreletRequires/Minversion")))
                                ->value();
                        }
                
                        if (m->has("CoreletRequires/Version", BPTString))
                        {
                            ac.providerVersion = 
                                dynamic_cast<const String*>
                                (m->get("CoreletRequires/Version"))
                                ->value();
                        }
                    }
                }

                if (seenServices.find(ac) == seenServices.end()) {
                    seenServices.insert(ac);
                    returnList.push_back(ac);
                }
            }
        }
    }

    returnList.sort();

    return returnList;
}

void
QueryCache::pruneBlacklisted(AvailableServiceList & lst)
{
    std::list<AvailableService>::iterator it = lst.begin();    

    while (it != lst.end())
    {
        if (NULL != m_serviceFilter &&
            !m_serviceFilter->serviceMayRun(it->name,
                                            it->version.asString()))
        {
            BPLOG_INFO_STRM("Purging blacklisted service from available "
                            << "service list: "
                            << it->name << " - " << it->version.asString());
            it = lst.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void
QueryCache::latestPlatformVersion(std::string plat)
{
    m_qType = T_PlatformVersion;

    if (m_serverURLs.size() == 0) {
        BPLOG_ERROR("QueryCache::latestPlatformVersion called without any distribution servers");
        hop((void *) false);
        return;
    }

    if (plat.empty()) {
        BPLOG_ERROR("QueryCache::latestPlatformVersion without valid OS");
        hop((void *) false);
        return;
    }
    
    m_plat = plat;
    
    m_sw.reset(); m_sw.start();
    std::list<std::string>::iterator it;
    
    for (it = m_serverURLs.begin(); it != m_serverURLs.end(); it++) {
        std::string url = 
            WSProtocol::buildURL(*it, WSProtocol::LATEST_PLATFORM_VERSION_PATH);
        url += "/" + m_plat;

        bp::http::RequestPtr myReq = WSProtocol::buildRequest(url);    
        bp::http::client::TransactionPtr tran(new bp::http::client::Transaction(myReq));
        MyListenerPtr l = MyListener::alloc(*this, tran);
        m_listeners[*it] = l;
        BPLOG_INFO_STRM(l << ": initiate GET of latest platform for  " << m_plat);
        l->m_transaction->initiate(l);
    }

    // if we couldn't successfully kick off a single query, then we're
    // sunk
    if (m_numComplete == m_serverURLs.size()) {
        BPLOG_ERROR("Couldn't query any distribution servers for available platform version.");
        hop((void *) false);
    }
}


void
QueryCache::MyListener::onTimeout()
{
    if (m_listening) {
        m_owner.listenerCompleted(shared_from_this(), "timeout");
    }
}


void
QueryCache::MyListener::onClosed()
{
    if (m_listening) {
        m_owner.listenerCompleted(shared_from_this(), "");
    }
}


void 
QueryCache::MyListener::onCancel()
{
    if (m_listening) {
        m_owner.listenerCompleted(shared_from_this(), "cancelled");
    }
}


void
QueryCache::MyListener::onError(const std::string& msg)
{
    if (m_listening) {
        m_owner.listenerCompleted(shared_from_this(), msg);
    }
}

                                                
