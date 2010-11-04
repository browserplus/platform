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

#include <iostream>

#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "ConsoleLib/ConsoleLib.h"
#include "Output.h"
#include "platform_utils/ARGVConverter.h"
#include "platform_utils/APTArgParse.h"
#include "ServiceManager/ServiceManager.h"
#include "Permissions/Permissions.h"
#include "Permissions/PermissionsManager.h"
#include "BPUtils/OS.h"
#include "platform_utils/ProductPaths.h"
#include "platform_utils/bpdebug.h"

// here's our implementation of handling commands
#include "CommandExecutor.h"

using namespace std;
using namespace std::tr1;

bp::runloop::RunLoop s_rl;

class ServiceUpdateListener : public IDistQueryListener {
public:
    ServiceUpdateListener(DistQuery* dq, bp::file::Path& downloadPath, bp::file::Path& certFile)
        : m_distQuery(dq)
        , m_downloadSuccess(false)
        , m_downloadPath(downloadPath)
        , m_certFile(certFile)
        , m_serviceName("")
        , m_serviceVersion("") {
    }
    virtual ~ServiceUpdateListener() {
    }
    bool DidDownloadSucceed() const {
        // we only support one dependent service right now.
        // the provider path stuff needs to be re-thought a little to support more complex hierarchy.
        return m_neededServices.size() == 0 || m_downloadSuccess;
    }
    ServiceList CompletedServices() {
        return m_completedServices;
    }
private:
    virtual void onTransactionFailed(unsigned int tid, const std::string& msg) {
        m_downloadSuccess = false;
        s_rl.stop();
    }
    virtual void gotAvailableServices(unsigned int tid, const ServiceList & list) {
        m_downloadSuccess = false;
    }
    virtual void onServiceFound(unsigned int tid, const AvailableService & service) {
        std::stringstream ss;
        ss << "Found viable service: " << service.name << " v" << service.version.asString();
        output::puts(output::T_INFO, ss.str());
        m_serviceName = service.name;
        m_serviceVersion = service.version.asString();
        m_downloadSuccess = false;
    }
    virtual void onDownloadProgress(unsigned int tid, unsigned int pct) {
        if (output::getSlaveMode()) {
            output::puts(output::T_INFO, ".");
        }
        else {
            std::cout << ".";
        }
    }
    virtual void onDownloadComplete(unsigned int tid, const std::vector<unsigned char> & buf) {
        if (output::getSlaveMode()) {
            // NO-OP
        }
        else {
            std::cout << std::endl;
        }
        pair<string, string> p = m_neededServices.front();
        ServiceUnpacker unpacker(buf, m_downloadPath, p.first, p.second, m_certFile);
        string errMsg;
        std::stringstream ss;
        ss << "Installing service: " << p.first << " v" << p.second;
        output::puts(output::T_INFO, ss.str());
        m_downloadSuccess = unpacker.unpack(errMsg) && unpacker.install(errMsg);
        if (m_downloadSuccess) {
            m_neededServices.pop_front();
            m_completedServices.push_back(p);
            fetchNextService();
        }
        else {
            std::stringstream ss;
            ss << "Installing failed: " << errMsg;
            output::puts(output::T_ERROR, ss.str());
            s_rl.stop();
        }
    }
    virtual void onRequirementsSatisfied(unsigned int, const ServiceList & clist) {
        // Run thru clist, only adding services that we don't
        // have to m_neededServices.  We check for the existence
        // of a service's manifest.json since if a running service
        // is removed, the directory and dll will still exist,
        // but the manifest file won't (since windows can't remove
        // open files or their containing dirs).
        bp::file::Path serviceDir(m_downloadPath);
        ServiceList::const_iterator it;
        for (it = clist.begin(); it != clist.end(); ++it) {
            bp::file::Path path = serviceDir / it->first / it->second / "manifest.json";
            if (bp::file::exists(path)) {
                m_completedServices.push_back(*it);
            }
            else {
                m_neededServices.push_back(*it);
            }
        }
        fetchNextService();
    }
    unsigned int fetchNextService() {
        if (!m_neededServices.empty()) {
            pair<string, string> p = m_neededServices.front();
            std::stringstream ss;
            ss << "Downloading service: " << p.first << " v" << p.second;
            output::puts(output::T_INFO, ss.str());
            return m_distQuery->downloadService(p.first, p.second,
                                                bp::os::PlatformAsString());
        } else {
            s_rl.stop();
        }
        return 0;
    }
private:
    DistQuery* m_distQuery;
    bool m_downloadSuccess;
    bp::file::Path m_downloadPath;
    bp::file::Path m_certFile;
    std::string m_serviceName;
    std::string m_serviceVersion;
    ServiceList m_neededServices;
    ServiceList m_completedServices;
};

static APTArgDefinition g_args[] = {
    { "log", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "enable console logging, argument like \"info\" or \"debug\""
    },
    { "slave", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "\"slave\" mode causes all output from service runner to be JSON formatted "
      "this is useful for driving service runner from a host process."
    },
    { "logfile", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "Enable file logging, argument is a path, when combined with '-log' "
      "logging will occur to a file at the level specified."
    },
    { "version", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "get the BrowserPlus platform version with this tool was built."
    },
    { "providerPath", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "When running a dependent service, you may explicitly specify "
      "the path to the provider service.  This is useful when developing "
      "provider services.  If a dependent service is specified, and no "
      "-providerPath is supplied, then we look for an installed service "
      "that satisfies the dependent's requirements."
    },
    { "downloadPath", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "When running a dependent service, you may explicitly specify "
      "the path to the download a provider service.  This is useful "
      "when developing provider services.  If a dependent service is "
      "specified, and no -downloadPath is supplied, then we look for"
      "an installed service that satisfies the dependent's requirements."
    },
    { "distroServer", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "When downloading provider services, use this distro server."
      "This is useful when developing provider services.  If a dependent "
      "service is specified, and no -distroServer is supplied, then "
      "any attempt to download will be handled by production servers."
    },
    { "certFile", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "When validating downloaded provider services, use this certificate file."
      "This is useful when developing provider services.  If a dependent "
      "service is specified, and no -certFile is supplied, then "
      "the service may fail to unpack."
    },
    { "debugService", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "Used to launch a debugger and attach to the correct process, "
      "the breakpoint occurs before your service is loaded so that "
      "all of the service code may be debugged and verified."
    },
};
      
static void 
setupLogging(const APTArgParse& argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();

    std::string config = argParser.argument("log");
    bp::file::Path path(argParser.argument("logfile"));

    if (config.empty() && path.empty()) return;
    if (config.empty()) config = "info";
    
    // Setup the system-wide minimum log level.
        bp::log::Level logLevel = bp::log::levelFromString(config);
    bp::log::setLogLevel(logLevel);

    // For now always use "msec" time format.
    // We could add an APTArg but seems unnecessary at the moment.
        bp::log::TimeFormat timeFormat = bp::log::TIME_MSEC;
    
        if (path.empty()) {
        bp::log::setupLogToConsole(logLevel,"BrowserPlus Service Runner",
                                   timeFormat);
        } else {
        bp::log::setupLogToFile(path,logLevel,bp::log::kTruncate,timeFormat);
        }
}

/** a class to listen for UserQuitEvents and stop the runloop upon
 *  their arrival */
class RunManager :  public ICommandHandlerListener,
                    public ServiceRunner::IControllerListener
{
public:
    RunManager(CommandParserPtr parser,
               shared_ptr<CommandExecutor> executor)
        : m_parser(parser), m_executor(executor)
    {
    }
private:   
    void onUserQuit() {
        output::puts(output::T_INFO, "shutting down...");
        s_rl.stop();
    }
    void initialized(ServiceRunner::Controller * c,
                     const std::string & service,
                     const std::string & version,
                     unsigned int) 
    {
        std::stringstream ss;
        ss << "service initialized: " << service << " v" << version;        
        output::puts(output::T_INFO, ss.str());
        c->describe();
    }
    void onEnded(ServiceRunner::Controller * c) 
    {
        output::puts(output::T_ERROR, "Spawned service exited!  (enable logging for more "
                     "detailed diagnostics - '-log debug').");
        exit(1);
    }
    CommandParserPtr m_parser;
    shared_ptr<CommandExecutor> m_executor;

    void onDescribe(ServiceRunner::Controller *,
                    const bp::service::Description & desc) {
        // take over listening responsibilities of the controller
        m_executor->start(desc);
        // start parsing from the command line
        m_parser->start();
    }

    // unused overrides (because we pass off the controller to
    // the controller manager once it's initialized)
    void onAllocated(ServiceRunner::Controller *, unsigned int,
                     unsigned int) { }
    void onInvokeResults(ServiceRunner::Controller *,
                         unsigned int,
                         unsigned int,
                         const bp::Object *) { }
    void onInvokeError(ServiceRunner::Controller *,
                       unsigned int,
                       unsigned int,
                       const std::string &,
                       const std::string &) { }
    void onCallback(ServiceRunner::Controller *,
                    unsigned int, unsigned int, long long int,
                    const bp::Object *) { }
    void onPrompt(ServiceRunner::Controller *, unsigned int,
                  unsigned int,
                  const bp::file::Path &,
                  const bp::Object *) { }
};

class MyServiceFilter : public virtual IServiceFilter {
public:
    virtual ~MyServiceFilter() {}
    virtual bool serviceMayRun(const std::string&, const std::string&) const { return true; }
};

bool
downloadRequires(const std::list<std::string>& distroServers, bp::service::Summary s, bp::file::Path& downloadPath, bp::file::Path& certFile, ServiceList& pathList, bool useInstalled) {
    // generate list of ServiceRequireStatements
    std::list<ServiceRequireStatement> reqStmts;
    ServiceRequireStatement reqStmt;
    reqStmt.m_name = s.usesService();
    reqStmt.m_version = s.usesVersion().asString();
    reqStmt.m_minversion = s.usesMinversion().asString();
    reqStmts.push_back(reqStmt);
    // satisfy requirements
    MyServiceFilter sf;
    DistQuery dq(distroServers, &sf);
    ServiceUpdateListener serviceUpdateListener(&dq, downloadPath, certFile);
    dq.setListener(&serviceUpdateListener);
    std::list<bp::service::Summary> installed;
    unsigned int tid = dq.satisfyRequirements(bp::os::PlatformAsString(), reqStmts, installed);
    if (tid == 0) {
        return false;
    }
    // execute everything
    s_rl.run();
    // done
    if (serviceUpdateListener.DidDownloadSucceed()) {
        pathList = serviceUpdateListener.CompletedServices();
    }
    return serviceUpdateListener.DidDownloadSucceed();
}

int
main(int argc, const char ** argv)
{
    // handle non-ascii args on win32
    APT::ARGVConverter conv;
    conv.convert(argc, argv);

    // Services run in a separate process.  Running services in a separate
    // process requires a distinct executable, given combined limitations
    // of win32 and osx, and given the fact that services themselves are
    // dynamic libraries.
    //
    // To solve this problem we typically link code required to run a service
    // (logic to communicate with a host process over IPC, and to load and
    //  interface the service, logging, etc), INTO the host process.  command
    // line arguments then let us disambiguate between the host process, 
    // and a spawned parent process.  This tactic keeps all logic required
    // to run services in a single file and reduces administrative overhead
    // as well as decreasing the likelyhood that things will get out of sync
    // (i.e. - version mismatch between host process and service process) 
    //
    // The ServiceRunner library exposes a function (runServiceProcess)
    // which handles all aspects of running a spawned service.  The fact that
    // this function is in a library makes it easy to develop new binaries
    // capable of running services.
    // (lth/27.04.2009)
    if (argc > 1 && !std::string("-runService").compare(argv[1])) {
        // optional -breakpoint arguments set forced breaks
        std::list<std::string> breakpoints;
        for (int i = 2; i < argc; i++) {
            if (!std::string(argv[i]).compare("-breakpoint")) {
                if ((i + 1) < argc) {
                    breakpoints.push_back(std::string(argv[i + 1]));
                    i++; // go past this value
                }
            }
        }
        bp::debug::setForcedBreakpoints(breakpoints);
        if (ServiceRunner::runServiceProcess(argc, argv)) return 0;
        return 1;
    }

    s_rl.init();

    // parse command line arguments
    APTArgParse argParser(
        " Run a BrowserPlus service.\n"
        "usage: ServiceRunner [options] [ <service> <version> | <path> ]");
    int x = argParser.parse(sizeof(g_args)/sizeof(g_args[0]), g_args,
                            argc, argv);

    if (argParser.argumentPresent("slave")) {
        // in slave mode all output is json formatted and prompt printing
        // is surpressed
        output::setSlaveMode();
        CommandParser::setPrompt(NULL);
    }
    

    if (x < 0) {
        output::puts(output::T_ERROR, argParser.error());
        exit(1);
    } else if (argParser.argumentPresent("version")) {
        output::puts(output::T_RESULTS, std::string(BPVERSION));
        exit(0);
    } else if (x != argc - 2 && x != argc - 1) {
        output::puts(output::T_ERROR, "missing service & version or path argument");
        exit(1);
    }

    setupLogging(argParser);
    
    // scope here so that all objects are cleaned up by end of main
    {
        shared_ptr<ServiceRunner::Controller> controller;

        if (x == argc - 2) {
            std::string service(argv[argc - 2]), version(argv[argc - 1]);
            controller.reset(new ServiceRunner::Controller(service, version));
        } else {
            bp::file::Path path(argv[argc - 1]);
            controller.reset(new ServiceRunner::Controller(path));            
        }

        if (argParser.argumentPresent("debugService")) {
            // manually push debugBreak to controller since there is no bp.config
            std::list<std::string> breakpoints;
            breakpoints.push_back("runServiceProcess");
            controller->setDebugBreakpoints(breakpoints);
        }

        // now let's parse the manifest and determine if this is a dependent
        bp::service::Summary s;
        std::string err;
        if (!s.detectService(controller->path(), err)) {
            std::stringstream ss;
            ss << "couldn't load service: " << err;
            output::puts(output::T_ERROR, ss.str());
            exit(1);
        }

        bp::file::Path providerPath;
        // if a path was specified on the command line, we'll
        // use that
        if (argParser.argumentPresent("providerPath")) {
            providerPath = argParser.argument("providerPath");
        }
        
        // for dependent services we'll need to concoct a reasonable
        // path to the provider service
        if (s.type() == bp::service::Summary::Dependent) {
            bp::file::Path downloadPath;
            // This should be the location to the provider services.
            std::list<std::string> distroServers;
            if (argParser.argumentPresent("distroServer")) {
                distroServers.push_back(argParser.argument("distroServer"));
            }
            else {
                distroServers.push_back("http://browserplus.yahoo.com");
            }
            if (argParser.argumentPresent("downloadPath")) {
                ServiceList pathList;
                downloadPath = argParser.argument("downloadPath");
                // This should be the location to the provider services.
                bp::file::Path certFile;
                if (argParser.argumentPresent("certFile")) {
                    certFile = argParser.argument("certFile");
                }
                else {
                    // Default cert file distributes in same directory as current executable.
                    // Try that one if none passed in with command-line arguments.
                    bp::file::Path p1 = bp::file::programPath();
                    certFile = p1.parent_path() / "BrowserPlus.crt";
                }
                if (!downloadRequires(distroServers, s, downloadPath, certFile, pathList, false)) {
                    std::stringstream ss;
                    ss << "Couldn't run service because I couldn't "
                       << "find an appropriate installed " << std::endl
                       << "provider service satisfying: "  << std::endl
                       << "  name:       " << s.usesService()
                       << std::endl
                       << "  version:    " << s.usesVersion().asString()
                       << std::endl
                       << "  minversion: "
                       << s.usesMinversion().asString()
                       << std::endl
                       << "(" << err << ")";
                    output::puts(output::T_ERROR, ss.str());
                    exit(1);
                }
                // NEEDSWORK!!  ServiceRunner is stupid.
                if (pathList.size() > 1) {
                    std::stringstream ss;
                    ss << "Couldn't run service because too many "
                       << "upstream services required: " << std::endl;
                    for (ServiceList::const_iterator i = pathList.begin(); i != pathList.end(); i++) {
                        ss << "  " << i->first << " v" << i->second << std::endl;
                    }
                    output::puts(output::T_ERROR, ss.str());
                    exit(1);
                }
                // Set providerPath now, based on where we download to.
                if (pathList.size() == 1) {
                    ServiceList::const_iterator i = pathList.begin();
                    providerPath = downloadPath / i->first / i->second;
                }
            }
        }
        s_rl.init();

        // specify our own binary as the "harness program"
        bp::file::Path harnessProgram = bp::file::canonicalProgramPath(bp::file::Path(argv[0]));

        // let's try to figure out a meaningful process title
        std::string processTitle, summary;

        /* TODO: extract and pass actual locale */
        if (!s.localization("en", processTitle, summary))
        {
            processTitle.append("BrowserPlus: Spawned Service");
        }
        else
        {
            processTitle = (std::string("BrowserPlus: ") + processTitle);
        }

        // now start the controller
        if (!controller->run(harnessProgram, providerPath, processTitle,
                             argParser.argument("log"),
                             bp::file::Path(argParser.argument("logfile")),
                             err))
        {
            std::stringstream ss;
            ss << "couldn't spawn service at: " << controller->path() << " - "
               << err << std::endl;
            output::puts(output::T_ERROR, ss.str());
            exit(1);
        }
        
        // allocate a command parser
        CommandParserPtr parser(new CommandParser);

        // allocate a class that handles commands
        shared_ptr<CommandExecutor> chp(new CommandExecutor(controller));

        // allocate an object that listens for initialized and
        // quit events from the command parser and starts the parser,
        // and stops the application
        RunManager ql(parser, chp);
        controller->setListener(&ql);

        // set up the quit listener to listen to the command parser
        parser->setListener(&ql);

        // register functions from the command handler on the command parser
        parser->registerHandler(
            "allocate", chp,
            BP_COMMAND_HANDLER(CommandExecutor::allocate),
            0, 1,
            "Allocate a new service instance.  An optional arguyment is "
            "a URI, when provided it will be passed into the service "
            "allocated as the uri of the \"page\" that initialized "
            "BrowserPlus, which allows for testing of code that uses "
            "the URI (for cross domain checks, scoped storage, etc)");

        parser->registerHandler(
            "describe", chp,
            BP_COMMAND_HANDLER(CommandExecutor::describe),
            0, 0,
            "Extract and display the API description of the service.");

        parser->registerHandler(
            "destroy", chp,
            BP_COMMAND_HANDLER(CommandExecutor::destroy),
            1, 1,
            "Destroy a service instance, the single required parameter is "
            "the numeric instance id.");

        parser->registerHandler(
            "invoke", chp,
            BP_COMMAND_HANDLER(CommandExecutor::invoke),
            1, 2,
            "Invoke a function on a service instance.  First argument is "
            "function name, second are json encoded arguments.  Within JSON "
            "encoded arguments, one may pass Path and WritablePath types by "
            "prepending 'path:' or 'writable_path:' to the native file path "
            "represented as UTF8.");

        parser->registerHandler(
            "select", chp,
            BP_COMMAND_HANDLER(CommandExecutor::select),
            1, 1,
            "Select the current instance, this is a numeric id and identifies "
            "the instance to which method invocations will be routed.  The "
            "show command will display allocated instances (by id) and the "
            "currently selected instance.");

        parser->registerHandler(
            "show", chp,
            BP_COMMAND_HANDLER(CommandExecutor::show),
            0, 0,
            "Show the currently allocated service instances.");

        parser->registerHandler(
            "prompts", chp,
            BP_COMMAND_HANDLER(CommandExecutor::prompts),
            0, 0,
            "Show the outstanding user prompts that have been sent by "
            "the service (you may use 'respond' to respond to these "
            "user prompts)");

        parser->registerHandler(
            "respond", chp,
            BP_COMMAND_HANDLER(CommandExecutor::respond),
            1, 2,
            "Respond to a prompt from the service.  The first argument is "
            "the prompt ID (see 'prompts' command), the second (optional) "
            "argument is a json string containing your respond");

        s_rl.run();
        parser->stop();    
    }

    s_rl.shutdown();

    return 0;
}
