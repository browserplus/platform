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
 * ServiceUpdater.h - A singleton responsible for periodic update of 
 *                    recently used services
 *
 * Created by Lloyd Hilaiel on Fri October 18th 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#include "ServiceUpdater.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/OS.h"
#include "ServiceInstaller.h"
#include "DistributionClient/DistributionClient.h"
#include "Permissions/Permissions.h"
#include "platform_utils/bpphash.h"
#include "RequireRequest.h"

using namespace std;
using namespace std::tr1;


static const char* s_lastCheckTSKey = "ServiceUpdater::lastCheckTS"; 

// no more frequently than 15 minutes
#define MIN_POLL_PERIOD 900

class UpdaterSingleton : virtual public bp::time::ITimerListener,
                         virtual public IDistQueryListener
{
public:
    UpdaterSingleton(unsigned int pollPeriod, 
                     shared_ptr<ServiceRegistry> serviceRegistry,
                     list<string> distroServers);
    ~UpdaterSingleton();

    void start();
    void stop();    
    bool busy();
private:
    // implemented methods from IDistQueryListener interface
    void onTransactionFailed(unsigned int tid,
                             const std::string& msg);
    void onCacheUpdated(unsigned int tid, const ServiceList & updates);

    void timesUp(bp::time::Timer * t);

    void upToDateCheck();

    std::string m_url;
    shared_ptr<ServiceRegistry> m_registry;
    unsigned int m_pollPeriod;    
    bp::time::Timer m_timer;
    DistQuery * m_distQuery;
    std::string m_platform;
    unsigned int m_tid;
};

static UpdaterSingleton* s_updater = NULL;

void
ServiceUpdater::startup(list<string> distroServers,
                        shared_ptr<ServiceRegistry> serviceRegistry,
                        unsigned int pollPeriodSeconds)
{
    BPASSERT(s_updater == NULL);
    s_updater = new UpdaterSingleton(pollPeriodSeconds, 
                                     serviceRegistry,
                                     distroServers);
    s_updater->start();
}


void
ServiceUpdater::shutdown()
{
    if (s_updater != NULL) {
        s_updater->stop();
        delete s_updater;
        s_updater = NULL;
    }
}

bool
ServiceUpdater::isBusy()
{
    BPASSERT(s_updater != NULL);
    return s_updater->busy();
}


UpdaterSingleton::UpdaterSingleton(unsigned int pollPeriod,
                                   shared_ptr<ServiceRegistry> serviceRegistry,
                                   list<string> distroServers)
{
    m_tid = 0;
    m_platform = bp::os::PlatformAsString();
    m_pollPeriod = ((pollPeriod > MIN_POLL_PERIOD)
                    ? pollPeriod : MIN_POLL_PERIOD);
    m_registry = serviceRegistry;
    m_distQuery = new DistQuery(distroServers, PermissionsManager::get());
    if (m_distQuery != NULL) {
        m_distQuery->setListener(this);
    } else {
        BPLOG_ERROR("unable to get distQuery");
    }
}


UpdaterSingleton::~UpdaterSingleton()
{
    delete m_distQuery;
}


void
UpdaterSingleton::start()
{
    if (m_distQuery == NULL) {
        return;
    }
    
    unsigned int secsToNextCheck = m_pollPeriod + 2;
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
    } catch (const std::runtime_error&) {
        // force an update
        last.set(0);
    }
    
    long diff = now.diffInSeconds(last);
    BPASSERT(diff >= 0);
    if (diff > (long) m_pollPeriod) {
        mustCheck = true;
    } else {
        secsToNextCheck = (m_pollPeriod - diff) + 2;
    }

    if (mustCheck) {
        BPLOG_INFO_STRM("last update check at "
                        << last.asString()
                        << ", checking for updates now");
    } else {
        BPLOG_INFO_STRM("last update check at "
                        << last.asString()
                        << " checking again for updates in "
                        << secsToNextCheck << "s");
    }

    if (mustCheck) {
        // must check, get installed services
        list<bp::service::Summary> installed =
            m_registry->availableServiceSummaries();
        
        // now get our previous require statements
        list<ServiceRequireStatement> reqStmts;
        using namespace bp;
        string requireMapStr;
        (void)bp::phash::get(RequireRequest::kRequireStatementsKey, requireMapStr);
        Map* m = dynamic_cast<Map*>(Object::fromPlainJsonString(requireMapStr));
        if (m == NULL) m = new Map;
        Map::Iterator miter(*m);
        const char* key = NULL;
        while ((key = miter.nextKey()) != NULL) {
            const List* l = dynamic_cast<const List*>(m->get(key));
            if (!l) continue;
            for (unsigned int i = 0; i < l->size(); ++i) {
                const String* s = dynamic_cast<const String*>(l->value(i));
                if (!s) continue;
                vector<string> tokens = bp::strutil::split(
                    s->value(), RequireRequest::kVersionSeparator);
                if (tokens.size() != 2) continue;
                ServiceRequireStatement req;
                req.m_name = key;
                req.m_version = tokens[0];
                req.m_minversion = tokens[1];
                reqStmts.push_back(req);
            }
        }
        
        // pass off to DistQuery to update
        m_tid = m_distQuery->updateCache(m_platform, reqStmts, installed);
        if (m_tid == 0) {
            BPLOG_ERROR("Unable to invoke DistQuery::updateCache");
        }
    }

    // set the timer 
    m_timer.cancel();
    m_timer.setListener(this);
    m_timer.setMsec(1000 * secsToNextCheck);    
}    


void
UpdaterSingleton::stop()
{
    m_timer.cancel();
}


bool
UpdaterSingleton::busy()
{
    return m_tid != 0;
}


void
UpdaterSingleton::timesUp(bp::time::Timer *)
{
    start();
}

void
UpdaterSingleton::onTransactionFailed(unsigned int,
                                      const std::string& msg)
{
    BPLOG_WARN_STRM("update failed: " << msg << ", purging require history.");
    bp::phash::set(RequireRequest::kRequireStatementsKey, std::string());
    m_tid = 0;
}


void
UpdaterSingleton::onCacheUpdated(unsigned int,
                                 const ServiceList &)
{
    // store time of last check
    BPTime now;
    bp::phash::set(s_lastCheckTSKey, now.asString());
    m_tid = 0;
}

