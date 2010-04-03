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

/*
 * An abstraction which is employed by a forked process to 
 * load a specified service and communicate back with the forker.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/14
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "Process.h"
#include <iostream>
#include <stdlib.h>
#include "BPUtils/APTArgParse.h"
#include "BPUtils/bpconfig.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpdebug.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/ProductPaths.h"
#include "OutputRedirector.h"
#include "ServiceLibrary.h"
#include "ServiceProtocol.h"


static void 
setupLogging(std::string config, bp::file::Path path)
{
    if (config.empty()) {
        config = "info";
    }
    
    // Setup the system-wide minimum log level.
    bp::log::setLogLevel(bp::log::levelFromConfig(config));
    
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();

    // Get log time format from bp.config file.
    // If we can't get it for any reason, default (empty str) is fine.
    std::string timeFormat;
    bp::config::ConfigReader reader;
    if (reader.load(bp::paths::getConfigFilePath())) {
        (void) reader.getStringValue("LogTimeFormat",timeFormat);
    }

    if (path.empty()) {
        bp::log::setupLogToConsole(config,"",timeFormat);
    } else {
        // Services open the daemon logfile in append mode.
        // Deleting the log file is done by BPDaemon.cpp: setupLogging().
		bp::log::setupLogToFile(path,config,bp::log::kAppend,timeFormat);
    }
}

bool
ServiceRunner::runServiceProcess(int argc, const char ** argv)
{
    // Offer developers the option to attach a debugger here.
    bp::debug::breakpoint( "runServiceProcess" );
    
    OutputRedirector redirector;

    static APTArgDefinition args[] = {
        { "runService", APT::NO_ARG, APT::NO_DEFAULT, APT::REQUIRED,
          APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
          "required cmd line arg indicating that we're running a service"
        },
        { "ipcName", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
          APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
          "the name of the ipc channel that we should connect to once "
          "the service is loaded."
        },
        { "providerPath", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
          APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
          "When running dependent services, the provider must be explicitly "
          "specified."
        },
        { "log", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
          APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
          "Enable logging and specify a level to run at.  Default is 'info'. "
          "if 'logfile' is not also specified, logging will occur to console. "
        },
        { "logfile", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
          APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
          "Enable logging to a specified file."
        }
    };


    std::string pName(argv[0]);
    pName.append(": Service Harness");
    APTArgParse argParser(pName);
    int x = argParser.parse(sizeof(args)/sizeof(args[0]), args,
                            argc, argv);

    // verify we have correct arguments
    if (x < 0) {
        std::cerr << argParser.error() << std::endl;
        return false;
    }

    std::string ipcName = argParser.argument("ipcName");
    bp::file::Path providerPath(argParser.argument("providerPath"));

    // absolute first order of business, enable logging!
    if (argParser.argumentPresent("log") ||
        argParser.argumentPresent("logfile"))
    {
        std::string loglevel = argParser.argument("log");
        bp::file::Path logfile(argParser.argument("logfile"));
        setupLogging(loglevel, logfile);

        // if we're redirecting to a file we'll intercept stdout && stderr  
        if (!logfile.empty()) {
            redirector.redirect();
        }
    }

    BPLOG_INFO("Service Process running");

    std::string err;

    bp::runloop::RunLoop rl;
    rl.init();

    // Allocate service and parse manifest
    ServiceLibrary lib;
    bool parsed = lib.parseManifest(err);

    if (!parsed) {
        BPLOG_ERROR_STRM("Couldn't parse service manifest: " << err);
        return false;
    }

    {
        bp::time::Stopwatch sw;
        sw.start();

        // attempt to load service
        if (!lib.load(providerPath, err)) {
            BPLOG_ERROR_STRM("Couldn't load service"
                             << (err.length() ? ": " + err : "."));
            return false;
        }
        
        BPLOG_INFO_STRM("Service Process loaded " << lib.name() << " v"
                        << lib.version() << " successfully in "
                        << sw.elapsedSec() << "s");
    }
    
    // allocate and connect protocol handler,
    // this will also send the first message across containing the
    // true service name and version as extracted from the dynamically
    // loaded library (or ruby file, whatever)
    BPLOG_INFO_STRM("Service Process (" << lib.name() << ") connecting to ipc: "
                    << ipcName);

    ServiceProtocol proto(&lib, &rl, ipcName);
    if (!proto.connect()) {
        BPLOG_WARN("Service Process couldn't connect to controller, exiting");
        return false;
    }
    
    // enter runloop and await further instructions
    rl.run();

    rl.shutdown();

    BPLOG_INFO("Service Process exiting");

    return true;
}
