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
 * BPProtoStressTest - a client which links BPProtocol and stresses the
 *                     creation and destruction of connections to the
 *                     browserplus daemon
 */

#include <iostream>
#include "BPProtocol/BPProtocol.h"
#include "BPUtils/BPLog.h"
#include "platform_utils/APTArgParse.h"
#include "platform_utils/bpconfig.h"
#include "stresstest.h"

static void 
setupLogging(const APTArgParse& argParser)
{
    bp::log::removeAllAppenders();
    if (argParser.argumentPresent("l")) {
        bp::log::Level level = bp::log::levelFromString(argParser.argument("l"));
        bp::log::setLogLevel(level);
        bp::log::setupLogToConsole(level);
    }
}


// Parses the command-line using an APTArgParse object.
// Returns true on success
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
        },
        { "s", APT::TAKES_ARG, "10", APT::REQUIRED,
        APT::IS_INTEGER, APT::MAY_NOT_RECUR,
        "the number of simultaneous connections to maintain."
        },
        { "d", APT::TAKES_ARG, "10", APT::REQUIRED,
        APT::IS_INTEGER, APT::MAY_NOT_RECUR,
        "the duration of the test in seconds."
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
    APTArgParse argParser("BPProto Stress Test Client");
    if (!processCommandLine(argParser, argc, argv))
    {
        // Exit on invalid cmd line.
        return 1;
    }

    // establish constants
    unsigned int simulConns = argParser.argumentAsInteger("s");
    unsigned int duration = argParser.argumentAsInteger("d");

    setupLogging(argParser);

    // init protocol library
    BPInitialize();

    // ready to go!  now run the test
    runTest(simulConns, duration);

    // stop protocol library
    BPShutdown();

    return 0;
}
