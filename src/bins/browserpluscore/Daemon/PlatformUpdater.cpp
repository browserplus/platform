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
 * PlatformUpdater.h - A singleton responsible for periodic platform update
 *
 * Created by Gordon Durand on Wed March 26th 2008.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "PlatformUpdater.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpphash.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bptimer.h"
#include "BPUtils/OS.h"
#include "BPUtils/ProductPaths.h"
#include "DistributionClient/DistributionClient.h"
#include "Permissions/Permissions.h"

#include <iostream>

using namespace std;
using namespace bp::process;
using namespace bp::file;
using namespace bp::paths;

static const char* s_lastCheckTSKey = "PlatformUpdater::lastCheckTS"; 

class PlatformUpdaterSingleton : virtual public bp::time::ITimerListener,
                                 virtual public IDistQueryListener
{
public:
    PlatformUpdaterSingleton(list<string> distroServers,
                             unsigned int poll);
    ~PlatformUpdaterSingleton();

    void start();
    void stop();
    bool busy();
    bool spawnUpdate(const string& version);
    bool updateSpawned(const string& version);

private:
    // implemented methods from IDistQueryListener inteface
    void gotLatestPlatformVersion(unsigned int tid, const std::string & latest);
    void onLatestPlatformDownloaded(
        unsigned int tid,
        const LatestPlatformPkgAndVersion & pkgAndVersion);
    void onTransactionFailed(unsigned int tid);

    bool installToCache(const LatestPlatformPkgAndVersion & platformInfo);
    unsigned int m_pollPeriod;
    DistQuery * m_distQuery;
    std::string m_platform;
    bp::time::Timer m_timer;
    unsigned int m_tid;
    bool m_busy;
    set<string> m_updates;
    
    void timesUp(bp::time::Timer * t);
};

static PlatformUpdaterSingleton * s_updater = NULL;

void
PlatformUpdater::startup(list<string> distroServers,
                         unsigned int poll)
{
    BPASSERT(s_updater == NULL);
    s_updater = new PlatformUpdaterSingleton(distroServers, poll);
    s_updater->start();
}


void
PlatformUpdater::shutdown()
{
    if (s_updater != NULL) {
        s_updater->stop();
        delete s_updater;
        s_updater = NULL;
    }
}


bool 
PlatformUpdater::spawnUpdate(const string& version)
{
    BPASSERT(s_updater != NULL);
    return s_updater->spawnUpdate(version);
}


bool
PlatformUpdater::updateSpawned(const string& version)
{
    BPASSERT(s_updater != NULL);
    return s_updater->updateSpawned(version);
}


bool
PlatformUpdater::isBusy()
{
    return((s_updater == NULL) ? false : s_updater->busy());
}


PlatformUpdaterSingleton::PlatformUpdaterSingleton(list<string> distroServers,
                                                   unsigned int poll)
{
    m_pollPeriod = poll;
    m_busy = false;
    m_tid = 0;
    m_platform = bp::os::PlatformAsString();
    m_distQuery = new DistQuery(distroServers, PermissionsManager::get());
    if (m_distQuery != NULL) {
        m_distQuery->setListener(this);
    } else {
        BPLOG_ERROR("unable to get distQuery");
    }
}


PlatformUpdaterSingleton::~PlatformUpdaterSingleton()
{
    delete m_distQuery;
}


void
PlatformUpdaterSingleton::start()
{
    if (m_distQuery == NULL) {
        return;
    }
    
    bool mustCheck = false;
    string lastCheck;
    BPTime last(0L);
    BPTime now;

    try {
        if (bp::phash::get(s_lastCheckTSKey, lastCheck)) {
            // if lastcheck is malformed, this may throw
            last.set(BPTime(lastCheck));
            if (now.compare(last) < 0) {
                throw runtime_error("last check is in future");
            }
        }
    } catch (const runtime_error&) {
        // force an update
        last.set(0);
    }
    
    unsigned long secsToNextCheck = m_pollPeriod;
    long diff = now.diffInSeconds(last);
    BPASSERT(diff >= 0);
    if (diff > (long) m_pollPeriod) {
        mustCheck = true;
    } else {
        secsToNextCheck = (m_pollPeriod - diff) + 2;
    }

    if (mustCheck) {
        BPLOG_INFO_STRM("last platform update check at "
                        << last.asString()
                        << " checking for updates now");
    } else {
        BPLOG_INFO_STRM("last platform update check at "
                        << last.asString()
                        << " checking again for updates in "
                        << secsToNextCheck << "s");
    }

    if (mustCheck) {
        m_tid = m_distQuery->latestPlatformVersion(m_platform);
        if (m_tid == 0) {
            BPLOG_ERROR("Unable to invoke DistQuery::latestPlatformVersion");
        }
    }

    // set the timer 
    m_timer.cancel();
    m_timer.setListener(this);
    m_timer.setMsec(1000 * secsToNextCheck);    
}    


void
PlatformUpdaterSingleton::stop()
{
    m_timer.cancel();
    m_updates.clear();
}


bool
PlatformUpdaterSingleton::busy()
{
    return((m_tid != 0) && !m_busy);
}


bool 
PlatformUpdaterSingleton::spawnUpdate(const string& version)
{
    if (updateSpawned(version)) {
        return true;
    }
    
    // ok, must spawn update process which will wait until daemon exit,
    // then do the update
    m_busy = true;
    m_updates.insert(version);
    string daemonLock = bp::paths::getIPCLockName();
    Path cacheDir = getPlatformCacheDirectory() / version;
    Path updater = cacheDir / "BrowserPlusUpdater";
    updater = canonicalProgramPath(updater);
    if (!exists(updater)) {
        return false;
    }
    spawnStatus status;
    vector<string> args;
    args.push_back(cacheDir.utf8());
    args.push_back(daemonLock);
    bool rval = spawn(updater, args, &status);
    m_busy = false;
    return rval;
}


bool 
PlatformUpdaterSingleton::updateSpawned(const std::string& version)
{
    return(m_updates.find(version) != m_updates.end());
}


bool 
PlatformUpdaterSingleton::installToCache(
    const LatestPlatformPkgAndVersion & platformInfo)
{
    m_busy = true;
    PlatformUnpacker unpacker(platformInfo.m_pkg, 
                              bp::paths::getPlatformCacheDirectory(),
                              platformInfo.m_version);
    string errMsg;
    bool rval = (unpacker.unpack(errMsg) && unpacker.install(errMsg));
    m_busy = false;
    return rval;
}


void
PlatformUpdaterSingleton::timesUp(bp::time::Timer *)
{
    start();
}

void
PlatformUpdaterSingleton::gotLatestPlatformVersion(unsigned int,
                                                   const std::string & latest)
{
    bp::ServiceVersion latestVersion;
    if (!latestVersion.parse(latest)) {
        BPLOG_WARN_STRM("DistQuery returned bad latest platform version "
                        << latest);    
        return;
    }
        
    bp::ServiceVersion current;
    (void) current.parse(bp::paths::versionString());
    if (latestVersion.compare(current) == 1) {
        Path cacheDir = getPlatformCacheDirectory() / latestVersion.asString();
        if (!isDirectory(cacheDir)) {
            // newer version available and we don't have it, initiate download
            m_tid = m_distQuery->downloadLatestPlatform(m_platform);
            if (m_tid == 0) {
                BPLOG_ERROR("Unable to invoke DistQuery::downloadLatestPlatform");
                return;
            }
        }
    } else {
        BPLOG_INFO_STRM("No update available, we're on latest: "
                        << latestVersion.asString());
    }
        
    // store time of last check
    BPTime now;
    bp::phash::set(s_lastCheckTSKey, now.asString());
    m_tid = 0;
}
    
void
PlatformUpdaterSingleton::onLatestPlatformDownloaded(
    unsigned int,
    const LatestPlatformPkgAndVersion & pkgAndVersion)
{
    // event contains an object containing version number and 
    // a std::vector of bpkg goodness
    installToCache(pkgAndVersion);
    m_tid = 0;
}

void
PlatformUpdaterSingleton::onTransactionFailed(unsigned int)
{
    // TODO: aside from logging, what might be an appropriate
    // response here?
    BPLOG_ERROR("platform update check failed");
    m_tid = 0;
}

