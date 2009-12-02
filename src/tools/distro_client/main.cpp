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

// here's our implementation of handling commands
#include "CommandExecutor.h"

using namespace std;
using namespace std::tr1;


bp::runloop::RunLoop s_rl;

static APTArgDefinition g_args[] = {
    { "l", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "enable console logging, argument like \"info,ThrdLvlFuncMsg\""
    },
    { "s", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
      APT::NOT_INTEGER, APT::MAY_RECUR,
      "Specify secondary distribution servers.  May be repeated any number of"
      "times."
    }
};


static void 
setupLogging(const APTArgParse& argParser)
{
    // Clear out any existing appenders.
    bp::log::removeAllAppenders();
    if (argParser.argumentPresent("l")) {
        // Setup the system-wide minimum log level.
        std::string level = argParser.argument("l");
        bp::log::setupLogToConsole(level);
    }
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
    s_rl.init();

    // parse command line arguments
    APTArgParse argParser("Distribution web service query client\n"
                          "usage: DistQuery <dist url>");
    int x = argParser.parse(sizeof(g_args)/sizeof(g_args[0]), g_args,
                            argc, argv);
    if (x < 0) {
        std::cerr << argParser.error() << std::endl;
        exit(1);
    } else if (x != argc - 1) {
        std::cerr << "missing required url argument\n" << std::endl;
        exit(1);
    }

    setupLogging(argParser);
    
    // scope here so that all objects are cleaned up by end of main
    {
        std::list<std::string> distroServers;
        distroServers.push_back(std::string(argv[argc-1]));
        {
            std::vector<std::string> v = argParser.argumentValues("s");
            std::vector<std::string>::iterator it;
            for (it = v.begin(); it != v.end(); it++) {
                distroServers.push_back(*it);
            }
        }
            
        // allocate an object that listens for quit events from the
        // command parser and stops the application
        QuitListener ql;

        // allocate a command parser
        CommandParserPtr parser(new CommandParser);

        // set up the quit listener to listen to the command parser
        parser->setListener(&ql);

        // allocate a class that handles commands
        shared_ptr<CommandExecutor> chp(
            new CommandExecutor(distroServers));

        // register functions from the command handler on the command parser
        parser->registerHandler(
            "available", chp,
            BP_COMMAND_HANDLER(CommandExecutor::available),
            0, 1,
            "List all avaiilable corelets on the distribution server."
            "Accepts an option argument which is the platform.");

        parser->registerHandler(
            "platform", chp,
            BP_COMMAND_HANDLER(CommandExecutor::platform),
            0, 0,
            "Describe the platform upon which you are running.");

        parser->registerHandler(
            "permissions", chp,
            BP_COMMAND_HANDLER(CommandExecutor::permissions),
            0, 0,
            "Acquire and display the current permissions.");

        parser->registerHandler(
            "details", chp,
            BP_COMMAND_HANDLER(CommandExecutor::details),
            3, 3,
            "Acquire information about a specific corelet.  Three arguments "
            "corelet name, corelet version, and platform.");

        parser->registerHandler(
            "find", chp,
            BP_COMMAND_HANDLER(CommandExecutor::find),
            2, 4,
            "Determine if a corelet exists matching the specified parameters."
            "  Parameters are \n"
            "  1. platform (osx or win32)\n"
            "  2. corelet name\n"
            "  3. [version]\n"
            "  4. [minversion]");

        parser->registerHandler(
            "satisfy", chp,
            BP_COMMAND_HANDLER(CommandExecutor::satisfy),
            1, 2,
            "Given a list of require statements, generate a list of corelets "
            "that would satisfy these require statments and all require " 
            "statements expressed by dependent corelets.  The first argument "
            "is optional arguments.  The arguments option is a map with "
            "any of the keys 'platform' (value 'osx'|'win32'), "
            "'wantNewest' (true|false), or 'useInstalled (true|false) ."
            "The second is a json string, which is an array of maps. "
            "Each map expresses a require statement. "
            "sample usage: \n"
            "> satisfy "
            "'[{\"name\":\"FlickrAPI\",\"ver\":\"1\",\"minver\":\"1.0.3\"}]' "
            "> satisfy {\"platform\":\"osx\",\"useInstalled\":true}, {\"name\":\"FlickrAPI\"}]'");

        parser->registerHandler(
            "installed", chp,
            BP_COMMAND_HANDLER(CommandExecutor::installed),
            0, 0,
            "List installed corelets.");

        parser->registerHandler(
            "cached", chp,
            BP_COMMAND_HANDLER(CommandExecutor::cached),
            0, 0,
            "List corelets in cache.");

        parser->registerHandler(
            "isCached", chp,
            BP_COMMAND_HANDLER(CommandExecutor::isCached),
            2, 2,
            "Given a corelet name and version, check if it's available in "
            "the update cache.");

        parser->registerHandler(
            "updateCache", chp,
            BP_COMMAND_HANDLER(CommandExecutor::updateCache),
            1, 1,
            "Update cache, downloading corelet updates if available.  "
            "Argument is a JSON string of require statements, which will "
            "be considered to be the 'require history'");

        parser->registerHandler(
            "purgeCache", chp,
            BP_COMMAND_HANDLER(CommandExecutor::purgeCache),
            0, 0,
            "Purge all corelets from the corelet cache.");

        parser->registerHandler(
            "installFromCache", chp,
            BP_COMMAND_HANDLER(CommandExecutor::installFromCache),
            2, 2,
            "Install a corelet from the cache.  Two arguments required, "
            "corelet name and version respectively.");

        parser->registerHandler(
            "haveUpdates", chp,
            BP_COMMAND_HANDLER(CommandExecutor::haveUpdates),
            1, 1,
            "Report whether updates are available in cache which "
            "would better satisfy the requirements (expressed in json as "
            "argument to haveUpdates) than what is installed");

        parser->registerHandler(
            "strings", chp,
            BP_COMMAND_HANDLER(CommandExecutor::strings),
            3, 3,
            "Get localized names and descriptions for a list of corelets.  "
            "first argument is platform, second is locale, third is json "
            "list of maps of corelet 'name' and 'version'.");

        parser->registerHandler(
            "latestPlatformVersion", chp,
            BP_COMMAND_HANDLER(CommandExecutor::latestPlatformVersion),
            0, 0,
            "Query the distribution server for the latest available platform "
            "version.");

        parser->registerHandler(
            "downloadLatestPlatform", chp,
            BP_COMMAND_HANDLER(CommandExecutor::downloadLatestPlatform),
            0, 0,
            "Download the latest platform from the distribution server.");

        parser->start();
        s_rl.run();
        parser->stop();    
    }

    s_rl.shutdown();

    return 0;
}
