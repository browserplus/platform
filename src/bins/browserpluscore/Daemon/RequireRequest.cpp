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
* RequireRequest.cpp
 */

#include "RequireRequest.h"
#include <sstream>
#include "ActiveSession.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/bpphash.h"
#include "BPUtils/bprandom.h"
#include "BPUtils/OS.h"
#include "BPUtils/ProductPaths.h"
#include "ServiceInstaller.h"
#include "Permissions/Permissions.h"
#include "PlatformUpdater.h"

using namespace std;
using namespace std::tr1;


const char* RequireRequest::kPlatformDescriptionKey = 
    "RequireRequest::kPlatformDescriptionKey";

const char* RequireRequest::kRequireStatementsKey = 
    "RequireRequest::kRequireStatementsKey";

const char* RequireRequest::kVersionSeparator = "/";

RequireRequest::RequireRequest(
    const list<ServiceRequireStatement>& requires,
    BPCallBack progressCallback,
    weak_ptr<ActiveSession> activeSession,
    unsigned int tid,
    const string & primaryServer,
    const list<string> & secondaryDistroServers)
: m_requires(requires), m_toInstall(), m_updatesOnly(false),
  m_platformUpdates(), m_platformUpdateDescriptions(),
  m_descriptions(), 
  m_currentInstall(), m_installSize(0), m_installedSize(0), m_smmTid(tid), 
  m_distTid(0), m_installTid(0), m_promptCookie(0),
  m_zeroTotalProgressPosted(false),
  m_progressCB(progressCallback), m_activeSession(activeSession),
  m_distQuery(NULL)
{
    shared_ptr<ActiveSession> asp = m_activeSession.lock();
    if (asp != NULL) {
        m_registry = asp->registry();
        m_locale = asp->locale();
    }
          
    // Setup the distribution server urls.
    list<string> distroServers = secondaryDistroServers;
    distroServers.push_front(primaryServer);
    
    m_distQuery = new DistQuery(distroServers, PermissionsManager::get());
    m_distQuery->setListener(this);
}


RequireRequest::~RequireRequest()
{    
    delete m_distQuery;

    // always release lock in case we acquired it but are 
    // being destroyed prior to getting the event.
    // releasing a lock we don't hold is a no-op
    RequireLock::releaseLock(m_thisWeak);
}


void
RequireRequest::run()
{
    // will get RequireLock::LockAttainedEvent when it's our turn
    BPLOG_DEBUG_STRM(this << " require, smm_tid = " 
                     << m_smmTid << " asks for lock");
    m_thisWeak = shared_from_this();
    RequireLock::attainLock(m_thisWeak);
}


bool
RequireRequest::checkDomainPermission(const std::string& permission)
{
    shared_ptr<ActiveSession> asp = m_activeSession.lock();

    if (asp == NULL) {
        return false;
    }
    
    // does domain/permission need approval?
    std::string domain = asp->domain();
    if (domain.compare("unknown") == 0) {
        return false;
    }
    PermissionsManager* pmgr = PermissionsManager::get();
    switch (pmgr->queryDomainPermission(domain, permission)) {
        case PermissionsManager::eAllowed:
            // empty
            break;
        case PermissionsManager::eNotAllowed:
            return false;
        case PermissionsManager::eUnknown:
            switch (asp->transientPermission(permission)) {
                case PermissionsManager::eAllowed:
                    // empty
                    break;
                case PermissionsManager::eNotAllowed:
                    return false;
                case PermissionsManager::eUnknown:    
                    // permission will be localized by promptUser()
                    m_permissions.insert(permission);
                    break;
            }
    }
    return true;
}


bool 
RequireRequest::silentPlatformUpdate(const std::string& version)
{
    shared_ptr<ActiveSession> asp = m_activeSession.lock();
    if (asp == NULL) {
        BPLOG_WARN_STRM("no active session");
        return false;
    }
    std::string domain = asp->domain();
    
    PermissionsManager* pmgr = PermissionsManager::get();
    PermissionsManager::Permission perm = pmgr->queryAutoUpdatePlatform(domain);
    if (perm == PermissionsManager::eAllowed) {
        BPLOG_INFO_STRM("silent platform update " << version
                        << " for " << domain);
        PlatformUpdater::spawnUpdate(version);
        return true;
    }
    return false;
}


bool 
RequireRequest::silentServiceUpdate(const std::string& service,
                                    const std::string& version)
{
    shared_ptr<ActiveSession> asp = m_activeSession.lock();
    if (asp == NULL) {
        BPLOG_WARN_STRM("no active session");
        return false;
    }
    std::string domain = asp->domain();
    
    PermissionsManager* pmgr = PermissionsManager::get();
    PermissionsManager::Permission perm = pmgr->queryAutoUpdateService(domain,
                                                                       service);
    if (perm == PermissionsManager::eAllowed) {
        if (m_distQuery->isCached(service, version)) {
            if (m_distQuery->installServiceFromCache(service, version)) {
                BPLOG_INFO_STRM("silent update of " << service
                                << "/" << version << " for domain " << domain);
                m_registry->forceRescan();
            } else {
                stringstream ss;
                ss << "error updating service " << service << " / " << version;
                BPLOG_WARN_STRM(m_smmTid << " Require fails: " << ss.str());
                postFailure("core.serverError", ss.str());
            }
            return true;
        }
    }
    return false;
}

void
RequireRequest::doRun()
{
    if (!checkDomainPermission(PermissionsManager::kAllowDomain)) {
        BPLOG_WARN_STRM(m_smmTid << " Require fails: blacklisted domain");
        postFailure("core.permissionsError", "blacklisted domain");
        return;
    }
    
    // platform updates?
    checkPlatformUpdates();
    vector<bp::ServiceVersion>::iterator ui = m_platformUpdates.begin();
    while (ui != m_platformUpdates.end()) {
        string version = ui->asString();
        
        // update silently if possible
        if (silentPlatformUpdate(version)) {
            ui = m_platformUpdates.erase(ui);
        } else {
            // will have to prompt user
            string summary;
            bp::localization::getLocalizedString(kPlatformDescriptionKey, 
                                                 m_locale,
                                                 summary,
                                                 ui->majorVer(),
                                                 ui->minorVer(),
                                                 ui->microVer(),
                                                 true);
            ServiceSynopsis synopsis;
            synopsis.m_name = "BrowserPlus";
            synopsis.m_version = version;
            synopsis.m_title = synopsis.m_name;
            synopsis.m_summary = summary;
            synopsis.m_sizeInBytes = 0;
            synopsis.m_isUpdate = true;
            m_platformUpdateDescriptions.push_back(synopsis);
            ++ui;
        }
    }
    
    // can we satisfy everything?
    bool haveAllServices = true;
    list<ServiceRequireStatement>::const_iterator it;

    // First check installed services for missing needed provider services.
    // If any providers are missing, add them to the request.
    list<ServiceRequireStatement> toAdd;
    for (it = m_requires.begin(); it != m_requires.end(); ++it) {
        bp::service::Summary summary;
        if (m_registry->summary(it->m_name, it->m_version,
                                it->m_minversion, summary)) {
            string providerName = summary.usesService();
            if (!providerName.empty()) {
                string providerVersion = summary.usesVersion().asString();
                string providerMinversion = summary.usesMinversion().asString();
                ServiceRequireStatement rs = {providerName, providerVersion,
                                              providerMinversion};
                bp::service::Summary providerSummary;
                if (!m_registry->summary(providerName, providerVersion,
                                         providerMinversion, providerSummary)) {
                    BPLOG_INFO_STRM(m_smmTid << (it->m_name) << " - "
                                    << (it->m_version) << " - "
                                    << (it->m_minversion)
                                    << " is missing provider "
                                    << providerName << " - "
                                    << providerVersion << " - "
                                    << providerMinversion
                                    << ", adding to requirements");
                    toAdd.push_back(rs);
                }
            }
        }
    }
    m_requires.splice(m_requires.end(), toAdd);
        
    // now see if we can satisfy the request
    for (it = m_requires.begin(); it != m_requires.end(); ++it) {
        bp::service::Summary summary;
        if (m_registry->summary(it->m_name, it->m_version,
                                it->m_minversion, summary)) {
            // do we need permissions?
            std::set<std::string> perms = summary.permissions();
            std::set<std::string>::const_iterator it;
            for (it = perms.begin(); it != perms.end(); ++it) {
                if (!checkDomainPermission(*it)) {
                    BPLOG_WARN_STRM(m_smmTid << " Require fails: "
                                    << "permission '" << *it << "' denied");
                    postFailure("core.permissionError",
                                "permission '" + *it + "' denied");
                    return;
                }
            }
        } else {
            // info logging good here, this is not the common case
            BPLOG_INFO_STRM(m_smmTid << " no active service which satisfies: "
                            << (it->m_name) << " - "
                            << (it->m_version) << " - "
                            << (it->m_minversion));
            haveAllServices = false;
        }
    }
    
    // What's installed?
    list<bp::service::Summary> installed =
        m_registry->availableServiceSummaries();
    
    std::string platform = bp::os::PlatformAsString();
    if (haveAllServices) {
        // at most, we have updates or permissions
        m_updatesOnly = true;
        m_toInstall = m_distQuery->haveUpdates(m_requires, installed);
        
        // silently install approved service updates
        ServiceList::iterator it = m_toInstall.begin();
        while (it != m_toInstall.end()) {
            if (silentServiceUpdate(it->first, it->second)) {
                it = m_toInstall.erase(it);
            } else {
                ++it;
            }
        }
        
        if (m_toInstall.empty()) {
            // no service updates, may have platform updates
            updateRequireHistory(m_requires);
            if (m_permissions.empty() && m_platformUpdates.empty()) {
                // we're golden
                postSuccess();
                return;
            }
            promptUser();
            return;
        }
        m_distTid = m_distQuery->serviceSynopses(
            platform, m_locale, m_toInstall);

        if (m_distTid == 0) {
            BPLOG_WARN_STRM(m_smmTid
                            << " Require fails: getting service descriptions");
            postFailure("core.distConnError",
                        "error getting service descriptions");
            return;
        }
    } else {
        // Gotta download and install some services
        m_distTid = m_distQuery->satisfyRequirements(platform, m_requires,
                                                     installed);
        if (m_distTid == 0) {
            BPLOG_WARN_STRM(m_smmTid << " Require fails: couldn't "
                            << "initiate requirement satisfaction");
            postFailure("core.distConnError",
                        "error getting service requirements");
            return;
        }
    }

    // will get events via localizedDescriptions() and satisfyRequirements()
}


ServiceSynopsis
RequireRequest::getDescription(const std::string & name,
                               const std::string & /*version*/)
{
    std::list<ServiceSynopsis>::iterator di;
    for (di = m_descriptions.begin(); 
         di != m_descriptions.end(); ++di) {
        if (di->m_name.compare(name) == 0) {
            return *di;
            break;
        }
    }
    return ServiceSynopsis();
}


void 
RequireRequest::postProgress(const std::string & nameArg,
                             const std::string & versionArg,
                             int localPercent)
{
    shared_ptr<ActiveSession> asp = m_activeSession.lock();

    if (asp == NULL) {
        return;
    }
        
    if (m_progressCB != 0) {
        unsigned int localSize = 0; 
        std::string name, version, displayName;
        ServiceSynopsis synopsis = getDescription(nameArg, versionArg);
        if (!synopsis.m_name.empty()) {
            name = synopsis.m_name;
            version = synopsis.m_version;
            displayName = synopsis.m_title;
            localSize = synopsis.m_sizeInBytes ? synopsis.m_sizeInBytes : 1;
        } else {
            name = nameArg;
            version = versionArg;
            displayName = nameArg;
        }
        if (localPercent > 100) localPercent = 100;
        unsigned int localDownloadedSize = (unsigned int) ((float) localPercent/100 * localSize);
        if (localDownloadedSize > localSize) localDownloadedSize = localSize;
        unsigned int downloadedSize = m_installedSize + localDownloadedSize;
        int totalPercent = (int) (((float) downloadedSize / m_installSize) * 100);
        if (totalPercent > 100) totalPercent = 100;
        if (localPercent == 100) {
            m_installedSize += localSize;
        }
        
        // guarantee that 0/100 totalPercent only happen once
        if (totalPercent == 0) {
            if (m_zeroTotalProgressPosted) return;
            m_zeroTotalProgressPosted = true;
            m_lastPostTimer.reset();
            m_lastPostTimer.start();            
        }
        bool lastPost = (localPercent == 100) && (m_toInstall.empty());
        if (lastPost) {
            totalPercent = 100;
        } else if (totalPercent == 100) {
            totalPercent = 99;
        }
        
        // now throttle to once per 1/4 second, but we always
        // post 0/100 percent events
        if (localPercent != 0 && localPercent != 100
            && totalPercent != 0 && totalPercent != 100)
        {
            if (m_lastPostTimer.elapsedSec() < 0.250) {
                return;
            }
            m_lastPostTimer.reset();
            m_lastPostTimer.start();            
        }
        
        bp::Map* cbArgs = new bp::Map;
        cbArgs->add("name", new bp::String(name));
        cbArgs->add("version", new bp::String(version));
        cbArgs->add("displayName", new bp::String(displayName));
        cbArgs->add("localPercentage", new bp::Integer(localPercent));
        cbArgs->add("totalPercentage", new bp::Integer(totalPercent));        
        bp::Map* cbInfo = new bp::Map;
        cbInfo->add("callback", new bp::Integer(m_progressCB));
        cbInfo->add("parameters", cbArgs);
        asp->invokeCallback(m_smmTid, cbInfo);
        delete cbInfo;
    }
}


void
RequireRequest::checkPlatformUpdates()
{
    using namespace bp::file;
    
    bp::ServiceVersion thisVersion;
    bool b = thisVersion.parse(bp::paths::versionString());
    BPASSERT(b);
    if (!b) {
        BP_THROW_FATAL("couldn't parse version from bp::paths::versionString()");
    }
    
    
    // Get all subdirs of platform cache dir
    vector<string> updates;
    tDirIter end;
    Path platCache = bp::paths::getPlatformCacheDirectory();
    if (isDirectory(platCache)) {
        try {
            for (tDirIter iter(platCache); iter != end; ++iter) {
                updates.push_back(utf8FromNative(iter->path().filename()));;
            }
        } catch (const tFileSystemError& e) {
            BPLOG_WARN_STRM("unable to iterate thru " << platCache
                            << ": " << e.what());
        }
    }
    vector<string>::iterator it = updates.begin();
    while (it != updates.end()) {
        bp::ServiceVersion v;
        if (v.parse(*it)) {
            // remove any updates which have already been installed or are 
            // older than our current version
            Path installedPath = bp::paths::getBPInstalledPath(v.majorVer(),
                                                               v.minorVer(), 
                                                               v.microVer());
            bool alreadyInstalled = bp::file::exists(installedPath);
            bool olderThanCurrent = (v.compare(thisVersion) < 0);
            if (alreadyInstalled || olderThanCurrent) {
                BPLOG_INFO_STRM("version " << *it
                                << (alreadyInstalled ? "already installed" 
                                                     : "older than current version")
                                << ", removing update");
                Path updatePath = bp::paths::getPlatformCacheDirectory() / *it;
                bp::file::remove(updatePath);
                it = updates.erase(it);
                continue;
            } 
        } else {
            // ignore things which don't appear to be updates
            BPLOG_INFO_STRM("update " << *it << " malformed, ignored");
            it = updates.erase(it);
            continue;
        }
        ++it;
    }
    
    // if we have updates, get the latest in each 
    // major rev and delete the others
    if (!updates.empty()) {
        vector<bp::ServiceVersion> toDelete;
        if (updates.size() == 1) {
            // easy case, only one update
            bp::ServiceVersion v;
            if (v.parse(updates[0]) 
                && !PlatformUpdater::updateSpawned(v.asString())) {
                m_platformUpdates.push_back(v);
            }
        } else {
            // harder case, find latest in each major rev and "outdated" ones
            map<int, bp::ServiceVersion> m;
            for (unsigned int i = 0; i < updates.size(); i++) {
                bp::ServiceVersion v;
                if (!v.parse(updates[i])
                    || PlatformUpdater::updateSpawned(v.asString())) {
                    continue;
                }
                int major = v.majorVer();
                map<int, bp::ServiceVersion>::iterator it = m.find(major);
                if (it == m.end()) {
                    m[major] = v;
                } else {
                    if (v.compare(it->second) == 1) {
                        toDelete.push_back(it->second);
                        m[major] = v;
                    } else {
                        toDelete.push_back(v);
                    }
                }
            }
            map<int, bp::ServiceVersion>::iterator it;
            for (it = m.begin(); it != m.end(); ++it) {
                m_platformUpdates.push_back(it->second);
            }
        }
        for (unsigned int i = 0; i < m_platformUpdates.size(); i++) {
            BPLOG_INFO_STRM("found platform update "
                            << m_platformUpdates[i].asString());
        }
        for (unsigned int i = 0; i < toDelete.size(); i++) {
            Path p = bp::paths::getPlatformCacheDirectory() / toDelete[i].asString();
            bp::file::remove(p);
            BPLOG_INFO_STRM("delete outdated platform update "
                            << toDelete[i].asString());
        }
    }
}


void
RequireRequest::installNextService()
{
    // first handle platform updates
    for (unsigned int i = 0; i < m_platformUpdates.size(); i++) {
        string plat = m_platformUpdates[i].asString();
        PlatformUpdater::spawnUpdate(plat);
    }
    m_platformUpdates.clear();
    
    // now work on services
    if (!m_toInstall.empty()) {
        // initiate next update or download/install
        pair<std::string, std::string> item = m_toInstall.front();
        m_toInstall.pop_front();
        m_distTid = 0;
        m_currentInstall = getDescription(item.first, item.second);

        if (m_distQuery->isCached(item.first, item.second)) {
            postProgress(item.first, item.second, 0);
            if (!m_distQuery->installServiceFromCache(item.first, item.second)) {
                stringstream ss;
                ss << "error updating service " << item.first
                   << " / " << item.second;
                BPLOG_WARN_STRM(m_smmTid << " Require fails: " << ss.str());
                postFailure("core.serverError", ss.str());
                return;
            }
            postProgress(item.first, item.second, 100);
            m_registry->forceRescan();
            installNextService();
        } else {
            m_installTid = ServiceInstaller::installService(
                item.first, item.second, bp::paths::getServiceDirectory(),
                shared_from_this());
            if (m_installTid == 0) {
                stringstream ss;
                ss << "error installing service " << item.first
                   << " / " << item.second;
                BPLOG_WARN_STRM(m_smmTid << " Require fails: " << ss.str());
                postFailure("core.canNotInstall", ss.str());
                return;       
            }
            postProgress(item.first, item.second, 0);
        }
    } else {
        // whee!  we're done
        updateRequireHistory(m_requires);
        postSuccess();
    }
}

void
RequireRequest::gotRequireLock()
{
    BPLOG_DEBUG_STRM(this << " got require lock, smmTid = " << m_smmTid);
    doRun();
}

void
RequireRequest::installStatus(unsigned int installId,
                              const std::string & name,
                              const std::string & version,
                              unsigned int pct)
{
    if (installId != m_installTid) {
        BPLOG_WARN_STRM(m_smmTid << " Got install progress event with "
                        << "unknown install tid: " << installId);
        return;
    }
        
    if (!(pct % 10)) {
        BPLOG_INFO_STRM(name << "/" << version << " is " << pct << "% down...");
    }
        
    // don't post 0/100 percent progress. they are handled by 
    // installNextService and our receipt of a InstalledEvent
    if (pct != 0 && pct != 100) postProgress(name, version, pct);
}


// invoked when installation is complete
void
RequireRequest::installed(unsigned int installId,
                          const std::string & name,
                          const std::string & version)
{
    if (installId != m_installTid) {
        BPLOG_WARN_STRM(m_smmTid << " Got Install event with unknown "
                        << "install tid: " << installId);
        return;
    }
        
    // successful installation! install the next service in the queue
    postProgress(name, version, 100);
    installNextService();
}

// invoked when installation fails
void
RequireRequest::installationFailed(unsigned int installId)
{
    if (installId != m_installTid) {
        BPLOG_WARN_STRM(m_smmTid << " Got Install event with unknown "
                        << "install tid: " << installId);
        return;
    }
        
    BPLOG_WARN("Require fails, could not install service");
    postFailure("core.serverError", "could not attain service");
}

void
RequireRequest::onUserResponse(unsigned int cookie, const bp::Object & resp)
{
    using namespace bp;

    // get a strong pointer to active session for the duration of the
    // function
    shared_ptr<ActiveSession> asp = m_activeSession.lock();
    if (asp == NULL) {
        return;
    }

    // now parse out the user response.  
    std::string response;
    if (resp.type() == BPTString) {
        response = (std::string) resp;
    }
        
    if (cookie != m_promptCookie) {
        BPLOG_WARN_STRM(m_smmTid << " Got UserConfirm/DenyEvent event with "
                        << "unknown cookie: " << cookie);  
        return;
    }
        
    bool allow = (response.find("Allow") != std::string::npos);
    bool always = (response.find("Always") != std::string::npos);
        
    // if user said "always", update domain permissions, else
    // update session's transient permissions
    if (always) {
        std::string domain = asp->domain();
        PermissionsManager* pmgr = PermissionsManager::get();
        PermissionsManager::Permission perm = allow ? 
                                              PermissionsManager::eAllowed
                                              : PermissionsManager::eNotAllowed;

        if (!m_platformUpdates.empty()) {
            pmgr->setAutoUpdatePlatform(domain, perm);
        }

        ServiceList::const_iterator ci;
        for (ci = m_toInstall.begin(); ci != m_toInstall.end(); ++ci) {
            pmgr->setAutoUpdateService(domain, ci->first, perm);
        }
        
        std::set<std::string>::const_iterator si;
        for (si = m_permissions.begin(); si != m_permissions.end(); ++si) {
            const std::string& perm = *si;
            if (perm.compare(PermissionsManager::kAllowDomain) == 0) {
                if (allow) {
                    pmgr->allowDomain(domain);
                } else {
                    pmgr->disallowDomain(domain);
                }
            } else {
                if (allow) {
                    pmgr->addDomainPermission(domain, perm);
                } else {
                    pmgr->revokeDomainPermission(domain, perm);
                }
            }
        }
    } else {
        std::set<std::string>::const_iterator si;
        for (si = m_permissions.begin(); si != m_permissions.end(); ++si) {
            asp->setTransientPermission(*si, allow);
        }
    }
        
    if (allow) {
        installNextService();
    } else {
        if (m_permissions.size() > 0) {
            BPLOG_WARN("Require fails: permission denied");
            postFailure("core.permissionError",
                        "this page requires permissions which you have denied");
        } else if (m_updatesOnly) {
            // ok to reject updates only, we'll just use what we got
            m_distQuery->purgeCache();
            postSuccess();
        } else {
            BPLOG_WARN("Require fails: user refused installation");
            postFailure("core.userRefusedComponentInstallation",
                        "this page requires plugins which you do not wish to install");
        }
    }
}


void
RequireRequest::onTransactionFailed(unsigned int tid)
{
    if (tid == m_distTid) {
        BPLOG_WARN_STRM(m_smmTid << " Require fails: DistQuery error");
        postFailure("BP.requireError", "");
    } else {
        BPLOG_WARN_STRM(m_smmTid << " transaction failed with unknown tid "
                        << tid);
    }
    return;
}

void
RequireRequest::onRequirementsSatisfied(unsigned int tid,
                                        const ServiceList & clist)
{
    if (tid != m_distTid) {
        BPLOG_WARN_STRM(m_smmTid << " Got SatisfyRequirementsEvent "
                        << "with unknown id: " << tid);
        return;
    }
        
    if (clist.empty()) {
        BPLOG_WARN_STRM(m_smmTid << " Require fails: unavailable components");
        postFailure("core.serverError",
                    "this page requires components which are not available"
                    " for download");
        return;
    }
        
    // Got some services to update/install.  Get localized descriptions
    list<pair<std::string, std::string> >::const_iterator li;
    for (li = clist.begin(); li != clist.end(); ++li) {
        if (!silentServiceUpdate(li->first, li->second)) {
            m_toInstall.push_back(*li);
            BPLOG_INFO_STRM("queue " << li->first << "/" << li->second
                            << " for install/update");
        }
    }
    
    m_distTid = m_distQuery->serviceSynopses(bp::os::PlatformAsString(),
                                             m_locale, m_toInstall);
    if (m_distTid == 0) {
        BPLOG_WARN("Require fails: couldn't localize descriptions");
        postFailure("core.serverError",
                    "unable to request localized service descriptions");
        return;     
    }
}

void
RequireRequest::gotServiceSynopsis(unsigned int tid,
                                   const ServiceSynopsisList & sslist)
{
    if (tid != m_distTid) {
        BPLOG_WARN_STRM(m_smmTid
                        << " Got LocalizedDescriptionsEvent event with "
                        << "unknown tid: " << tid);  
        postFailure("core.serverError",
                    "internal error when localizing service descriptions");
        return;
    }

    std::list<ServiceSynopsis>::const_iterator di;
    for (di = sslist.begin(); di != sslist.end(); ++di) {
        m_descriptions.push_back(*di);
    }
    promptUser();
}


void 
RequireRequest::updateRequireHistory(
    const std::list<ServiceRequireStatement> & requires)
{
    std::list<ServiceRequireStatement>::const_iterator it;
    for (it = requires.begin(); it != requires.end(); ++it) {
        string name(it->m_name);
        string version(it->m_version);
        string minversion(it->m_minversion);
        
        string mapStr;
        bp::Map* m = NULL;
        if (bp::phash::get(kRequireStatementsKey, mapStr)) {
            m = dynamic_cast<bp::Map*>(bp::Object::fromPlainJsonString(mapStr));
        }
        if (!m) m = new bp::Map;
        if (!m->has(name.c_str(), BPTList)) {
            m->add(name, new bp::List);
        }
        const bp::List* l = dynamic_cast<const bp::List*>(m->get(name.c_str()));
        string ourVersion = version + kVersionSeparator + minversion;
        bool found = false;
        for (unsigned int i = 0; i < l->size(); ++i) {
            const bp::String* s = dynamic_cast<const bp::String*>(l->value(i));
            if (!s) continue;
            if (ourVersion.compare(s->value()) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            const_cast<bp::List*>(l)->append(new bp::String(ourVersion));
            BPLOG_INFO_STRM("added to update require history: "
                            << name << "(" << version 
                            << "/" << minversion << ")");
            bp::phash::set(kRequireStatementsKey, m->toPlainJsonString());
        }
        delete m;
    }
}


void
RequireRequest::promptUser()
{
    shared_ptr<ActiveSession> asp = m_activeSession.lock();
    if (asp == NULL) {
        return;
    }
    
    // check permissions for each service in m_descriptions, adding
    // those which need approval to m_permissions
    std::list<ServiceSynopsis>::const_iterator si;
    for (si = m_descriptions.begin(); si != m_descriptions.end(); ++si)
    {
        std::set<std::string> perms = si->m_permissions;
        std::set<std::string>::const_iterator it;
        for (it = perms.begin(); it != perms.end(); ++it) {
            if (!checkDomainPermission(*it)) {
                BPLOG_WARN_STRM(m_smmTid << " Require failes, permission "
                                << "denied: " << *it);
                postFailure("core.permissionError",
                            "permission '" + *it + "' denied");
                return;
            }
        }
    }
    
    vector<string> perms;
    if (m_permissions.size() > 0) {
        // localize permissions, ensure that "domain may use bp" appears first
        string locale = m_locale;
        PermissionsManager* pmgr = PermissionsManager::get();
        if (m_permissions.count(PermissionsManager::kAllowDomain) > 0) {
            perms.push_back(pmgr->getLocalizedPermission(PermissionsManager::kAllowDomain,
                                                         locale));
        }
        set<string>::const_iterator it;
        for (it = m_permissions.begin(); it != m_permissions.end(); ++it) {
            if (it->compare(PermissionsManager::kAllowDomain) == 0) {
                continue;
            }
            perms.push_back(pmgr->getLocalizedPermission(*it, locale));
        }
    }

    // get install/update sizes.  we say that an update has a size of
    // 1 byte to keep the callback percentage calculations sane
    std::list<ServiceSynopsis>::const_iterator di;
    m_installSize = 0;
    for (di = m_descriptions.begin(); 
         di != m_descriptions.end(); ++di) {
        if (di->m_sizeInBytes > 0) {
            m_installSize += di->m_sizeInBytes;
        } else {
            m_installSize++;
        }
    }

    // build up a list of services we must prompt the user for
    std::list<ServiceSynopsis> servicesToPromptFor;    
    {
        std::list<ServiceSynopsis>::iterator it;
        shared_ptr<ActiveSession> asp = m_activeSession.lock();
        if (asp == NULL) {
            BPLOG_WARN_STRM("no active session");
            return;
        }
        std::string domain = asp->domain();
        PermissionsManager* pmgr = PermissionsManager::get();
        for (it = m_descriptions.begin();  it != m_descriptions.end(); ++it) {
            if (pmgr->queryAutoUpdateService(domain, it->m_name) !=
                PermissionsManager::eAllowed)
            {
                servicesToPromptFor.push_back(*it);
            }
        }
    }

    // if there's nothing to prompt for, we're done
    // otherwise, pester the user
    if (perms.empty() && m_platformUpdateDescriptions.empty()
        && servicesToPromptFor.empty())
    {
        if (m_toInstall.empty()) postSuccess();
        else installNextService();
    } else {
        m_promptCookie = bp::random::generate();
        asp->displayInstallPrompt(shared_from_this(), m_promptCookie, perms,
                                  m_platformUpdateDescriptions,
                                  servicesToPromptFor);
    }
}


void 
RequireRequest::postSuccess()
{
    bp::List rlist;
    list<ServiceRequireStatement>::const_iterator li;
    for (li = m_requires.begin(); li != m_requires.end(); ++li) {
        bp::service::Description desc;
        if (!m_registry->describe(li->m_name, li->m_version,
                                  li->m_minversion, desc))
        {
            // this should not happen
            BPLOG_ERROR_STRM(
                "RequireRequest::postSuccess(), no description for "
                << li->m_name << "/" << li->m_version << "/"
                << li->m_minversion);
            postFailure("core.serverError", "could not attain service");
            return;
        }
        bp::Object* obj = desc.toBPObject();
        rlist.append(obj);
    }

    // let the next guy have a shot, call BEFORE we invoke our listener,
    // as our listener may decide to delete us blowing away the this pointer
    RequireLock::releaseLock(shared_from_this());

    // let listener know we're all done
    shared_ptr<IRequireListener> listener = m_listener.lock();
    if (listener) listener->onComplete(m_smmTid, rlist);
}


void 
RequireRequest::postFailure(const std::string& error,
                            const std::string& verboseError)
{
    // let the next guy have a shot, call BEFORE we invoke our listener,
    // as our listener may decide to delete us blowing away the this pointer
    RequireLock::releaseLock(shared_from_this());

    shared_ptr<IRequireListener> listener = m_listener.lock();
    if (listener) {
        listener->onFailure(m_smmTid, error, verboseError);
    }
}

void 
RequireRequest::setListener(weak_ptr<IRequireListener> listener)
{
    m_listener = listener;
}
