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
 *  AutoShutdown.cpp
 *
 *  Created by David Grigsby on 9/12/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "AutoShutdown.h"
#include "ServiceInstaller.h"
#include "ServiceUpdater.h"
#include "PlatformUpdater.h"
#include "Permissions/Permissions.h"
#include "BPDaemon.h"

using namespace std;
using namespace std::tr1;


AutoShutdownAgent::AutoShutdownAgent(
    int nPollPeriodSecs,
    shared_ptr<SessionManager> sessionManager)
    : m_nPollPeriodSecs(nPollPeriodSecs),    
      m_sessionManager(sessionManager)
{
    m_timer.setListener(this);
    // Setup a recurring timer that will cause events to be posted to us.
    start();
}


AutoShutdownAgent::~AutoShutdownAgent()
{
	stop();
}


void AutoShutdownAgent::start()
{
    m_timer.cancel();
    m_timer.setMsec(m_nPollPeriodSecs * 1000);
}


void AutoShutdownAgent::stop()
{
    m_timer.cancel();
}


void
AutoShutdownAgent::timesUp(bp::time::Timer * /*t*/)
{
	stop();

    // If we have become blacklisted (can happen if permissions 
    // update tells us that we're evil), commit suicide
    if (!PermissionsManager::get()->mayRun()) {
        shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
        daemon->stop();
    } 
    
    // Any fatal permissions errors lead to suicide
    else if (PermissionsManager::get()->error()) {
        shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
        daemon->stop();
    }
    
    // If we're idle, stop the runloop.  This will lead to shutdown of
    // the daemon.
    else if ((m_sessionManager->numCurrentSessions() == 0) &&
             !PlatformUpdater::isBusy() &&
             !ServiceInstaller::isBusy() &&
             !ServiceUpdater::isBusy() &&
             !PermissionsManager::get()->isBusy() &&
             !BPDaemon::getSharedDaemon()->registry()->isBusy())
    {
        shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
        daemon->stop();
	}
    else
    {
		start();
	}
}
