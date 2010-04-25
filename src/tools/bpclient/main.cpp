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
#include "BPProtocol/BPProtocol.h"
#include "BPUtils/APTArgParse.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "ConsoleLib/ConsoleLib.h"

// here's our implementation of handling commands
#include "CommandExecutor.h"

using namespace std;
using namespace std::tr1;


bp::runloop::RunLoop s_rl;

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

static void 
setupLogging(const APTArgParse& argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();
    if (argParser.argumentPresent("l")) {
        bp::log::Level level = bp::log::levelFromString(argParser.argument("l"));
        bp::log::setLogLevel(level);
        bp::log::setupLogToConsole(level);
    }
}

// Parses the command-line using an APTArgParse object.
// Returns true on success.
static bool
processCommandLine(APTArgParse& argParser, int argc, const char ** argv)
{
    // Note.  All args are marked MAY_RECUR since the argv that we
    // are given is a combination of command line args and 
    // config file args
    APTArgDefinition args[] =
    {
        { "l", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_RECUR,
        "enable console logging, argument is level (info, debug, etc.)" 
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

int
main(int argc, const char ** argv)
{
    s_rl.init();

    APTArgParse argParser("BPCommandLineClient");
    if (!processCommandLine(argParser, argc, argv)) 
    {
        // Exit on invalid cmd line.
        return 1;
    }

    setupLogging(argParser);

    // scope here so that all objects are cleaned up by end of main
    {
        // init protocol library
        BPInitialize();

        // allocate an object that listens for quit events from the
        // command parser and stops the application
        QuitListener ql;

        // allocate a command parser
        CommandParserPtr parser(new CommandParser);

        // set up the quit listener to listen to the command parser
        parser->setListener(&ql);

        // allocate a class that handles commands
        shared_ptr<CommandExecutor> chp(new CommandExecutor);

        // register functions from the command handler on the command parser
    
        // register a handler for the "connect" command 
        parser->registerHandler(
            "connect", chp,
            BP_COMMAND_HANDLER(CommandExecutor::connect),
            0, 0, "connect to BPCore");

        // execute a request
        parser->registerHandler(
            "exec", chp,
            BP_COMMAND_HANDLER(CommandExecutor::execute),
            3, 4,
            "execute a service function.  takes 3 or 4 arguments:\n"
            "  1. corelet to call\n"
            "  2. corelet version\n"
            "  3. function to call on corelet\n"
            "  4. JSON representation of arguments to function\n\n");

        // describe the interface of a corelet.
        parser->registerHandler(
            std::string("describe"), chp,
            BP_COMMAND_HANDLER(CommandExecutor::describe),
            2, 2,
            "describe the interface of a corelet");

        // enumerate the currently installed corelets.
        parser->registerHandler(
            "installed", chp,
            BP_COMMAND_HANDLER(CommandExecutor::enumerate),
            0, 0,
            "display the currently installed corelets.");

        // require a corelet
        parser->registerHandler(
            "require", chp,
            BP_COMMAND_HANDLER(CommandExecutor::require),
            1, 1,
            "require corelets by specifying a list of maps, each element contains: \n"
            "  service - the name of the corelet\n"
            "  version - a version pattern that the corelet must patch\n"
            "  minversion - a version pattern that the corelet must exceed\n");

        // get some state
        parser->registerHandler(
            "state", chp,
            BP_COMMAND_HANDLER(CommandExecutor::getState),
            1, 1,
            "get some state, try 'state help' to see available state "
            "strings\n");

        parser->registerHandler(
            "setstate", chp,
            BP_COMMAND_HANDLER(CommandExecutor::setState),
            2, 2,
            "get some state, try 'setstate help' to see available state "
            "strings\n");

        parser->start();
        s_rl.run();
        parser->stop();    
    }

    // stop protocol library
    BPShutdown();

    s_rl.shutdown();

    return 0;
}
