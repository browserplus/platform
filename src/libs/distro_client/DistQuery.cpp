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

#include "DistQuery.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/InstallID.h"
#include "WSProtocol.h"
#include "CoreletQuery.h"
#include "CoreletQueryUtil.h"
#include "QueryCache.h"
#include "PendingUpdateCache.h"

#include <iostream>

#define TID_START 1000

DistQuery::DistQuery(std::list<std::string> distroServers,
                     const IServiceFilter * serviceFilter)
    : m_tid(TID_START), m_serviceFilter(serviceFilter), m_listener(NULL)
{
    std::list<std::string>::const_iterator it;
    for (it = distroServers.begin(); it != distroServers.end(); ++it) {
        if (it->empty()) continue;
        try {
            bp::url::Url url(*it);
            m_distroServers.push_back(*it);
        } catch (const bp::url::ParseError& e) {
            BPLOG_WARN_STRM(*it << " failed to parse: " << e.what());
        }
    }
}


DistQuery::~DistQuery()
{
}


void
DistQuery::setListener(IDistQueryListener * listener)
{
    m_listener = listener;
}



DistQuery::TransactionContextPtr
DistQuery::allocTransaction(TransactionContext::Type t)
{
    TransactionContextPtr tc(new TransactionContext(*this));
    tc->m_type = t;
    tc->m_tid = m_tid++;
    tc->m_stopWatch.start();
    return tc;
}


unsigned int
DistQuery::availableServices(const std::string & platform)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::ListCorelets);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->availableServices(platform);

    return tc->m_tid;
}


unsigned int
DistQuery::downloadCorelet(const std::string & name,
                           const std::string & version,
                           const std::string & platform)
{
    if (name.empty() || version.empty() || platform.empty()) {
        return 0;
    }

    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc =  
        allocTransaction(TransactionContext::Download);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->downloadCorelet(name, version, platform);

    return tc->m_tid;
}


unsigned int
DistQuery::coreletDetails(const std::string & name, 
                          const std::string & version,
                          const std::string & platform)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::CoreletDetails);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->coreletDetails(name, version, platform);

    return tc->m_tid;
}


unsigned int
DistQuery::findService(const std::string & name,
                       const std::string & version,
                       const std::string & minversion,
                       const std::string & platform)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::FindCorelet);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->findCorelet(name, version, minversion, platform);

    return tc->m_tid;
}


unsigned int
DistQuery::permissions()
{
    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::Permissions);
    
    std::string url = WSProtocol::buildURL(m_distroServers.front(),
                                           WSProtocol::PERMISSIONS_PATH);
    bp::http::RequestPtr req(WSProtocol::buildRequest(url));
    tc->m_transaction.reset(new bp::http::client::Transaction(req));
    BPLOG_INFO_STRM(this << ": initiate GET for permissions");
    tc->m_transaction->initiate(tc.get());

    m_transactions.push_back(tc);
    
    return tc->m_tid;
}


DistQuery::TransactionContextPtr
DistQuery::findTransactionByCoreletQuery(const void * cq)
{
    std::list<TransactionContextPtr >::iterator it;
    for (it = m_transactions.begin(); it != m_transactions.end(); it++) {
        if ((*it)->m_coreletQuery.get() == cq) {
            TransactionContextPtr t = *it;
            m_transactions.erase(it);
            return t;
        }
    }

    return TransactionContextPtr();
}


void
DistQuery::removeTransaction(unsigned int tid)
{
    std::list<TransactionContextPtr >::iterator it;
    for (it = m_transactions.begin(); it != m_transactions.end(); it++) {
        if ((*it)->m_tid == tid) {
            m_transactions.erase(it);
            return;
        }
    }
}


unsigned int
DistQuery::reportPageUsage(const std::string & ysOSVersion,
                           const std::string & ysBPVersion,
                           const std::string & ysURL,
                           const std::string & ysID,
                           const std::string & ysUA,
                           const std::string & ysServices)
{
    // Assemble the url.
    // We currently do not want "version" nor "api".
    std::string url = m_distroServers.front() + "/" + WSProtocol::USAGE_PATH;
    
    bp::StrPairList lpsFields;
    lpsFields.push_back(std::make_pair("t", "pv"));
    lpsFields.push_back(std::make_pair("os", ysOSVersion));
    lpsFields.push_back(std::make_pair("bp", ysBPVersion));
    if (!ysURL.empty()) {
        lpsFields.push_back(std::make_pair("url", ysURL));
    }
    lpsFields.push_back(std::make_pair("id", ysID));
    lpsFields.push_back(std::make_pair("ua", ysUA));
    if (!ysServices.empty()) {
        lpsFields.push_back(std::make_pair("s", ysServices));
    }
    std::string sQuery = bp::url::makeQueryString(lpsFields);
    url.append(sQuery);
    BPLOG_DEBUG_STRM(this << ": Reporting page usage: " << url);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::ReportPageUsage);

    bp::http::RequestPtr req(WSProtocol::buildRequest(url));
    tc->m_transaction.reset(new bp::http::client::Transaction(req));
    BPLOG_INFO_STRM(this << ": initiate GET to report page usage");
    tc->m_transaction->initiate(tc.get());

    m_transactions.push_back(tc);
    
    return tc->m_tid;
}


unsigned int
DistQuery::reportInstall(const std::string & ysOSVersion,
                         const std::string & ysBPVersion,
                         const std::string & ysID)
{
    // Assemble the url.
    // We currently do not want "version" nor "api".
    std::string url = m_distroServers.front() + "/" + WSProtocol::USAGE_PATH;

    bp::StrPairList lpsFields;
    lpsFields.push_back(std::make_pair("t", "id"));
    lpsFields.push_back(std::make_pair("os", ysOSVersion));
    lpsFields.push_back(std::make_pair("bp", ysBPVersion));
    lpsFields.push_back(std::make_pair("id", ysID));
    std::string sQuery = bp::url::makeQueryString(lpsFields);
    url.append(sQuery);
    BPLOG_DEBUG_STRM(this << ": Reporting new install: " << url);

    TransactionContextPtr tc =
        allocTransaction(TransactionContext::ReportPageUsage);

    bp::http::RequestPtr req(WSProtocol::buildRequest(url));
    tc->m_transaction.reset(new bp::http::client::Transaction(req));
    BPLOG_INFO_STRM(this << ": initiate GET to report new install");
    tc->m_transaction->initiate(tc.get());

    m_transactions.push_back(tc);

    return tc->m_tid;
}


unsigned int
DistQuery::satisfyRequirements(
    const std::string & platform,
    const std::list<CoreletRequireStatement> & requirements,
    const std::list<bp::service::Summary> & installedCorelets)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::SatisfyRequirements);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->satisfyRequirements(platform, requirements, installedCorelets);

    return tc->m_tid;
}


unsigned int
DistQuery::updateCache(
    const std::string & platform,
    const std::list<CoreletRequireStatement> & requirements,
    const std::list<bp::service::Summary> & installedCorelets)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::UpdateCache);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->updateCache(platform, requirements, installedCorelets);

    return tc->m_tid;
}


unsigned int
DistQuery::serviceSynopses(const std::string & platform,
                           const std::string & locale,
                           const CoreletList & corelets)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::AttainServiceSynopses);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->serviceSynopses(platform, locale, corelets);

    return tc->m_tid;
}


void
DistQuery::purgeCache()
{
    (void) PendingUpdateCache::purge();
}


bool
DistQuery::installCoreletFromCache(const std::string & name,
                                   const std::string & version)
{
    return PendingUpdateCache::install(name, version);
}


bool
DistQuery::isCached(const std::string & name, const std::string & version)
{
    return PendingUpdateCache::isCached(name, version);
}


CoreletList
DistQuery::cachedCorelets()
{
    std::list<bp::service::Summary> cached = PendingUpdateCache::cached();

    std::list<bp::service::Summary>::iterator it;

    CoreletList lst;

    for (it = cached.begin(); it != cached.end(); it++)
    {
        lst.push_back(
            std::pair<std::string, std::string>(it->name(), it->version()));
    }

    return lst;
}


CoreletList
DistQuery::haveUpdates(
    const std::list<CoreletRequireStatement> &requirements,
    const std::list<bp::service::Summary> & installed)
{
    std::list<bp::service::Summary> updates = PendingUpdateCache::cached();

    CoreletList cl;

    AvailableCoreletList shouldInstall;
    if (CoreletQueryUtil::haveUpdates(requirements, installed, updates,
                                      shouldInstall))
    {
        cl = CoreletQueryUtil::reformatAvailableCoreletList(shouldInstall);
    }
    
    return cl;
}


unsigned int
DistQuery::latestPlatformVersion(const std::string & platform)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::LatestPlatformVersion);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->latestPlatformVersion(platform);

    return tc->m_tid;
}


unsigned int
DistQuery::downloadLatestPlatform(const std::string & platform)
{
    CoreletQuery * cq = new CoreletQuery(m_distroServers, m_serviceFilter);
    cq->setListener(this);

    TransactionContextPtr tc = 
        allocTransaction(TransactionContext::DownloadLatestPlatform);
    tc->m_coreletQuery.reset(cq);
    m_transactions.push_back(tc);

    cq->downloadLatestPlatform(platform);

    return tc->m_tid;
}

void
DistQuery::onTransactionFailed(const CoreletQuery * cq)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    if (m_listener) {
        m_listener->onTransactionFailed(ctx->m_tid);
    }
}

void
DistQuery::gotAvailableServices(const CoreletQuery * cq,
                                const AvailableCoreletList & list)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::ListCorelets);

    CoreletList cl =
        CoreletQueryUtil::reformatAvailableCoreletList(list);

    ctx->logTransactionCompletion(true);
    if (m_listener) {
        m_listener->gotAvailableServices(ctx->m_tid, cl);
    }
}

void
DistQuery::onServiceFound(const CoreletQuery * cq,
                          const AvailableCorelet & list)
{

    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::FindCorelet);

    ctx->logTransactionCompletion(true);
    if (m_listener) {
        m_listener->onServiceFound(ctx->m_tid, list);
    }
}

void
DistQuery::onDownloadProgress(const CoreletQuery * cq,
                              unsigned int pct)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    m_transactions.push_front(ctx);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::Download ||
           ctx->m_type == TransactionContext::UpdateCache ||
           ctx->m_type == TransactionContext::DownloadLatestPlatform);


    if ((ctx->m_type == TransactionContext::Download ||
         ctx->m_type == TransactionContext::DownloadLatestPlatform)
        && m_listener != NULL)
    {
        m_listener->onDownloadProgress(ctx->m_tid, pct);
    }
}

void
DistQuery::onDownloadComplete(const CoreletQuery * cq,
                              const std::vector<unsigned char> & buf)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::Download);
    ctx->logTransactionCompletion(true);
    if (m_listener) {
        m_listener->onDownloadComplete(ctx->m_tid, buf);
    }
}

void
DistQuery::gotServiceDetails(const CoreletQuery * cq,
                             const bp::service::Description & desc)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::CoreletDetails);

    ctx->logTransactionCompletion(true);
    m_listener->gotServiceDetails(ctx->m_tid, desc);
}

void
DistQuery::onRequirementsSatisfied(const CoreletQuery * cq,
                                   const CoreletList & clist)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::SatisfyRequirements);
    ctx->logTransactionCompletion(true);    

    m_listener->onRequirementsSatisfied(ctx->m_tid, clist);
}

void
DistQuery::onCacheUpdated(const CoreletQuery * cq,
                          const CoreletList & updates)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::UpdateCache);
    ctx->logTransactionCompletion(true);
    if (m_listener != NULL) {
        m_listener->onCacheUpdated(ctx->m_tid, updates);
    }
}

void
DistQuery::gotServiceSynopsis(const CoreletQuery * cq,
                              const ServiceSynopsisList & sslist)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::AttainServiceSynopses);
    ctx->logTransactionCompletion(true);
    m_listener->gotServiceSynopsis(ctx->m_tid, sslist);
}

void
DistQuery::gotLatestPlatformVersion(const CoreletQuery * cq,
                                    const std::string & latest)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::LatestPlatformVersion);
    ctx->logTransactionCompletion(true);
    if (m_listener) {
        m_listener->gotLatestPlatformVersion(ctx->m_tid, latest);
    }
}

void
DistQuery::onLatestPlatformDownloaded(
        const CoreletQuery * cq,
        const LatestPlatformPkgAndVersion & pkgAndVersion)
{
    TransactionContextPtr ctx = findTransactionByCoreletQuery(cq);
    BPASSERT(ctx != NULL);
    BPASSERT(ctx->m_type == TransactionContext::DownloadLatestPlatform);

    ctx->logTransactionCompletion(true);
    if (m_listener) {
        m_listener->onLatestPlatformDownloaded(ctx->m_tid, pkgAndVersion);
    }
}


DistQuery::TransactionContext::TransactionContext(DistQuery& owner)
    : bp::http::client::Listener(),
      m_owner(owner), m_type(None), m_tid(0), m_stopWatch(),
      m_transaction(), m_coreletQuery()
{
}


DistQuery::TransactionContext::~TransactionContext()
{
}


void 
DistQuery::TransactionContext::onResponseStatus(const bp::http::Status& status,
                                                const bp::http::Headers& headers)
{
    bp::http::client::Listener::onResponseStatus(status, headers);
    if (status.code() != bp::http::Status::OK) {
        BPLOG_ERROR_STRM("HTTP error " << status.code() 
                         << "(" << status.toString() << ")");
        logTransactionCompletion(false);
        
        if (m_owner.m_listener) {
            m_owner.m_listener->onTransactionFailed(m_tid);
        }
        return;
    }
}


void
DistQuery::TransactionContext::onTimeout()
{
    BPLOG_ERROR_STRM("HTTP timeout");
    bp::http::client::Listener::onTimeout();
    logTransactionCompletion(false);
    if (m_owner.m_listener) {
        m_owner.m_listener->onTransactionFailed(m_tid);
    }
    m_owner.removeTransaction(m_tid);
}

void
DistQuery::TransactionContext::onClosed()
{
    BPLOG_INFO_STRM("HTTP transaction closed");
    bp::http::client::Listener::onClosed();
    logTransactionCompletion(true);
    if (m_type == TransactionContext::ReportPageUsage) {
        // (buffer contains "ok")
        if (m_owner.m_listener) {
            m_owner.m_listener->onPageUsageReported(m_tid);
        }
    } else {
        // buffer contains bpkg
        if (m_owner.m_listener) {
            std::vector<unsigned char> body;
            body.insert(body.begin(),
                        response()->body.begin(),
                        response()->body.end());
            m_owner.m_listener->gotPermissions(m_tid, body);
        }
    }
    m_owner.removeTransaction(m_tid);
}


void 
DistQuery::TransactionContext::onCancel()
{
    BPLOG_ERROR_STRM("HTTP transaction cancelled");
    bp::http::client::Listener::onCancel();
    logTransactionCompletion(false);
    if (m_owner.m_listener) m_owner.m_listener->onTransactionFailed(m_tid);
    m_owner.removeTransaction(m_tid);
}


void
DistQuery::TransactionContext::onError(const std::string& msg)
{
    BPLOG_ERROR_STRM("HTTP error " << msg);
    bp::http::client::Listener::onError(msg);
    logTransactionCompletion(false);
    if (m_owner.m_listener) m_owner.m_listener->onTransactionFailed(m_tid);
    m_owner.removeTransaction(m_tid);
}


void
DistQuery::TransactionContext::logTransactionCompletion(bool success)
{
    const char * what = "internal error";

    switch (m_type) {
        case TransactionContext::ReportPageUsage:
            what = "report usage";
            break;
        case TransactionContext::UpdateCache:
            what = "service cache update";
            break;
        case TransactionContext::ListCorelets:
            what = "enumerate services";
            break;
        case TransactionContext::CoreletDetails:
            what = "describe service";
            break;
        case TransactionContext::Permissions:
            what = "attain permissions";
            break;
        case TransactionContext::Download:
            what = "download service";
            break;
        case TransactionContext::FindCorelet:
            what = "locate service";
            break;
        case TransactionContext::SatisfyRequirements:
            what = "satisfy requirements";
            break;
        case TransactionContext::AttainServiceSynopses:
            what = "attain service synopses";
            break;
        case TransactionContext::DownloadLatestPlatform:
            what = "download latest platform";
            break;
        case TransactionContext::LatestPlatformVersion:
            what = "determine latest available platform version";
            break;
        case TransactionContext::None:
            what = "unknown transaction";
    }
    
    const char * how = success ? "successfully" : "with errors";

    if (success) {
        BPLOG_INFO_STRM("(" << m_tid << ") " << what
                        << " completed " << how << " in " 
                        << m_stopWatch.elapsedSec() << "s");
    } else {
        BPLOG_WARN_STRM("(" << m_tid << ") "
                        << what << " completed " << how << " in " 
                        << m_stopWatch.elapsedSec() << "s");
    }
}

void
IDistQueryListener::gotAvailableServices(unsigned int,
                                         const CoreletList &)
{
}

void
IDistQueryListener::onServiceFound(unsigned int ,
                                   const AvailableCorelet &)
{
}
    
void
IDistQueryListener::onDownloadProgress(unsigned int,
                                       unsigned int)
{
}
    
void
IDistQueryListener::onDownloadComplete(unsigned int,
                                       const std::vector<unsigned char> &)
{
}
    
void
IDistQueryListener::gotServiceDetails(unsigned int,
                                      const bp::service::Description &)
{
}
    
void
IDistQueryListener::gotPermissions(unsigned int, std::vector<unsigned char>)
{
}
    
void
IDistQueryListener::onPageUsageReported(unsigned int)
{
}
    
void
IDistQueryListener::onRequirementsSatisfied(unsigned int, const CoreletList &)
{
}
    
void
IDistQueryListener::onCacheUpdated(unsigned int, const CoreletList &)
{
}
    
void
IDistQueryListener::gotServiceSynopsis(unsigned int ,
                                       const ServiceSynopsisList &)
{
}
    
void
IDistQueryListener::gotLatestPlatformVersion(unsigned int, const std::string &)
{
}
    
void
IDistQueryListener::onLatestPlatformDownloaded(
        unsigned int, const LatestPlatformPkgAndVersion &)
{
}
