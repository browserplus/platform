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
 * A tool which will validate services and publish them locally.
 */

#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "platform_utils/APTArgParse.h"
#include "platform_utils/ARGVConverter.h"
#include "platform_utils/ProductPaths.h"
#include "platform_utils/ServicesUpdatedFile.h"
#include "ServiceRunnerLib/ServiceRunnerLib.h"
#include "platform_utils/bpdebug.h"

using namespace std;
using namespace std::tr1;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;


static bp::runloop::RunLoop s_rl;
static bfs::path s_harnessProgram;
static bp::time::Stopwatch s_sw;
static bool s_log = false;


static APTArgDefinition g_args[] = {
    { "log", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "enable console logging, argument is level (info, debug, etc.)"
    },
    { "logfile", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "Enable file logging, argument is a path, when combined with '-log' "
      "logging will occur to a file at the level specified."
    },
    {"u", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR, 
      "uninstall rather than install"
    },
    { "v", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "be verbose (enabling logging increases verbosity even more)"
    },
    { "t", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "output detailed timing information"
    },
    { "n", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "Dry run.  Validate the service but do not actually install it."
    },
    { "f", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "[f]orce overwriting of existing service."
    }
};


#define BPOUT(x) \
{ \
    if (s_log) { \
        BPLOG_INFO_STRM(x); \
    } else { \
        cerr << x << endl; \
    } \
}


// A class which will serve as a listener to the service controller.
class ServiceManager :  public ServiceRunner::IControllerListener
{
public:
    typedef enum {
        eGetDescription,
        eInstallHook,
        eUninstallHook
    } Action;
    ServiceManager(shared_ptr<APTArgParse> argParser,
                   Action a)
        : m_argParser(argParser), m_apiVersion(0), m_action(a), m_code(0)
    {
    }

    bp::service::Description description()
    {
        return m_desc;
    }
    unsigned int apiVersion() 
    {
        return m_apiVersion;
    }
    int code() const 
    {
        return m_code;
    }
    string actionStr() const 
    {
        switch (m_action) {
        case eGetDescription:
            return "getDescription";
        case eInstallHook:
            return "installHook";
        case eUninstallHook:
            return "uninstallHook";
        }
        return "unknown";
    }

private:   
    bp::service::Description m_desc;

    void stop(int code)
    {
        m_code = code;
        s_rl.stop();
    }

    void initialized(ServiceRunner::Controller* c,
                     const string& service,
                     const string& version,
                     unsigned int apiVersion)
    {
        if (m_argParser->argumentPresent("v")) {
            BPOUT("service initialized: " << service << " v" << version);
        }
        m_apiVersion = apiVersion;

        bfs::path serviceDir = bp::paths::getServiceDirectory() / service / version;
        bp::SemanticVersion v;
        if (!v.parse(version)) {
            BPOUT("Bad version " << version);
            stop(-1);
        }
        bfs::path dataDir = bp::paths::getServiceDataDirectory(service,
                                                               v.majorVer());
        switch (m_action) {
        case eGetDescription:
            c->describe();
            break;
        case eInstallHook:
            c->installHook(serviceDir, dataDir);
            break;
        case eUninstallHook:
            c->uninstallHook(serviceDir, dataDir);
            break;
        default:
            BPOUT("Bad action code: " << m_action);
            stop(-2);
        }
    }

    void onEnded(ServiceRunner::Controller * c) 
    {
        BPOUT("Spawned service exited!  (enable logging for more "
              << "detailed diagnostics - '-log debug').");        
        stop(-3);
    }

    void onDescribe(ServiceRunner::Controller*,
                    const bp::service::Description& desc)
    {
        // we've extracted a description of the service, let's save it.
        m_desc = desc;
        stop(0);
    }

    // unused overrides (because we pass off the controller to
    // the controller manager once it's initialized)
    void onAllocated(ServiceRunner::Controller*,
                     unsigned int,
                     unsigned int) 
    {
    }
    void onInvokeResults(ServiceRunner::Controller*,
                         unsigned int,
                         unsigned int,
                         const bp::Object*) 
    {
    }
    void onInvokeError(ServiceRunner::Controller*,
                       unsigned int,
                       unsigned int,
                       const string&,
                       const string&) 
    {
    }
    void onCallback(ServiceRunner::Controller*,
                    unsigned int,
                    unsigned int,
                    long long int,
                    const bp::Object*) 
    {
    }
    void onPrompt(ServiceRunner::Controller*,
                  unsigned int,
                  unsigned int,
                  const bfs::path&,
                  const bp::Object*) 
    {
    }

    void onInstallHook(ServiceRunner::Controller* c,
                       int code)
    {
        if (m_argParser->argumentPresent("v")) {
            BPOUT("installHook returned " << code);        
        }
        stop(code);
    }

    void onUninstallHook(ServiceRunner::Controller* c,
                         int code)
    {
        if (m_argParser->argumentPresent("v")) {
            BPOUT("uninstallHook returned " << code);        
        }
        stop(code);
    }

    shared_ptr<APTArgParse> m_argParser;
    unsigned int m_apiVersion;
    Action m_action;
    int m_code;
};


// Helper func to make timestamp strings.
string
timeStamp()
{
    stringstream ss;
    ss << "[" << s_sw.elapsedSec() << "s] ";
    return ss.str();
}


static void 
setupLogging(shared_ptr<APTArgParse> argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();

    string level = argParser->argument("log");
    bfs::path path(argParser->argument("logfile"));

    if (level.empty() && path.empty()) return;

    s_log = true;

    if (level.empty()) level = "info";

    bp::log::Level logLevel = bp::log::levelFromString(level);

    if (path.empty()) bp::log::setupLogToConsole(logLevel);
    else bp::log::setupLogToFile(path, logLevel, bp::log::kAppend);
}


static int
runService(shared_ptr<APTArgParse> argParser,
           shared_ptr<ServiceManager> serviceMan,
           const bfs::path& absPath,
           const bp::service::Summary& summary)
{
    string error, processTitle, ignore;

    /* TODO: extract and pass proper locale! */
    if (!summary.localization("en", processTitle, ignore)) {
        processTitle.append("BrowserPlus: Spawned Service");
    } else {
        processTitle = (string("BrowserPlus: ") + processTitle);
    }
    // now let's find a valid provider if this is a dependent
    bfs::path providerPath;
    if (summary.type() == bp::service::Summary::Dependent) {
        providerPath = ServiceRunner::determineProviderPath(summary, error);
        if (!error.empty()) {
            BPOUT("Couldn't run service because I couldn't "
                  << "find an appropriate installed "
                  << "provider service.");
            return(1);
        }        
    }

    // run action.  runloop stops after action
    if (argParser->argumentPresent("t")) {
        BPOUT(timeStamp() << "start " << serviceMan->actionStr());
    }
    shared_ptr<ServiceRunner::Controller> controller(new ServiceRunner::Controller(absPath));
    controller->setListener(serviceMan.get());
    if (!controller->run(s_harnessProgram, providerPath,
                         processTitle, argParser->argument("log"),
                         bfs::path(argParser->argument("logfile")),
                         error)) {
        cerr << "Couldn't run service: " << error;
        return(1);
    }
    s_rl.run();
    if (argParser->argumentPresent("t")) {
        BPOUT(timeStamp() << "finish " << serviceMan->actionStr());
    }
    return serviceMan->code();
}


static int 
doUninstall(shared_ptr<APTArgParse> argParser,
            const bfs::path& absPath,
            const bp::service::Summary& summary,
            int apiVersion)
{
    bool dryRun = argParser->argumentPresent("n");

    if (!bp::file::isDirectory(absPath)) {
        return 0;
    }

    // Possible uninstall hook?
    if (apiVersion >= 5) {
        if (!dryRun) {
            shared_ptr<ServiceManager> serviceMan
                (new ServiceManager(argParser, ServiceManager::eUninstallHook));
            int rv = runService(argParser, serviceMan, absPath, summary);
            if (rv != 0) {
                BPOUT("Uninstall hook failed, code = " << rv
                      << ", removing anyway");
            }
            serviceMan.reset();
        } else {
            BPOUT("would run any uninstall hook");
        }
    }

    // actually nuke the service
    if (argParser->argumentPresent("t")) {
        BPOUT(timeStamp() << "start remove " << absPath);
    }
    if (!dryRun) {
        bool rv = bpf::safeRemove(absPath);
        if (argParser->argumentPresent("t")) {
            BPOUT(timeStamp() << "remove " << absPath << " returns " << rv);
        }
    } else {
        BPOUT("would remove " << absPath);
    }

    if (!dryRun) {
        // now we'll indicate that services have changed for a running
        // BrowserPlusCore daemon
        ServicesUpdated::indicateServicesChanged();
    }

    // we always succeed, even when we fail...
    return 0;
}


static int 
doInstall(shared_ptr<APTArgParse> argParser,
          const bfs::path& source,
          const string& serviceName,
          const string& serviceVersion,
          const bp::service::Summary& summary,
          int apiVersion)
{
    BPLOG_DEBUG_STRM("install " << serviceName << " / " << serviceVersion
                     << " from " << source);
    bool overwrote = false;
    bool dryRun = argParser->argumentPresent("n");

    // Now do the install.  We copy to the
    // local service directory and call install hook
    bfs::path destination = bp::paths::getServiceDirectory()
                                 / serviceName / serviceVersion;

    // must we uninstall existing?
    if (bpf::isDirectory(destination)) {
        if (argParser->argumentPresent("f")) {
            if (!dryRun) {
                int rv = doUninstall(argParser, destination, summary, apiVersion);
                BPOUT("uninstall of existing service returns " << rv);
            } else {
                BPOUT("would uninstall existing " << destination);
            }
        } else {
            BPOUT("cannot overwrite " << destination << " without -f argument");
            return -1;
        }
    }

    if (!dryRun) {
        if (argParser->argumentPresent("t")) {
            BPOUT(timeStamp() << "preparing installation directory...");
        }
        try {
            bfs::create_directories(destination);
        } catch(const bfs::filesystem_error&) {
            BPOUT("cannot create directory: '" << destination << "'");
            return -1;
        }
    }

    if (argParser->argumentPresent("v")) {
        BPOUT("installing service locally: " << serviceName << ", ver "
              << serviceVersion);
    }
    if (bp::file::isDirectory(source)) {
        try {
            bfs::directory_iterator end;
            for (bfs::directory_iterator it(source); it != end; ++it) {
                bfs::path fromPath = it->path();
                bfs::path relPath = bpf::relativeTo(fromPath, source);
                bfs::path toPath = destination / relPath;
                if (!dryRun) {
                    if (!bpf::safeCopy(fromPath, toPath)) {
                        BPOUT("error copying " << fromPath);
                        return(1);
                    } 
                } else {
                    BPOUT("would copy " << fromPath << " -> " << toPath);
                }
            }
        } catch (const bfs::filesystem_error& e) {
            BPOUT("unable to iterate thru " << source << ": " << e.what());
            return -1;
        }
    }
    if (argParser->argumentPresent("t")) {
        BPOUT(timeStamp() << "copying complete");
    }

    // Possibility of an install hook?
    if (apiVersion >= 5) {
        if (!dryRun) {
            shared_ptr<ServiceManager> serviceMan(
                new ServiceManager(argParser, ServiceManager::eInstallHook));
            int rv = runService(argParser, serviceMan, destination, summary);
            if (rv != 0) {
                BPOUT("Install hook failed, code = " << rv);
                (void) bpf::safeRemove(destination);
                return rv;
            }
        } else {
            BPOUT("would invoke any install hook");
        }
    }

    // let's update the moddate on the manifest.json to cause
    // BrowserPlus to update this service.  This is only necessary
    // if we overwrote the on-disk service
    if (overwrote) {
        bfs::path manifestPath = destination / "manifest.json";
        bp::file::touch(manifestPath);
    }
    
    // now we'll indicate that services have changed for a running
    // BrowserPlusCore daemon
    ServicesUpdated::indicateServicesChanged();
    return 0;
}


int
main(int argc,
     const char** argv)
{
    // handle non-ascii args on windows
    APT::ARGVConverter conv;
    conv.convert(argc, argv);

    // which mode are we running in?
    if (argc > 1 && !string("-runService").compare(argv[1])) {
        // optional -breakpoint arguments set forced breaks
        list<string> breakpoints;
        for (int i = 2; i < argc; i++) {
            if (!string(argv[i]).compare("-breakpoint")) {
                if ((i + 1) < argc) {
                    breakpoints.push_back(string(argv[i + 1]));
                    i++; // go past this value
                }
            }
        }
        bp::debug::setForcedBreakpoints(breakpoints);
        if (ServiceRunner::runServiceProcess(argc, argv)) return 0;
        return 1;
    }

    s_rl.init();

    // a stopwatch for timing
    s_sw.start();

    // win32 specific call to prevent display to user of useless
    // dialog boxes
#ifdef WIN32
    SetErrorMode(SetErrorMode(0) | SEM_NOOPENFILEERRORBOX |
                 SEM_FAILCRITICALERRORS);
#endif

    // make a usage string
    string usage("Install a BrowserPlus service.\n    ");
    usage.append(argv[0]);
    usage.append(" [opts] <service dir>");

    // parse command line arguments
    shared_ptr<APTArgParse> argParser(new APTArgParse(usage));
    
    int x = argParser->parse(sizeof(g_args)/sizeof(g_args[0]), g_args,
                             argc, argv);
    if (x < 0) {
        BPOUT(argParser->error());
        exit(-4);
    }

    // pathToHarness is ourself
    s_harnessProgram = bpf::canonicalProgramPath(bfs::path(argv[0]));

    bool dryRun = argParser->argumentPresent("n");
    bool uninstall = argParser->argumentPresent("u");

    if (uninstall) {
        if (x != (argc - 2)) {
            BPOUT("invalid arguments: ");
            if (x > (argc - 2)) { 
                BPOUT("path to service and version required");
            } else {
                BPOUT("extra command line arguments detected");
            }
            exit(-5);
        }
    } else if (x != (argc - 1)) {
        BPOUT("invalid arguments: ");
        if (x > (argc - 1)) { 
            BPOUT("path to service required");
        } else {
            BPOUT("extra command line arguments detected");
        }
        exit(-6);
    }

    setupLogging(argParser);

    bfs::path absPath;
    string serviceName, serviceVersion;
    if (uninstall) {
        serviceName = argv[x];
        serviceVersion = argv[x+1];
        absPath = bp::paths::getServiceDirectory() / serviceName / serviceVersion;
    } else {
        absPath = bp::file::canonicalPath(bfs::path(argv[x]));
    }

    // get service summary
    bp::service::Summary summary;
    string error;
    if (argParser->argumentPresent("t")) {
        BPOUT(timeStamp() << "detecting service at: " << absPath);
    }
    if (!summary.detectService(absPath, error)) {
        BPOUT("Invalid service: " << endl << "  " << error);
        if (uninstall) {
            BPOUT("removing anyway");
            (void) bpf::safeRemove(absPath);
        }
        exit(-7);
    }

    // get service description
    shared_ptr<ServiceManager> serviceMan(new ServiceManager(argParser,
                                                             ServiceManager::eGetDescription));
    int rv = runService(argParser, serviceMan, absPath, summary);
    if (rv != 0) {
        BPOUT("Unable to get service description, code = " << rv);
        if (uninstall) {
            BPOUT("Removing anyway");
        } else {
            exit(-9);
        }
    }
    bp::service::Description desc = serviceMan->description();
    int apiVersion = serviceMan->apiVersion();
    serviceMan.reset();

    // An uninstall?
    if (uninstall) {
        rv = doUninstall(argParser, absPath, summary, apiVersion);
    } else {
        // attain convenient representation of name and version of service
        // we're publishing
        serviceName = desc.name();
        serviceVersion = desc.versionString();
        rv = doInstall(argParser, summary.path(), desc.name(),
                       desc.versionString(), summary, apiVersion);
    }
    
    // all done!
    s_rl.shutdown();

    if (argParser->argumentPresent("v")) {    
        stringstream ss;
        ss << serviceName << " " << serviceVersion << " validated ";
        if (!dryRun) {
            ss << "and " << (uninstall ? "uninstalled" : "installed");
        }
        ss << " in " << s_sw.elapsedSec() << "s";
        BPOUT(ss.str());
    }
    exit(rv);
}
