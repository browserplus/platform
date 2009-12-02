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

#include <iostream>
#include "BPUtils/APTArgParse.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bpfile.h"

// here's our implementation of handling commands
#include "CommandExecutor.h"

using namespace std;
using namespace std::tr1;


bp::runloop::RunLoop s_rl;

static APTArgDefinition g_args[] = {
    { "log", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "enable console logging, argument like \"info,ThrdLvlFuncMsg\""
    },
    { "logfile", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "Enable file logging, argument is a path, when combined with '-log' "
      "logging will occur to a file at the level specified."
    }
};

static void 
setupLogging(const APTArgParse& argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();

    std::string level = argParser.argument("log");
    bp::file::Path path(argParser.argument("logfile"));

    if (level.empty() && path.empty()) return;
    
    if (level.empty()) level = "info";

    if (path.empty()) bp::log::setupLogToConsole(level);
    else bp::log::setupLogToFile(path, level);
}

/** a class to listen for UserQuitEvents and stop the runloop upon
 *  their arrival */
class QuitListener :  public ICommandHandlerListener
{
public:
  void onUserQuit() {
      std::cerr << "shutting down..." << std::endl;
      s_rl.stop();
  }
};

int
main(int argc, const char ** argv)
{
#ifndef WIN32
    (void) signal(SIGPIPE, SIG_IGN);
#endif

    s_rl.init();

    // parse command line arguments
    APTArgParse argParser("Service Manager command line client\n"
                          "usage: ServiceManagerTester");
    int x = argParser.parse(sizeof(g_args)/sizeof(g_args[0]), g_args,
                            argc, argv);
    if (x < 0) {
        std::cerr << argParser.error() << std::endl;
        exit(1);
    } else if (x != argc) {
        std::cerr << "extraneous command line arguments\n" << std::endl;
        exit(1);
    }

    setupLogging(argParser);
    
    // scope here so that all objects are cleaned up by end of main
    {
        // allocate an object that listens for quit events from the
        // command parser and stops the application
        QuitListener ql;

        // allocate a command parser
        CommandParserPtr parser(new CommandParser);

        // set up the quit listener to listen to the command parser
        parser->setListener(&ql);

        // allocate a class that handles commands
        bp::file::Path logfile(argParser.argument("logfile"));
        shared_ptr<CommandExecutor> chp(
            new CommandExecutor(argParser.argument("log"), logfile));
        
        // register functions from the command handler on the command parser
        parser->registerHandler(
            "available", chp,
            BP_COMMAND_HANDLER(CommandExecutor::available),
            0, 0,
            "List all installed services.");

        parser->registerHandler(
            "have", chp,
            BP_COMMAND_HANDLER(CommandExecutor::have),
            1, 3,
            "Report whether we have a service satisfying the specified "
            "requirement, containing name [version] [minversion].");

        parser->registerHandler(
            "describe", chp,
            BP_COMMAND_HANDLER(CommandExecutor::describe),
            1, 3,
            "Attain and output the description for a service which "
            "satisfies the specified requirement, containing name "
            "[version] [minversion].");

        parser->registerHandler(
            "summarize", chp,
            BP_COMMAND_HANDLER(CommandExecutor::summarize),
            1, 3,
            "Attain and output the summary for a service which satisfies the "
            "specified requirement, containing name [version] [minversion].");

        parser->registerHandler(
            "instantiate", chp,
            BP_COMMAND_HANDLER(CommandExecutor::instantiate),
            1, 2,
            "Allocate an instance of a specified service.  The service is "
            "specified by name and optionally by (partial) version.");

        parser->registerHandler(
            "destroy", chp,
            BP_COMMAND_HANDLER(CommandExecutor::destroy),
            1, 1,
            "Destroy an instance by ID.");

        parser->registerHandler(
            "execute", chp,
            BP_COMMAND_HANDLER(CommandExecutor::execute),
            2, 3,
            "Execute a function on an instance providing optional arguments. "
            "First arg is instance id, second function name, third is a json "
            "string containing arguments.");

        parser->registerHandler(
            "rescan", chp,
            BP_COMMAND_HANDLER(CommandExecutor::rescan),
            0, 0,
            "Rescan the disk for updated services.");

        parser->registerHandler(
            "purge", chp,
            BP_COMMAND_HANDLER(CommandExecutor::purge),
            2, 2,
            "Purge a service from disk, shutting it down and stopping any "
            "allocated instances");

        parser->start();
        s_rl.run();
        parser->stop();    
    }

    s_rl.shutdown();

    return 0;
}
