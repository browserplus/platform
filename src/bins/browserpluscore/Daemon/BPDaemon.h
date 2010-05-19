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
 * BPDaemon is a top level object of which there is only a single instance
 * process wide.  it's implementation is inside main.cpp.  BPDaemon
 * controls the lifetime of all high level signleton instances thorughout
 * the process.
 */

#ifndef __BPDAEMON_H__
#define __BPDAEMON_H__

#include "AutoShutdown.h"
#include "BPUtils/APTArgParse.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpconfig.h"
#include "BPUtils/bprunloop.h"
#include "SessionManager.h"

class BPDaemon : virtual public IPermissionsManagerListener,
                 public std::tr1::enable_shared_from_this<BPDaemon>
{
public:
    BPDaemon(int argc, const char** argv);
    virtual ~BPDaemon();

    // runs the main event loop
    void run();

    // stops the daemon, may be called by anyone to stop the program
    void stop();

    static std::tr1::shared_ptr<class BPDaemon> getSharedDaemon();

    std::tr1::shared_ptr<SessionManager> sessionManager();
    
    std::tr1::shared_ptr<CoreletRegistry> registry();
    
private:
    void startup();
    void setupCoreletRegistry();
    bool setupServer();
    bool getDistroServerList();
    bool setupCoreletInstaller();
    bool setupCoreletUpdater();
    bool setupPlatformUpdater();
    bool setupPermissionsUpdater();
    void setupAutoShutdown();
    bool checkKillSwitch();
    void gotUpToDate();
    void cantGetUpToDate();
    
    std::string m_logLevel;
    bp::file::Path m_logFile;
    bp::config::ConfigReader m_configReader;
    APTArgParse m_argParser;
    std::list<std::string> m_distroServers;
    std::tr1::shared_ptr<CoreletRegistry> m_registry;
    bp::ipc::ChannelServer m_server;
    std::tr1::shared_ptr<SessionManager> m_sessionManager;
    std::tr1::shared_ptr<AutoShutdownAgent> m_shutdownAgent;

    bp::runloop::RunLoop m_rl;

    // set in ctor, null'd in dtor
    static class BPDaemon * s_singletonDaemon;
};

#endif
