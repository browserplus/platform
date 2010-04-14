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
 * main, core BrowserPlus process logic (where the main() lives)
 *
 * First introduced by Lloyd Hilaiel on 2007/05/07
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#include "BPDaemon.h"
#include <signal.h>
#include <sstream>

#include "AutoShutdown.h"
#include "BPUtils/bpexitcodes.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/ProcessLock.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bpplatformutil.h"
#include "CoreletManager/CoreletManager.h"
#include "Permissions/Permissions.h"
#include "PlatformUpdater.h"
#include "PermissionsUpdater.h"
#include "RequireLock.h"
#include "SessionManager.h"

// the "InactiveServices" corelet builtin.
#include "InactiveServicesCorelet.h"
#include "InactiveServicesCoreletFactory.h"

// singleton responsible for installation of corelets
#include "CoreletInstaller.h"

// singleton responsible for the periodic update of corelets
#include "CoreletUpdater.h"

using namespace std;
using namespace std::tr1;


// when running in the foreground we'll output informational/error
// messages to console before logging is enabled
static bool s_foreground = false;

// define statics in BPDaemon 
BPDaemon * BPDaemon::s_singletonDaemon = NULL;

// this must be global, and must be closed in signal handlers to solve
// the race condition on win32 where you're not gauranteed that your 
// program will finish after the user types ctrl-C
static bp::ProcessLock g_DaemonLockHandle;

static unsigned long s_updatePollPeriod = 86400;

// all of the code that should run once at shutdown, no matter what.
static void
shutdownProcess()
{
    CoreletInstaller::shutdown();
    CoreletUpdater::shutdown();
    PlatformUpdater::shutdown();
    PermissionsUpdater::shutdown();
    RequireLock::shutdown();
    bp::releaseProcessLock(g_DaemonLockHandle);
    g_DaemonLockHandle = NULL;
}


// windows specific signal handling
#ifdef WIN32
static BOOL
winSigHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_SHUTDOWN_EVENT)
    {
        shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
        daemon->stop();
        shutdownProcess();
        return TRUE;
    }
    return FALSE;
}
#else
// signal handler to shut us down cleanly on SIGTERM
static void
unixSigHandler(int sig)
{
    shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
    daemon->stop();
    shutdownProcess();
}
#endif


static void
setupErrorHandling()
{
    // win32 specific call to prevent display to user of useless
    // dialog boxes
#ifdef WIN32
    SetErrorMode(SetErrorMode(0) | SEM_NOOPENFILEERRORBOX |
                 SEM_FAILCRITICALERRORS);

    SetConsoleCtrlHandler((PHANDLER_ROUTINE) winSigHandler, TRUE);
#else 
    // shutdown cleanly on SIGTERM and SIGINT (^C) 
    signal(SIGINT, unixSigHandler);
    signal(SIGTERM, unixSigHandler);
#endif
}


// Parses the command-line using an APTArgParse object.
// Returns true on success.
static bool
processCommandLine(APTArgParse& argParser, int argc, const char ** argv)
{
    // Note.  All args are marked MAY_RECUR since the argv that we
    // are given is a combination of command line args and 
    // config file args
    static APTArgDefinition args[] =
    {
        { "l", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_RECUR,
        "enable logging, argument like \"info,ThrdLvlFuncMsg\""
        },
        { "fg", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_RECUR,
        "run in foreground.  Logging will appear on console."
        },
        { "cd", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_RECUR,
        "[c]orelet [d]irectory, directories in which to search for corelets. "
        "May be specified multiple times."
        }
    };
    
    // parse command line arguments
    bool rval = true;
    int x = argParser.parse(sizeof(args)/sizeof(args[0]), args, argc, argv);
    if (x < 0 || x != argc)
    {
        std::cerr << argParser.error() << std::endl;
        rval = false;
    }
    
    return rval;
}


static void 
setupLogging(const APTArgParse& argParser,
             std::string& config,
             bp::file::Path& file,
             const bp::config::ConfigReader& reader)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();
    
    if (argParser.argumentPresent("l")) { 
        // Setup the system-wide minimum log level.
        config = argParser.argument("l");
        bp::log::setLogLevel(bp::log::levelFromConfig(config));

        // Get log time format (if present) from the config file.
        // (no cmd-line support currently).  
        string timeFormat;
        (void) reader.getStringValue("LogTimeFormat", timeFormat);
        
        // if "-fg", log to console, else to logfile
        if (argParser.argumentPresent("fg")) {
            bp::log::setupLogToConsole(config, "", timeFormat);
        } else {
            file = bp::paths::getObfuscatedWritableDirectory() /
                   "BrowserPlusCore.log";

            long long int rolloverKB = 0;
            (void) reader.getIntegerValue("LogFileRolloverKB", rolloverKB);
            
            // Note: Daemon and Service logging are interleaved in same file.
            // We'll handle any rollover, services will just append.
            bp::log::setupLogToFile(file, config, bp::log::kSizeRollover, 
				                    timeFormat, (unsigned int) rolloverKB);
        }
    }
}


BPDaemon::BPDaemon(int argc, const char** argv)
    : m_argParser("Browser Plus Core")
{
    m_rl.init();

    s_singletonDaemon = this;
    
    // Setup a config file reader.
    bp::file::Path configFilePath = bp::paths::getConfigFilePath();
    if (!m_configReader.load(configFilePath)) {
        std::cerr << "couldn't read config file at: "
                  << configFilePath << std::endl;
        ::exit(bp::exit::kCantLoadConfigFile);
    }

    // If options present in config, grab them
    std::string configOptions;
    std::vector<std::string> options;
    if (m_configReader.getStringValue("Options", configOptions)) {
        options = bp::strutil::split(configOptions, " ");
    }
    
    // Add args from command line.  Since they are added
    // last, they override config options
    options.insert(options.begin(), argv[0]);  // make sure cmd name is first
    for (int i = 1; i < argc; i++) {
        options.push_back(argv[i]);
    }
    
    // form combined argc/argv from options, then give to arg parser
    int numArgs = options.size();
    const char** args = new const char*[numArgs];
    for (int i = 0; i < numArgs; i++) {
        args[i] = options[i].c_str();
    }
    
    bool argsOk = processCommandLine(m_argParser, numArgs, args);
    delete [] args;
    if (!argsOk)
    {
        // Exit on invalid cmd line.
        ::exit(bp::exit::kCantProcessCommandLine);
    }

    // set the foreground flag as soon as possible 
    if (m_argParser.argumentPresent("fg")) {
        std::cout << "Running in foreground mode..." << std::endl;
        s_foreground = true;
    }
}

BPDaemon::~BPDaemon()
{
    s_singletonDaemon = NULL;
    if (m_registry != NULL) 
    {
        // free resources for more effective leak checking.
        m_registry->unregisterAll();
    }

    m_rl.shutdown();
}

void
BPDaemon::run()
{
    using namespace bp::file;
    using namespace bp::paths;
    
    // If another B+ process is running, shutdown immediately,
    // only indication is exit code.
    if (NULL == (g_DaemonLockHandle = bp::acquireProcessLock(false)))
    {
        // another BrowserPlusCore process is running
        ::exit(bp::exit::kDuplicateProcess);
    }

#ifdef DEBUG
    // delay logging until here.  don't want
    // daemons which shutdown due to DaemonLock
    // to cream logs.  for release builds,
    // logging isn't setup until after we've 
    // checked the killswitch (in case there's 
    // a vulnerability in logging).
    setupLogging(m_argParser, m_logLevel, m_logFile, m_configReader);
#endif
    
    setupErrorHandling();

    PermissionsManager* pmgr = PermissionsManager::get();
    if (!pmgr) 
    {
        ::exit(bp::exit::kNoPermissionsManager);
    }
        
    // do we have adequate permissions to try to run?
    if (pmgr->upToDateCheck(this))
    {
        startup();
    } else {
        // will get IPermissionsManageListenerr::upToDate() called
    }

    m_rl.run();

    BPLOG_INFO("performing delayed deletes...");
    bp::file::delayDelete();

    // remove any uninstalled, unused platforms
    // Don't freak out, removePlatform() only removes
    // uninstalled, non-running platforms
    BPLOG_INFO("removing uninstalled platforms...");
    bp::file::Path dir = getProductTopDirectory();
    if (bp::file::isDirectory(dir)) {
        try {
            bp::file::tDirIter end;
            for (bp::file::tDirIter iter(dir); iter != end; ++iter) {
                string s = bp::file::utf8FromNative(iter->path().filename());
                bp::ServiceVersion version;
                if (version.parse(s)) {
                    BPLOG_DEBUG_STRM("examine " << version.asString());
                    bp::platformutil::removePlatform(version, false);
                }
            }
        } catch (const bp::file::tFileSystemError& e) {
            BPLOG_WARN_STRM("unable to iterate thru " << dir
                            << ": " << e.what());
        }
    }

    // shut it down.
    BPLOG_INFO("Shutting down...");
    shutdownProcess();
}

void
BPDaemon::stop()
{
    m_rl.stop();
}

// "setting up" a corelet registry is a matter of allocating it and
// registering all "built in" corelets
void
BPDaemon::setupCoreletRegistry()
{
    // allocate, passing through logging options
    m_registry.reset(new CoreletRegistry(m_logLevel, m_logFile));

    // register the "InactiveServices" corelet
    m_registry->registerCorelet(
        *InactiveServicesCorelet::getDescription(),
        shared_ptr<InactiveServicesCoreletFactory>(
            new InactiveServicesCoreletFactory()));

    // now set the corelet directorys
    if (m_argParser.argumentPresent("cd"))
    {
        std::vector<std::string> coreletDirs = m_argParser.argumentValues("cd");
        std::vector<std::string>::iterator it;
        for (it = coreletDirs.begin(); it != coreletDirs.end(); it++)
        {
            m_registry->setPluginDirectory(bp::file::Path(*it));
        }
    }

    // Add the default corelet directory
    m_registry->setPluginDirectory(bp::paths::getCoreletDirectory());
}


bool
BPDaemon::setupServer()
{
    BPLOG_INFO("Starting up, Logging setup.")
    
    // determine what ipc name we're connecting to
    std::string ipcPath = bp::paths::getIPCName();
    
    // allocate a session manager
    m_sessionManager.reset(new SessionManager(m_registry));

    // register our session manager as the listener of the the IPC server
    m_server.setListener(m_sessionManager.get());

    std::string errBuf;

    if (!m_server.start(ipcPath, &errBuf))
    {
        BPLOG_ERROR_STRM("Failed to bind " << ipcPath
                         << ": " << errBuf);
        return false;
    }
    
    BPLOG_INFO_STRM("Bound to '" << ipcPath << "'");

    return true;
}

bool
BPDaemon::getDistroServerList()
{
    if (!m_distroServers.empty()) {
        return true;
    }
    
    std::string primaryServer;
    if (!m_configReader.getStringValue("DistServer", primaryServer)) {
        return false;
    }

    // this may fail, we don't care, Secondaries are optional
    std::string secondaryString("SecondaryDistServers");
    (void) m_configReader.getArrayOfStrings(secondaryString,
                                            m_distroServers);

    m_distroServers.push_front(primaryServer);

    return true;
}

bool
BPDaemon::setupCoreletInstaller()
{
    if (!getDistroServerList()) return false;
    
    // start up the corelet installer
    CoreletInstaller::startup(m_distroServers, m_registry);

    return true;
}

bool
BPDaemon::setupCoreletUpdater()
{
    if (!getDistroServerList()) return false;

    long long int cVal;
    if (m_configReader.getIntegerValue("CoreletUpdatePollPeriod", cVal))
    {
        s_updatePollPeriod = (unsigned int) cVal;
    }

    // start up the corelet installer
    CoreletUpdater::startup(m_distroServers,
                            m_registry,
                            s_updatePollPeriod);

    return true;
}

bool
BPDaemon::setupPlatformUpdater()
{
    if (!getDistroServerList()) return false;
    
    unsigned int poll = 60 * 60 * 24;  // 1 day
    long long int cVal;
    if (m_configReader.getIntegerValue("PlatformUpdatePollPeriod", cVal)) {
        poll = (unsigned int) cVal;
    }
    PlatformUpdater::startup(m_distroServers, poll);
    return true;
}

bool
BPDaemon::setupPermissionsUpdater()
{
    unsigned int poll = 60 * 60 * 24;  // 1 day
    PermissionsUpdater::startup(poll);
    return true;
}

void
BPDaemon::setupAutoShutdown()
{
    const int knDefaultMaxIdleSecs = 5;

    int nMaxIdleSecs = knDefaultMaxIdleSecs;
    long long int lnConfigVal;
    if (m_configReader.getIntegerValue("MaxIdleSecs", lnConfigVal))
    {
        nMaxIdleSecs = static_cast<int>(lnConfigVal);
    }

    // If MaxIdleSecs is <=0, we interpret this as "no auto shutdown".
    if (nMaxIdleSecs > 0)
    {
        m_shutdownAgent.reset(new AutoShutdownAgent(nMaxIdleSecs,
                                                    m_sessionManager));
        m_shutdownAgent->start();
    }
}

bool
BPDaemon::checkKillSwitch()
{
    // must be able to get PermissionsManager.
    // without it we can't verify squat.  
    // immediately check for kill switch
    PermissionsManager* pMgr = PermissionsManager::get();
    return pMgr->mayRun();
}

void
BPDaemon::gotUpToDate()
{
    startup();
}

void
BPDaemon::cantGetUpToDate()
{
    if (s_foreground) {
        std::cerr << "Cannot fetch up-to-date permissions, exiting..."
                  << std::endl;
    }
    exit(bp::exit::kCantGetUpToDatePerms);
}

void
BPDaemon::startup()
{
    // NOTE: error return codes must all be < 0 (allows 
    // bp::process::wait() to distinguish between error exits
    // and signals). 
    if (!checkKillSwitch())
    {
        // this exit code known to BPProtocol SessionCreator.cpp
        ::exit(bp::exit::kKillswitch);
    }        

    RequireLock::initialize();

#ifndef DEBUG
    // delay logging until here.  don't want
    // daemons which shutdown due to DaemonLock
    // to cream logs.  for release builds,
    // logging isn't setup until after we've 
    // checked the killswith (in case there's 
    // a vulnerability in logging)
    setupLogging(m_argParser, m_logLevel, m_logFile, m_configReader);
#endif

    setupCoreletRegistry();
    
    if (!setupServer())
    {
        ::exit(bp::exit::kCantSetupIpcServer);
    }
    
    if (!setupCoreletInstaller())
    {
        ::exit(bp::exit::kCantSetupCoreletInstaller);
    }
    
    if (!setupCoreletUpdater()) {
        ::exit(bp::exit::kCantSetupCoreletUpdater);
    }
    
    if (!setupPlatformUpdater()) {
        ::exit(bp::exit::kCantSetupPlatformUpdater);
    }

    if (!setupPermissionsUpdater()) {
        ::exit(bp::exit::kCantSetupPermissionsUpdater);
    }
    
    // don't do autoshutdown if running in foreground
    if (!m_argParser.argumentPresent("fg"))
    {
        setupAutoShutdown();
    }
}

// access to top level singletons for others
shared_ptr<BPDaemon>
BPDaemon::getSharedDaemon()
{
    shared_ptr<BPDaemon> d;
    if (s_singletonDaemon) d = s_singletonDaemon->shared_from_this();
    return d;
;
}

shared_ptr<SessionManager>
BPDaemon::sessionManager()
{
    return m_sessionManager;
}

shared_ptr<CoreletRegistry>
BPDaemon::registry()
{
    return m_registry;
}
