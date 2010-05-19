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

#include "BPUtils/ARGVConverter.h"
#include "BPUtils/APTArgParse.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "ConsoleLib/ConsoleLib.h"

// here's our implementation of handling commands
#include "CommandExecutor.h"

using namespace std;
using namespace std::tr1;


bp::runloop::RunLoop s_rl;

static APTArgDefinition g_args[] = {
    { "log", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "enable console logging, argument like \"info\" or \"debug\""
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
        std::cerr << "shutting down..." << std::endl;
        s_rl.stop();
    }
    void initialized(ServiceRunner::Controller * c,
                     const std::string & service,
                     const std::string & version,
                     unsigned int) 
    {
        std::cout << "service initialized: "
                  << service << " v" << version << std::endl;        
        c->describe();
    }
    void onEnded(ServiceRunner::Controller * c) 
    {
        std::cerr << "Spawned service exited!  (enable logging for more "
                  << "detailed diagnostics - '-log debug')."
                  << std::endl;        
        // hard exit
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
    if (x < 0) {
        std::cerr << argParser.error() << std::endl;
        exit(1);
    } else if (argParser.argumentPresent("version")) {
        std::cout << BPVERSION << std::endl;
        exit(0);
    } else if (x != argc - 2 && x != argc - 1) {
        std::cerr << "missing service & version or path argument\n"
                  << std::endl;
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
        

        // now let's parse the manifest and determine if this is a dependent
        bp::service::Summary s;
        std::string err;
        if (!s.detectCorelet(controller->path(), err)) {
            std::cerr << "couldn't load service: " << err << std::endl;
            exit(1);
        }

        bp::file::Path providerPath;
        
        // for dependent services we'll need to concoct a reasonable
        // path to the provider service
        if (s.type() == bp::service::Summary::Dependent) {
            // if a path was specified on the command line, we'll
            // use that
            if (argParser.argumentPresent("providerPath")) {
                providerPath = argParser.argument("providerPath");
            } else {
                std::string err;

                // determine the path without using corelet manager
                providerPath = ServiceRunner::determineProviderPath(s, err);
                if (!err.empty()) {
                    std::cerr << "Couldn't run service because I couldn't "
                              << "find an appropriate installed " << std::endl
                              << "provider service satisfying: "  << std::endl
                              << "  name:       " << s.usesCorelet()
                              << std::endl
                              << "  version:    " << s.usesVersion().asString()
                              << std::endl
                              << "  minversion: "
                              << s.usesMinversion().asString()
                              << std::endl
                              << "(" << err << ")" << std::endl;
                    exit(1);
                }
            }
        }

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
            std::cerr << "couldn't spawn service at: "
                      << controller->path() << " - "
                      << err << std::endl;
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
            0, 0,
            "Allocate a new service instance.");

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
            "function name, second are json encoded arguments.");

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
