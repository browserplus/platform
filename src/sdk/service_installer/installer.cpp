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

#include "BPUtils/APTArgParse.h"
#include "BPUtils/ARGVConverter.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "platform_utils/ProductPaths.h"
#include "platform_utils/ServicesUpdatedFile.h"
#include "ServiceRunnerLib/ServiceRunnerLib.h"

using namespace std;
using namespace std::tr1;


static bp::runloop::RunLoop s_rl;

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

// A class which will serve as a listener to the service
// controller, primary responsibility is to extract the services
// description and handle errors along the way.
/** a class to listen for UserQuitEvents and stop the runloop upon
 *  their arrival */
class ServiceManager :  public ServiceRunner::IControllerListener
{
public:
    ServiceManager(shared_ptr<APTArgParse> argParser)
        : m_argParser(argParser)
    {
    }

    bp::service::Description description() { return m_desc; }

private:   
    bp::service::Description m_desc;

    void initialized(ServiceRunner::Controller * c,
                     const std::string & service,
                     const std::string & version,
                     unsigned int) 
    {
        if (m_argParser->argumentPresent("v")) {
            std::cout << "service initialized: "
                      << service << " v" << version << std::endl;        
        }
        c->describe();
    }
    void onEnded(ServiceRunner::Controller * c) 
    {
        std::cerr << "Spawned service exited!  (enable logging for more "
                  << "detailed diagnostics - '-log debug')."
                  << std::endl;        
        // hard exit!
        exit (1);
    }
    void onDescribe(ServiceRunner::Controller *,
                    const bp::service::Description & desc)
    {
        // we've extracted a description of the service, let's save it.
        m_desc = desc;

        // stop the runloop, returning control to main
        s_rl.stop();
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
    void onCallback(ServiceRunner::Controller *, unsigned int,
                    unsigned int, long long int, const bp::Object *) { }
    void onPrompt(ServiceRunner::Controller *, unsigned int,
                  unsigned int,
                  const bp::file::Path &,
                  const bp::Object *) { }

    shared_ptr<APTArgParse> m_argParser;
};




// Helper func to make timestamp strings.
std::string timeStamp( bp::time::Stopwatch& sw )
{
    std::stringstream ss;
    ss << "[" << sw.elapsedSec() << "s] ";
    return ss.str();
}

static void 
setupLogging(shared_ptr<APTArgParse> argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();

    std::string level = argParser->argument("log");
    bp::file::Path path(argParser->argument("logfile"));

    if (level.empty() && path.empty()) return;
    
    if (level.empty()) level = "info";

    bp::log::Level logLevel = bp::log::levelFromString(level);

    if (path.empty()) bp::log::setupLogToConsole(logLevel);
    else bp::log::setupLogToFile(path, logLevel);
}

int main(int argc, const char ** argv)
{
    // handle non-ascii args on windows
    APT::ARGVConverter conv;
    conv.convert(argc, argv);

    // which mode are we running in?
    if (argc > 1 && !std::string("-runService").compare(argv[1])) {
        if (ServiceRunner::runServiceProcess(argc, argv)) return 0;
        return 1;
    }

    s_rl.init();

    // a stopwatch for timing
    bp::time::Stopwatch sw;
    sw.start();

    // win32 specific call to prevent display to user of useless
    // dialog boxes
#ifdef WIN32
    SetErrorMode(SetErrorMode(0) | SEM_NOOPENFILEERRORBOX |
                 SEM_FAILCRITICALERRORS);
#endif

    // make a usage string
    std::string usage("Install a BrowserPlus service.\n    ");
    usage.append(argv[0]);
    usage.append(" [opts] <service dir>");

    // parse command line arguments
    shared_ptr<APTArgParse> argParser(new APTArgParse(usage));
    
    int x = argParser->parse(sizeof(g_args)/sizeof(g_args[0]), g_args,
                             argc, argv);
    if (x < 0) {
        std::cerr << argParser->error() << std::endl;
        exit(1);
    }
    else if (x != (argc - 1))
    {
        std::cerr << "invalid arguments: ";
        if (x > (argc - 1)) { 
            std::cerr << "path to service required";
        } else {
            std::cerr << "extra command line arguments detected";
        }
        std::cerr << std::endl;
        exit(1);
    }

    setupLogging(argParser);

    std::string error;
    bp::file::Path absPath = bp::file::canonicalPath(bp::file::Path(argv[x]));

    bp::service::Summary summary;
    
    if (argParser->argumentPresent("t")) {
        std::cout << timeStamp(sw)
                  << "detecting service at: " << absPath << "\n";
    }
    
    if (!summary.detectService(absPath, error))
    {
        std::cerr << "Invalid service: "
                  << std::endl << "  " << error << std::endl;
        exit(1);
    }
    
    bp::file::Path providerPath;
    
    // now let's find a valid provider if this is a dependent
    if (summary.type() == bp::service::Summary::Dependent)
    {
        std::string err;
        
        providerPath = ServiceRunner::determineProviderPath(summary, err);
    
        if (!err.empty()) {
            std::cerr << "Couldn't run service because I couldn't "
                      << "find an appropriate installed " << std::endl
                      << "provider service."  << std::endl;
            exit(1);
        }        
    }

    if (argParser->argumentPresent("t")) {
        std::cout << timeStamp(sw) << "service valid.\n";
    }

    // we're ready to load the service and extract it description
    shared_ptr<ServiceManager> serviceMan(new ServiceManager(argParser));
    shared_ptr<ServiceRunner::Controller> controller(
        new ServiceRunner::Controller(absPath));
    controller->setListener(serviceMan.get());
    
    // pathToHarness is ourself
    bp::file::Path harnessProgram = bp::file::canonicalProgramPath(bp::file::Path(argv[0]));

    // determine a reasonable title for the spawned service
    std::string processTitle, ignore;
    /* TODO: extract and pass proper locale! */
    if (!summary.localization("en", processTitle, ignore))
    {
        processTitle.append("BrowserPlus: Spawned Service");
    }
    else
    {
        processTitle = (std::string("BrowserPlus: ") + processTitle);
    }

    if (!controller->run(harnessProgram, providerPath,
                         processTitle, argParser->argument("log"),
                         bp::file::Path(argParser->argument("logfile")),
                         error))
    {
        std::cerr << "Couldn't run service: " << error.c_str()
                  << std::endl;
        exit(1);
    }
    

    // now run.  by the time the runloop is stopped we'll have a service
    // description available
    s_rl.run();

    if (argParser->argumentPresent("t")) {
        std::cout << timeStamp(sw) << "description received.\n";
    }

    // extract the description and clean up what we no longer need
    controller.reset();
    bp::service::Description desc = serviceMan->description();
    serviceMan.reset();
    s_rl.shutdown();

    // attain convenient representation of name and version of service
    // we're publishing
    std::string serviceName(desc.name());
    std::string serviceVersion(desc.versionString());

    // now switch between local installation and pushing to distribution
    // server
    if (!argParser->argumentPresent("n")) {
        bp::file::Path source = summary.path();

        if (argParser->argumentPresent("t")) {
            std::cout << timeStamp(sw)
                      << "preparing installation directory...\n";
        }

        bool overwrote = false;
        bp::file::Path destination = bp::paths::getServiceDirectory() /serviceName / serviceVersion;
        if (bp::file::exists(destination)) {
            if (argParser->argumentPresent("f")) {
                if (!bp::file::remove(destination)) {
                    std::cerr << "error deleting '" << destination << "'"
                              << std::endl;
                    exit(1);
                } 
                overwrote = true;
            } else {
                std::cerr << "can't install service, directory already "
                          << "exists and -f not specfied" << std::endl;
                exit(1);
            }
        }

        // for local service installation all we do is copy it to the
        // local service directory.
        try {
            boost::filesystem::create_directories(destination);
        } catch(const bp::file::tFileSystemError&) {
            std::cerr << "cannot create directory: '" << destination << "'"
                      << std::endl;
            exit(1);
        }

        if (argParser->argumentPresent("v")) {
            std::cout << "installing service locally: "
                      << serviceName << ", ver "
                      << serviceVersion
                      << std::endl;
        }
        
        if (bp::file::isDirectory(source)) {
            try {
                bp::file::tDirIter end;
                for (bp::file::tDirIter it(source); it != end; ++it) {
                    bp::file::Path p(it->path());
                    bp::file::Path relPath = p.relativeTo(source);
                    bp::file::Path fromPath = it->path();
                    bp::file::Path toPath = destination / relPath;
                    if (!bp::file::copy(fromPath, toPath)) {
                        std::cerr << "error copying " << fromPath << std::endl;
                        exit(1);
                    }
                }
            } catch (const bp::file::tFileSystemError& e) {
                std::cerr << "unable to iterate thru " << source.externalUtf8()
                        << ": " << e.what();
            }
        }

        if (argParser->argumentPresent("t")) {
            std::cout << timeStamp(sw) << "copying complete.\n";
        }

        // let's update the moddate on the manifest.json to cause
        // BrowserPlus to update this service.  This is only necessary
        // if we overwrote the on-disk service
        if (overwrote) {
            bp::file::Path manifestPath = destination / "manifest.json";
            bp::file::touch(manifestPath);
        }

        // now we'll indicate that services have changed for a running
        // BrowserPlusCore daemon
        ServicesUpdated::indicateServicesChanged();
    }
    
    // all done!
    if (argParser->argumentPresent("v")) {    
        std::cout << serviceName << " "
                  << serviceVersion
                  << " validated "
                  << (argParser->argumentPresent("n") ? "" : "and installed ")
                  << "in "
                  << sw.elapsedSec() << "s" << std::endl;
    }

    return 0;
}
