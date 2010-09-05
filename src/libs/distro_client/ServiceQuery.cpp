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
 * ServiceQuery - A class capable of querying multiple distribution servers
 *                to find and attain services.
 */

#include "ServiceQuery.h"
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpfile.h"
#include "ServiceQueryUtil.h"
#include "PendingUpdateCache.h"
#include "platform_utils/bplocalization.h"
#include "platform_utils/ProductPaths.h"
#include "WSProtocol.h"

// hopping actions.  We use a thread hopper to preserve
// sane API semantics and to prevent stack re-entrancy.
// hopping at this level is somewhat of a hack, as the HTTP
// implementation should be worked on a bit (let client call
// cancel() from within a callback, etc)

#define HOPACT_GET_NEXT_L10N ((void *) 0x1)
#define HOPACT_CANCEL_HTTP ((void *) 0x2)

using namespace std;
using namespace std::tr1;
namespace bpf = bp::file;

IServiceQueryListener::~IServiceQueryListener()
{
}

ServiceQuery::ServiceQuery(std::list<std::string> serverURLs,
                           const IServiceFilter * serviceFilter)
    : bp::http::client::Listener(), m_qc(serverURLs, serviceFilter),
      m_type(None), m_serviceFilter(serviceFilter), m_listener(NULL) 
{
    m_qc.setListener(this);
}


ServiceQuery::~ServiceQuery()
{
}

void
ServiceQuery::setListener(IServiceQueryListener * listener)
{
    m_listener = listener;
}

void 
ServiceQuery::onResponseStatus(const bp::http::Status& status,
                               const bp::http::Headers& headers)
{
    bp::http::client::Listener::onResponseStatus(status, headers);
    if (status.code() != bp::http::Status::OK) {
        BPLOG_WARN_STRM(this << ": HTTP error " << status.code() 
                        << "(" << status.toString() << ")");

        // cancel transaction now right after headers.  After an async
        // break, cancel() will be called on the http transaction
        // which will alert our listener and cause this transaction to
        // be deleted! (YIB-2921888)
        hop(HOPACT_CANCEL_HTTP);
    }
}


void 
ServiceQuery::onResponseBodyBytes(const unsigned char* pBytes, 
                                  unsigned int size)
{
    bp::http::client::Listener::onResponseBodyBytes(pBytes, size);
    if ((m_type == Download || m_type == DownloadLatestPlatform)
        && m_dlSize > 0)
    {
        unsigned int pct =
            (unsigned int) (100 * ((double) response()->body.size() /
                                   (double) m_dlSize));
        if (m_listener != NULL) {
            if (!m_zeroPctSent) {
                m_listener->onDownloadProgress(this, 0);
                m_zeroPctSent = true;
            }
            if (pct > m_lastPct) {
                m_lastPct = pct;
                m_listener->onDownloadProgress(this, pct);
            }
        }
    }
    
    m_cletBuf.insert(m_cletBuf.end(), pBytes, pBytes+size);
}


void 
ServiceQuery::onClosed()
{
    // maintain the life of this object until the completion
    // of the function in case one of our listeners delete us from
    // a callback.
    shared_ptr<ServiceQuery> tStrong(shared_from_this());

    BPLOG_INFO_STRM(this << ": onClosed");
    bp::http::client::Listener::onClosed();
    try {
        if (m_type == Download) {
            // now we've got the buffer of the downloaded service
            // file.  w00t.
            if (m_listener != NULL) {
                m_listener->onDownloadComplete(this, m_cletBuf);
            }
            m_cletBuf.clear();
        } else if (m_type == UpdateCache) {
            // now buf is a cached service we must install into cache
            BPLOG_INFO_STRM(this << ": CacheUpdate: downloaded "
                            << m_cletBuf.size()
                            << " bytes for "
                            << m_currentUpdate->name
                            << " v" << m_currentUpdate->version.asString());

            if (!PendingUpdateCache::save(m_currentUpdate->name,
                                                      m_currentUpdate->version.asString(),
                                                      m_cletBuf)) {
                std::string msg("unable to save " + m_currentUpdate->name
                                + "/" + m_currentUpdate->version.asString()
                                + " to pending update cache");
                transactionFailed(msg);
                return;
            }
            m_cletBuf.clear();
            
            if ((++m_currentUpdate) == m_updates.end()) {
                // all done
                if (m_listener) {
                    m_listener->onCacheUpdated(
                        this, 
                        ServiceQueryUtil::reformatAvailableServiceList(m_updates));
                }
                
            } else {
                startDownload(*m_currentUpdate);
            }
    
        } else if (m_type == ServiceDetails) {
            if (response()->body.size() == 0) {
                throw("missing response body");
            }
            
            std::string json = response()->body.toString();
            BPASSERT(!json.empty());
            std::string verboseError;
            bp::Object* payload =
                bp::Object::fromPlainJsonString(json, &verboseError);

            if (payload == NULL) {
                throw("syntax error in server response: " + verboseError);
            }
            
            bp::service::Description desc;
            
            if (desc.fromBPObject(payload)) {
                if (m_listener) {
                    m_listener->gotServiceDetails(this, desc);
                }
            } else {
                throw("bad service description");
            }
            delete payload;

        } else if (m_type == AttainServiceSynopses) {
            if (response()->body.size() == 0) {
                throw("no body in AttainServiceSynopses response");
            } else {
                // now body holds the localized description of the
                // service.  we must parse it and append information
                // to last entry in m_locDescs list
                parseLocalization(response()->body.elementAddr(0),
                                  response()->body.size());
            }

        } else if (m_type == DownloadLatestPlatform) {
            if (response()->body.size() == 0) {
                transactionFailed("downloaded platform has empty body");
            } else if (m_listener) {
                LatestPlatformPkgAndVersion pkgAndVersion;
                pkgAndVersion.m_pkg.insert(pkgAndVersion.m_pkg.begin(),
                                           response()->body.begin(),
                                           response()->body.end());
                pkgAndVersion.m_version = m_version;
                
                    
                m_listener->onLatestPlatformDownloaded(this, pkgAndVersion);
            }
        } else {
            throw("bad m_type" + m_type);
        }
    } catch (const std::string& s) {
        BPLOG_WARN_STRM(this << ": error (" << s << ") handling response");
        transactionFailed(s);
    }
}


void 
ServiceQuery::onTimeout()
{
    std::string msg("transaction timed out");
    BPLOG_WARN_STRM(this << ": " << msg);
    bp::http::client::Listener::onTimeout();
    transactionFailed(msg);
}

void 
ServiceQuery::onCancel() 
{
    std::string msg("transaction canceled");
    BPLOG_WARN_STRM(this << ": " << msg);
    bp::http::client::Listener::onCancel();
    transactionFailed(msg);
}


void 
ServiceQuery::onError(const std::string& msg) 
{
    std::string errMsg("transaction error " + msg);
    BPLOG_WARN_STRM(this << ": " << errMsg);
    bp::http::client::Listener::onError(msg);
    transactionFailed(errMsg);
}


void
ServiceQuery::availableServices(std::string platform)
{
    m_type = AvailableServices;
    m_qc.serviceList(platform);
}


void
ServiceQuery::findService(std::string name, std::string version,
                          std::string minversion, std::string platform)
{
    m_type = FindService;
    m_qc.serviceList(platform);

    m_name = name;
    m_version = version;
    m_minversion = minversion;
}

void
ServiceQuery::downloadService(std::string name, std::string version,
                              std::string platform)
{
    m_type = Download;
    m_qc.serviceList(platform);
    m_name = name;
    m_version = version;
    m_platform = platform;
}


void
ServiceQuery::serviceDetails(std::string name, std::string version,
                             std::string platform)
{
    m_type = ServiceDetails;
    m_qc.serviceList(platform);
    m_name = name;
    m_version = version;
    m_platform = platform;
}


void
ServiceQuery::serviceSynopses(const std::string & platform,
                              const std::string & locale,
                              const ServiceList & services)
{
    if (platform.empty()) {
        BP_THROW_FATAL("empty platform in ServiceQuery::serviceSynopses()");
    }
    if (locale.empty()) {
        BP_THROW_FATAL("empty locale in ServiceQuery::serviceSynopses()");
    }
    
    m_type = AttainServiceSynopses;
    m_platform = platform;
    m_locale = locale;
    
    m_serviceList = services;

    // hop before getting next localization.  this prevents us from
    // calling back to the caller before our function returns.
    hop(HOPACT_GET_NEXT_L10N);
}

void 
ServiceQuery::onHop(void * hopact)
{
    if (hopact == HOPACT_GET_NEXT_L10N) {
        getNextLocalization();    
    } else if (hopact == HOPACT_CANCEL_HTTP && m_httpTransaction) {
        m_httpTransaction->cancel();
    }
}

void
ServiceQuery::getNextLocalization()
{
    // nothing more to localize?  call it quits 
    if (m_serviceList.empty()) {
        if (m_listener) {
            m_listener->gotServiceSynopsis(this, m_locDescs);
        }
        return;
    }

    std::string name, version;
    std::pair<std::string, std::string> i =  m_serviceList.front();
    m_serviceList.pop_front();

    name = i.first;
    version = i.second;
    
    // is this thing in the cache?  If so we can use the localization
    // already on disk.
    bp::service::Summary sum;
    if (PendingUpdateCache::getSummary(name, version, sum))
    {
        std::string title, summary;
        
        if (!sum.localization(m_locale, title, summary)) {
            if (!sum.localization(std::string("en"), title, summary)) {
                std::stringstream ss;
                ss << "Couldn't localize " << name
                        << "/" << version << " from cache!";
                BPLOG_ERROR_STRM(this << ": " << ss.str());
                transactionFailed(ss.str());
                return;
            }
        }

        // now add a 
        ServiceSynopsis synopsis;
        synopsis.m_name = name;
        synopsis.m_version = version;
        synopsis.m_title = title;
        synopsis.m_summary = summary;
        
        // zero size, this is an update!
        synopsis.m_sizeInBytes = 0;
        synopsis.m_isUpdate = true;

        m_locDescs.push_back(synopsis);

        getNextLocalization();
    } else {
        // we must perform an http transaction to get localized strings
        // from the distro server.

        // XXX: what if there are no services on the distro servers?
        if (m_services.size() == 0) {
            m_serviceList.push_front(i);
            m_qc.serviceList(m_platform);
        } else {
            // figure out which distribution server to query
            AvailableService acp;
            if (!ServiceQueryUtil::findBestMatch(i.first, i.second,
                                                std::string(), m_services,
                                                acp))
            {
                std::stringstream ss;
                ss << "Couldn't localize " << name << "/" << version;
                BPLOG_ERROR_STRM(this << ": " << ss.str());
                transactionFailed(ss.str());
                return;
            }
            
            // populate name, version, and size now, and title and summary
            // when HTTP request comes back
            ServiceSynopsis synopsis;
            synopsis.m_name = name;
            synopsis.m_version = version;
            synopsis.m_sizeInBytes = acp.sizeBytes;

            // If any version already exists on disk, this is an update.
            // This happens when minversion in a require forces us
            // to download a newer version before we cache the update.
            bpf::Path servicePath = bp::paths::getServiceDirectory() / name;
            synopsis.m_isUpdate = bpf::isDirectory(servicePath);
            
            m_locDescs.push_back(synopsis);
            fetchLocalization(acp);
        }
    }
}


void
ServiceQuery::parseLocalization(const unsigned char* buf,
                                size_t len)
{
    if (m_locDescs.size() == 0) {
        std::string msg("internal error during localization, "
                         "parseLocalization called with an "
                         "empty localized list");
        BPLOG_ERROR_STRM(this << ": " << msg);
        transactionFailed(msg);
        return;
    }

    ServiceSynopsis & ld = m_locDescs.back();    
    BPLOG_INFO_STRM(this << ": parsing synopsis for "
                    << ld.m_name << ": " << ld.m_version);

    // now buf contains a synopsis.bpkg  we must validate and parse it
    // up, appending the information contained within to 
    // m_locDescs.back().
    std::string tmpStr((const char*) buf, len);
    bpf::Path pkg = bpf::getTempPath(bpf::getTempDirectory(), "synopsisPkg");
    if (!bp::strutil::storeToFile(pkg, tmpStr)) {
        std::stringstream ss;
        ss << "couldn't save synopsis to temp file: " << pkg;
        BPLOG_ERROR_STRM(this << ": " << ss.str());
        transactionFailed(ss.str());
        return;
    }
    std::string synopsisStr;
    std::string errMsg;
    BPTime ts;
    if (!bp::pkg::unpackToString(pkg, synopsisStr, ts, errMsg)) {
        std::stringstream ss;
        ss << "couldn't unpack synopsis: " << errMsg;
        BPLOG_ERROR_STRM(this << ": " << ss.str());
        (void) bpf::remove(pkg);
        transactionFailed(ss.str());
        return;
    }
    (void) bpf::remove(pkg);
    
    // now we've got json.  let's parse it.
    bp::Object * o = bp::Object::fromPlainJsonString(synopsisStr);
    if (o == NULL) {
        BPLOG_ERROR_STRM(this << ": couldn't parse synopsis JSON");
        (void) bpf::remove(pkg);
        transactionFailed("couldn't parse synopsis JSON");
        return;
    }

    // kewl.  let's parse out what we care about.
    if (o->has("localizations", BPTMap)) {
        const bp::Object * localizations = o->get("localizations");
        // find the correct localization
        std::vector<std::string> localeCands =
            bp::localization::getLocaleCandidates(m_locale);
        unsigned int i;
        for (i = 0; i < localeCands.size(); i++) {
            if (localizations->has(localeCands[i].c_str(), BPTMap)) {
                const bp::Object * locale =
                    localizations->get(localeCands[i].c_str());                

                bool gotIt = false;
                if (locale->has("title", BPTString)) {
                    ld.m_title = (std::string) *(locale->get("title"));
                    gotIt = true;
                }
                if (locale->has("summary", BPTString)) {
                    ld.m_summary = (std::string) *(locale->get("summary"));
                    gotIt = true;
                }

                if (gotIt) break;
            }
        }
        if (i == localeCands.size()) {
            BPLOG_WARN_STRM(this << ": no suitable localization found in synopsis: "
                            << ld.m_name << ": " << ld.m_version);
        }
    } else {
        BPLOG_WARN_STRM(this << ": synopsis missing title/summary localizations: "
                        << ld.m_name << ": " << ld.m_version);
    }

    if (o->has("permissions", BPTList)) {
        std::vector<const bp::Object *> perms = *(o->get("permissions"));
        for (unsigned int i = 0; i < perms.size(); i++) {
            if (perms[i]->type() == BPTString) {
                ld.m_permissions.insert(*(perms[i]));
            } else {
                BPLOG_WARN_STRM(this << ": synopsis permissions list contains non-strings"
                                << ld.m_name << ": " << ld.m_version);
            }
        }
        
    }
    
    delete o;
    
    // done!  now move on down the list
    getNextLocalization();
}


void
ServiceQuery::satisfyRequirements(
    std::string platform,
    const std::list<ServiceRequireStatement> & requirements,
    const std::list<bp::service::Summary> & installed)
{
    m_type = SatisfyRequirements;
    m_platform = platform;
    m_requirements = requirements;
    m_installed = installed;
    m_qc.serviceList(platform);
}

void
ServiceQuery::fetchDetails(const AvailableService & acp)
{
    std::string url = WSProtocol::buildURL(acp.serverURL,
                                           WSProtocol::SERVICE_METADATA_PATH);
    url += "/" + m_name + "/" + m_version + "/" + m_platform;

    bp::http::RequestPtr req(WSProtocol::buildRequest(url));
    m_httpTransaction.reset(new bp::http::client::Transaction(req));
    BPLOG_INFO_STRM(this << ": initiate GET to fetch service details for " 
                    << m_name << "/" << m_version << "/" << m_platform);
    m_httpTransaction->initiate(shared_from_this());
}


void
ServiceQuery::fetchLocalization(const AvailableService & acp)
{
    std::string url =
        WSProtocol::buildURL(acp.serverURL,
                             WSProtocol::SERVICE_SYNOPSIS_PATH);
    url += "/" + acp.name + "/" + acp.version.asString() + "/"
        + m_platform;

    bp::http::RequestPtr req(WSProtocol::buildRequest(url));
    m_httpTransaction.reset(new bp::http::client::Transaction(req));
    BPLOG_INFO_STRM(this << ": initiate GET to fetch service localization for " 
                    << acp.name << "/" << acp.version.asString()
                    << "/" << m_platform);
    m_httpTransaction->initiate(shared_from_this());
}


void
ServiceQuery::startDownload(const AvailableService & acp)
{
    m_dlSize = acp.sizeBytes;
    m_lastPct = 0;
    m_zeroPctSent = false;
    
    std::string url = WSProtocol::buildURL(acp.serverURL,
                                            WSProtocol::SERVICE_DOWNLOAD_PATH);
    url += "/" + acp.name + "/" + acp.version.asString() + "/" + m_platform;

    bp::http::RequestPtr req(WSProtocol::buildRequest(url));
    req->headers.add("Accept", "application/octet-stream");
    m_httpTransaction.reset(new bp::http::client::Transaction(req));
    BPLOG_INFO_STRM(this << ": initiate GET to start download of  " 
                    << acp.name << "/" << acp.version.asString()
                    << "/" << m_platform);
    m_httpTransaction->initiate(shared_from_this());
}


void
ServiceQuery::updateCache(
    std::string platform,
    const std::list<ServiceRequireStatement> & requirements,
    const std::list<bp::service::Summary> & installed)
{
    m_type = UpdateCache;
    m_platform = platform;
    m_requirements = requirements;
    m_installed = installed;
    m_qc.serviceList(platform);
}


void
ServiceQuery::latestPlatformVersion(std::string platform)
{
    m_type = LatestPlatformVersion;
    m_platform = platform;
    m_qc.latestPlatformVersion(platform);
}


void
ServiceQuery::downloadLatestPlatform(std::string platform)
{
    m_type = DownloadLatestPlatform;
    m_platform = platform;
    m_lastPct = 0;
    m_zeroPctSent = false;
    m_dlSize = 0;
    
    // first we have to determine which server to download from,
    // then we'll go hit that server
    m_qc.latestPlatformVersion(platform);
}


void
ServiceQuery::onServiceList(const AvailableServiceList & list)
{
    switch (m_type) {
        case AttainServiceSynopses: {
            m_services = list;
            getNextLocalization();
            break;
        }
        case SatisfyRequirements: {
            AvailableServiceList need;
            if (!ServiceQueryUtil::findSatisfyingServices(
                    m_requirements, m_installed, list, false, need))
            {
                transactionFailed("unable to find satisfying services");
            }
            else
            {
                // now we know the minimal number of services we'll
                // need to download and install.  let's add in
                // potential updates, and see if it makes sense to
                // bundle any in. 
                std::list<bp::service::Summary> cachedList = 
                    PendingUpdateCache::cached();
                    
                if (cachedList.size() > 0) {
                    AvailableServiceList cached = 
                        ServiceQueryUtil::serviceSummaryToACL(cachedList);

                    AvailableServiceList combined = list;
                    combined.insert(combined.begin(),
                                    cached.begin(), cached.end());
                        
                    (void) ServiceQueryUtil::findSatisfyingServices(
                        m_requirements, m_installed, combined, true, need);
                }

                ServiceList clist =
                    ServiceQueryUtil::reformatAvailableServiceList(need);

                if (m_listener) {
                    m_listener->onRequirementsSatisfied(this, clist);
                }
            }
            break;
        }
        case AvailableServices:
            if (m_listener) m_listener->gotAvailableServices(this, list);
            break;
        case FindService: {
            AvailableService acp;

            if (!ServiceQueryUtil::findBestMatch(m_name, m_version,
                                                 m_minversion, list, acp))
            {
                transactionFailed("unable to find best match");
            }
            else if (m_listener != NULL)
            {
                m_listener->onServiceFound(this, acp);
            }
            break;
        }
        case ServiceDetails: {
            AvailableService acp;

            if (!ServiceQueryUtil::findBestMatch(m_name, m_version,
                                                 m_minversion, list, acp))
            {
                transactionFailed("unable to find best match");
            }
            else 
            {
                // now we must hit server again for the details,
                // the serverURL in acp tells us which server the
                // service resides on.
                fetchDetails(acp);
            }
            break;
        }
        case Download: {
            AvailableService acp;

            if (!ServiceQueryUtil::findBestMatch(m_name, m_version,
                                                 m_minversion, list, acp))
            {
                transactionFailed("unable to find best match");
            }
            else 
            {
                // now we must hit server again for the download,
                // the serverURL in acp tells us which server the
                // service resides on.
                startDownload(acp);
                break;
            }
            break;
        }
        case UpdateCache: {
            // now we have all of the required inputs, let's
            // determine what services we need to download
            std::list<bp::service::Summary> got =
                PendingUpdateCache::cached();

            // merge the cached installed services into one list.  all
            // should be considered "installed" for the purposes of
            // determining updates.
            got.insert(got.begin(), m_installed.begin(), m_installed.end());
            
            m_updates.clear();
            ServiceQueryUtil::findSatisfyingServices(
                m_requirements, got, list, true, m_updates);

            if (m_updates.size() > 0) {
                m_currentUpdate = m_updates.begin();
                startDownload(*m_currentUpdate);
            } else if (m_listener) {
                m_listener->onCacheUpdated(this, ServiceList());
            }
            break;
        }
        case LatestPlatformVersion:
        case DownloadLatestPlatform:
        case None:
            BPLOG_ERROR_STRM(this << ": Internal error, uexpected m_type!?");
    }
}

void
ServiceQuery::onServiceListFailure()
{
    transactionFailed("service list failure");
}

void
ServiceQuery::transactionFailed(const std::string& msg)
{
    if (m_listener) m_listener->onTransactionFailed(this, msg);
}

void
ServiceQuery::onLatestPlatform(const LatestPlatformServerAndVersion & latest)
{
    if (m_type == LatestPlatformVersion) {
        if (m_listener) {
            m_listener->gotLatestPlatformVersion(
                this, latest.version.asString());
        }
    } else if (m_type == DownloadLatestPlatform) {
        // hang onto this for later
        m_version = latest.version.asString();
        m_dlSize = latest.size;

        // now we've found the latest version and the server that
        // holds it, and start an http transaction to download it 
        std::string url =
            WSProtocol::buildURL(latest.serverURL,
                                 WSProtocol::LATEST_PLATFORM_UPDATE_PATH);
        url += "/" + m_platform;

        bp::http::RequestPtr req(WSProtocol::buildRequest(url));
        m_httpTransaction.reset(new bp::http::client::Transaction(req));
        BPLOG_INFO_STRM(this << ": initiate GET of latest platform for "
                        << m_platform);
        m_httpTransaction->initiate(shared_from_this());
    } else {
        std::string msg("Internal error, unexpected m_type for LatestPlatformVersion event!?");
        BPLOG_ERROR_STRM(this << ": " << msg);
        transactionFailed(msg);
    }
}

void
ServiceQuery::onLatestPlatformFailure()
{
    transactionFailed("failed to get latest platform version");
}
