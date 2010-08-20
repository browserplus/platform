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
 * PermissionsUpdater.h - A singleton responsible for periodic permissions update
 *
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#include "PermissionsUpdater.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bptimer.h"
#include "BPUtils/bpexitcodes.h"
#include "Permissions/Permissions.h"

using namespace std;

class PermissionsUpdaterSingleton : virtual public bp::time::ITimerListener,
                                    virtual public IPermissionsManagerListener
{
public:
    PermissionsUpdaterSingleton(unsigned int pollPeriod);
    ~PermissionsUpdaterSingleton();

    void start();
    void stop();    
    bool busy();
private:
    // implemented methods from IPermissionsManagerListener interface
    void gotUpToDate();
    void cantGetUpToDate();

    // implemented methods from ITimerListener interface
    void timesUp(bp::time::Timer* t);

    unsigned int m_pollPeriod;
    bp::time::Timer m_timer;
    bool m_busy;
};

static PermissionsUpdaterSingleton* s_updater = NULL;

void
PermissionsUpdater::startup(unsigned int pollPeriodSeconds)
{
    BPASSERT(s_updater == NULL);
    s_updater = new PermissionsUpdaterSingleton(pollPeriodSeconds);
    s_updater->start();
}


void
PermissionsUpdater::shutdown()
{
    if (s_updater != NULL) {
        s_updater->stop();
        delete s_updater;
        s_updater = NULL;
    }
}

bool
PermissionsUpdater::isBusy()
{
    BPASSERT(s_updater != NULL);
    return s_updater->busy();
}


PermissionsUpdaterSingleton::PermissionsUpdaterSingleton(unsigned int pollPeriod)
    : m_pollPeriod(pollPeriod), m_busy(false)
{
}


PermissionsUpdaterSingleton::~PermissionsUpdaterSingleton()
{
}


void
PermissionsUpdaterSingleton::start()
{
    // Is time since last check greater than our 
    // poll period?  If so, initiate a permissions
    // check.  Otherwise, set timer to test again,
    // making sure that no more than m_pollPeriod
    // seconds passes between any checks.
    PermissionsManager* pmgr = PermissionsManager::get();
    bool mustCheck = false;
    unsigned int timeToNext = m_pollPeriod;
    BPTime lastCheck;
    if (pmgr->lastCheck(lastCheck)) {
        BPTime now;
        unsigned int secsSinceLast = now.diffInSeconds(lastCheck);
        if (secsSinceLast > m_pollPeriod) {
            mustCheck = true;
        } else {
            timeToNext = m_pollPeriod - secsSinceLast;
        }
    } else { 
        mustCheck = true;
    }

    if (mustCheck) {
        if (!pmgr->upToDateCheck(this)) {
            BPLOG_DEBUG("PermissionsUpdater checking permissions...");
            m_busy = true;
        }
    }

    // set the timer 
    m_timer.cancel();
    m_timer.setListener(this);
    m_timer.setMsec(1000 * timeToNext);    
}    


void
PermissionsUpdaterSingleton::stop()
{
    m_timer.cancel();
}


bool
PermissionsUpdaterSingleton::busy()
{
    return m_busy;
}


void
PermissionsUpdaterSingleton::timesUp(bp::time::Timer *)
{
    start();
}


void
PermissionsUpdaterSingleton::gotUpToDate()
{
    BPLOG_DEBUG("PermissionsUpdater got permissions");
    m_busy = false;
    if (!PermissionsManager::get()->mayRun()) {
        BPLOG_ERROR_STRM("PermissionsUpdater finds !mayRun(), exiting");
        // this exit code known to BPProtocol SessionCreator.cpp
        ::exit(bp::exit::kKillswitch);
    }
}


void
PermissionsUpdaterSingleton::cantGetUpToDate()
{
    BPLOG_ERROR_STRM("PermissionsUpdater unable to get permissions, exiting");
    m_busy = false;
    ::exit(bp::exit::kCantGetUpToDatePerms);
}

