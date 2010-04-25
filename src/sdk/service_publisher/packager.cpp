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
 * A tool to package a service into a format that may be pushed to
 * the distribution site.
 */

#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/APTArgParse.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"
#include "ServiceRunnerLib/ServiceRunnerLib.h"

// the gunk to do the push
#include "PusherMan.h"

using namespace std;
using namespace std::tr1;


#define DESC_FNAME "description.json"
// old style strings (2.0.6 and before, no permissions)
#define STRINGS_FNAME "strings.json"
// new style strings (2.0.7 and beyond, includes permissions)
#define SYNOPSIS_BPKG_FNAME "synopsis.bpkg"
#define TMPDIR_PREFIX "BrowserPlus"

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
      "be verbose"
    },
    { "t", APT::NO_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "echo detailed timing info"
    },
    { "privateKey", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "private signing key"
    },
    { "publicKey", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "public signing key"
    },
    { "password", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "signing key password"
    },
    { "p", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
      "specify the platform for which this service is written.  Choices are "
      " \"ind\" for platform independent services, \"win32\" for 32-bit "
      " windows services, or \"osx\" for intel architecture 32-bit services "
    }
};


static string
prettyName( const string& service, const string& version,
            const string& platform = "" )
{
    string ret = service + " " + version;
    if (!platform.empty()) {
        ret += " (" + platform + ")";
    }
    
    return ret;
}


// A class which will serve as a listener to the service
// controller, primary responsibility is to extract the services
// description and handle errors along the way.
/** a class to listen for UserQuitEvents and stop the runloop upon
 *  their arrival */
class ServiceManager :  public ServiceRunner::IControllerListener
{
public:
    ServiceManager(shared_ptr<APTArgParse> argParser)
        : m_apiVersion(), m_argParser(argParser)
    {
    }

    bp::service::Description description() { return m_desc; }

    unsigned int apiVersion() { return m_apiVersion; }

private:   
    bp::service::Description m_desc;
    unsigned int m_apiVersion;

    void initialized(ServiceRunner::Controller * c,
                     const string & service,
                     const string & version,
                     unsigned int apiVersion) 
    {
        m_apiVersion = apiVersion;
        if (m_argParser->argumentPresent("v")) {
            cout << "service initialized: "
                 << prettyName(service,version) << endl;        
        }
        c->describe();
    }
    void onEnded(ServiceRunner::Controller * c) 
    {
        cerr << "Spawned service exited!  (enable logging for more "
             << "detailed diagnostics - '-log debug')."
             << endl;        
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
                         unsigned int, unsigned int,
                         const bp::Object *) { }
    void onInvokeError(ServiceRunner::Controller *, unsigned int,
                       unsigned int, const string &,
                       const string &) { }
    void onCallback(ServiceRunner::Controller *, unsigned int,
                    unsigned int, long long int, const bp::Object *) { }
    void onPrompt(ServiceRunner::Controller *, unsigned int,
                  unsigned int, const bp::file::Path &, const bp::Object *) { }

    shared_ptr<APTArgParse> m_argParser;
};

// Helper func to make timestamp strings.
string timeStamp( bp::time::Stopwatch& sw )
{
    stringstream ss;
    ss << "[" << sw.elapsedSec() << "s] ";
    return ss.str();
}

static void 
setupLogging(shared_ptr<APTArgParse> argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();

    string level = argParser->argument("log");
    bp::file::Path path(argParser->argument("logfile"));

    if (level.empty() && path.empty()) return;
    
    if (level.empty()) level = "info";

	bp::log::Level logLevel = bp::log::levelFromString(level);

    if (path.empty()) bp::log::setupLogToConsole(logLevel);
    else bp::log::setupLogToFile(path, logLevel);
}

int main(int argc, const char ** argv)
{
    try {
        // which mode are we running in?
        if (argc > 1 && !string("-runService").compare(argv[1])) {
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
        string usage("Publish a BrowserPlus service.\n    ");
        usage.append(argv[0]);
        usage.append(" [opts] <service dir> <base url to webservice>");

        // parse command line arguments
        shared_ptr<APTArgParse> argParser(new APTArgParse(usage));

        int x = argParser->parse(sizeof(g_args)/sizeof(g_args[0]), g_args,
                                 argc, argv);
        if (x < 0) {
            cerr << argParser->error() << endl;
            exit(1);
        }
        else if (x != (argc - 2))
            {
                cerr << "missing required command line arguments" << endl;
                cerr << argParser->usage() << endl; 
                exit(1);
            } 

        setupLogging(argParser);

        string error;
        bp::file::Path absPath = bp::file::canonicalPath(bp::file::Path(argv[x]));

        bp::service::Summary summary;
    
        if (argParser->argumentPresent("t")) {
            cout << timeStamp(sw) << "detecting service at: "
                 << absPath << "\n";
        }
    
        if (!summary.detectCorelet(absPath, error))
            {
                cerr << "invalid service: " << error << endl;
                exit(1);
            }

        if (argParser->argumentPresent("t")) {
            cout << timeStamp(sw) << "service valid.\n";
        }

        string baseURL(argv[x+1]);

        bp::file::Path providerPath;
    
        // now let's find a valid provider if this is a dependent
        if (summary.type() == bp::service::Summary::Dependent)
            {
                string err;
        
                providerPath = ServiceRunner::determineProviderPath(summary, err);
    
                if (!err.empty()) {
                    cerr << "Couldn't run service because I couldn't "
                         << "find an appropriate installed " << endl
                         << "provider service."  << endl;
                    exit(1);
                }        
            }

        if (argParser->argumentPresent("t")) {
            cout << timeStamp(sw) << "service valid.\n";
        }

        // we're ready to load the service and extract it description
        shared_ptr<ServiceManager> serviceMan(new ServiceManager(argParser));
        shared_ptr<ServiceRunner::Controller> controller(
            new ServiceRunner::Controller(absPath));
        controller->setListener(serviceMan.get());
    
        // pathToHarness is ourself
        bp::file::Path harnessProgram = bp::file::canonicalProgramPath(bp::file::Path(argv[0]));

        // determine a reasonable title for the spawned service
        string processTitle, ignore;

        /* TODO: extract and pass proper locale */
        if (!summary.localization("en", processTitle, ignore))
            {
                processTitle.append("BrowserPlus: Spawned Service");
            }
        else
            {
                processTitle = (string("BrowserPlus: ") + processTitle);
            }

        if (!controller->run(harnessProgram, providerPath,
                             processTitle, argParser->argument("log"),
                             bp::file::Path(argParser->argument("logfile")),
                             error))
            {
                cerr << "Couldn't run service: " << error.c_str() << endl;
                exit(1);
            }
    

        // now run.  by the time the runloop is stopped we'll have a service
        // description available
        s_rl.run();

        if (argParser->argumentPresent("t")) {
            cout << timeStamp(sw) << "description received.\n";
        }

        // extract the description and clean up what we no longer need
        controller.reset();
        bp::service::Description desc = serviceMan->description();
        unsigned int coreletAPIVersion = serviceMan->apiVersion();

        serviceMan.reset();
        s_rl.shutdown();

        // attain convenient representation of name and version of corelet
        // we're publishing
        string coreletName(desc.name());
        string coreletVersion(desc.versionString());


        // publish to distro server
        if (!argParser->argumentPresent("privateKey")) {
            cerr << "Private signing key must be "
                 << "specified with -privateKey option" << endl;
            exit(1);
        }
        bp::file::Path privateKey(argParser->argument("privateKey"));
    
        if (!argParser->argumentPresent("publicKey")) {
            cerr << "Public signing key must be "
                 << "specified with -publicKey option" << endl;
            exit(1);
        }
        bp::file::Path publicKey(argParser->argument("publicKey"));

        string password;
        if (argParser->argumentPresent("password")) {
            password = argParser->argument("password");
        }
        
        if (!argParser->argumentPresent("p")) {
            cerr << "when publishing to distribution server you "
                 << "must specify the platform for" << endl
                 << "which the service you are pushing is written "
                 << "choices are: " << endl
                 << "  osx - intel architecture 32-bit services for osx"
                 << endl
                 << "  win32 - 32-bit windows services"
                 << endl
                 << "  ind - platform independent services"
                 << endl;

            exit(1);
        }
        string platform(argParser->argument("p"));
        
        if (0 != platform.compare("win32") &&
            0 != platform.compare("osx") &&
            0 != platform.compare("ind"))
            {
                cerr << "'" << platform
                     << "' is a invalid parameter to -p argument. "
                     << "-h for help." << endl;
                exit(1);
            }

        cout << "packaging service: "
             << prettyName(coreletName,coreletVersion,platform)
             << " at " << summary.path() << endl;

        bp::file::Path targetPath = bp::file::getTempDirectory();

        // get a good name for the intermediate directory
        targetPath = bp::file::getTempPath(targetPath, TMPDIR_PREFIX);
        if (!boost::filesystem::create_directories(targetPath)) {
            cerr << "couldn't create temp directory: " << targetPath << endl;
            exit(1);
        }

        // now we must generate description.json from the desc pointer
        // to <ouput directory>/description.json
        {
            string buf;
            bp::Object* o = desc.toBPObject();
            if (o == NULL || o->type() != BPTMap) {
                cerr << "Error serializing description!" << endl;
                exit(1);
            } 
            bp::Map * m = (bp::Map *) o;
            // now we must add corelet type to the description
            m->add("CoreletType", new bp::String(summary.typeAsString()));

            // for standalone or provider corelets, we include the
            // corelet API version.  For dependent corelets, we include
            // The corelet that they depend on.
            if (summary.type() == bp::service::Summary::Dependent) {
                // add CoreletRequires key containing Name, Minversion
                // and Version
                bp::Map * requiresMap = new bp::Map;

                requiresMap->add("Name",
                                 new bp::String(summary.usesCorelet()));
                requiresMap->add("Version",
                                 new bp::String(summary.usesVersion().asString()));

                requiresMap->add(
                                 "Minversion",
                                 new bp::String(summary.usesMinversion().asString()));

                m->add("CoreletRequires", requiresMap);
                
            } else {
                m->add("CoreletAPIVersion",
                       new bp::Integer(coreletAPIVersion));
                // zero never has nor never will be a valid corelet api
                // version
                BPASSERT(coreletAPIVersion != 0);
            }
            
            buf = m->toPlainJsonString();

            bp::file::Path descPath = targetPath / DESC_FNAME;

            if (!bp::strutil::storeToFile(descPath, buf)) {
                cerr << "error writing " << descPath << endl;
                exit(1);
            }
        }

        // now we must generate strings.json from the localized strings
        // in CoreletSummary (manifest.json)
        // to <ouput directory>/strings.json
        {
            bp::Map stringsMap;
            list<string> locales = summary.localizations();
            list<string>::iterator it;

            for (it = locales.begin(); it != locales.end(); it++)
                {
                    string tit, sum;
                
                    bool x;
                    x = summary.localization(*it, tit, sum);
                    BPASSERT(x);

                    bp::Map * entry = new bp::Map;
                    entry->add("title", new bp::String(tit.c_str()));
                    entry->add("summary", new bp::String(sum.c_str()));
                    stringsMap.add((*it).c_str(), entry);
                }

            string buf = stringsMap.toPlainJsonString();
            bp::file::Path path = targetPath / STRINGS_FNAME;
            
            // strings.json is deprecated as of 2.0.7 and above
            if (!bp::strutil::storeToFile(path, buf)) {
                cerr << "error writing " << path << endl;
                exit(1);
            }

            // now we'll build the new style synopsis.mime.  differences
            // are:
            // 1. it's signed
            // 2. it includes permissions
            bp::Map metadataMap;
            metadataMap.add("localizations", stringsMap.clone());
            
            // now add in permissions
            bp::List permsList;
            set<string> perms = summary.permissions();
            set<string>::iterator sit;
            for (sit = perms.begin(); sit != perms.end(); sit++) {
                permsList.append(new bp::String(*sit));
            }
            metadataMap.add("permissions", permsList.clone());
            buf = metadataMap.toPlainJsonString();            

            // now package synopsis
            path = targetPath / SYNOPSIS_BPKG_FNAME;
            if (!bp::pkg::packString(privateKey, publicKey, password,
                                     buf, path)) {
                cerr << "error packaging synopsis to " << path << endl;
                exit(1);
            }
        }
    
        // now package corelet directory to <output directory>/corelet.bpkg
        bp::file::Path coreletPath = summary.path();
        bp::file::Path pkgPath = targetPath / "corelet.bpkg";
        if (!bp::pkg::packDirectory(privateKey, publicKey, password,
                                    coreletPath, pkgPath)) {
            cerr << "error packaging service contents to " << pkgPath << endl;
            exit(1);
        }
    
        // now package results
        bp::file::Path finalPkgPath = bp::file::getTempPath(targetPath, TMPDIR_PREFIX); 
        if (!bp::pkg::packDirectory(privateKey, publicKey, password,
                                    targetPath, finalPkgPath)) {
            cerr << "error packaging service to " << finalPkgPath << endl;
            exit(1);
        }
    
        // push it!
        if (!pushFile(finalPkgPath, baseURL,
                      coreletName, coreletVersion, platform)) {
            cerr << "error pushing service to " << baseURL << endl;
            exit(1);
        }
        
        // now delete intermediate directory 
        if (!bp::file::remove(targetPath)) {
            cerr << "warning: unable to delete directory "
                 << targetPath << endl;
            // warning only - no exit here.
        }
    
        // all done!
        if (argParser->argumentPresent("v")) {    
            cout << prettyName(coreletName,coreletVersion,platform)
                 << " published in " << sw.elapsedSec() << "s" << endl;
        }
    } catch (const bp::file::tFileSystemError& e) {
        cerr << "filesystem error: " << e.what()
             << ", path1 = " << bp::file::Path(e.path1())
             << ", path2 = " << bp::file::Path(e.path2());
        exit(1);
    }

    return 0;
}

