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
 * main, BrowserPlusCore main process logic, which determines the
 * mode in which BrowserPlusCore will run, and executes the correct
 * subsystem (Daemon, ServiceRunner, and Eventually Preference Panel
 * (on win32)). main() lives here.  By linking what could be multiple
 * distinct different functionality into the same binary we decrease
 * distribution size.
 *
 * First introduced by Lloyd Hilaiel on 2007/05/07
 * Copyright (c) 2007-2009 Yahoo!, Inc. All rights reserved.
 */

#include "Daemon/BPDaemon.h"
#include "platform_utils/ARGVConverter.h"
#include "platform_utils/bpexitcodes.h"
#include "platform_utils/ProductPaths.h"
#include "platform_utils/bpdebug.h"
#include "ServiceRunnerLib/ServiceRunnerLib.h"

#ifndef WIN32
#include <signal.h>
#endif

using namespace std;
using namespace std::tr1;


int main(int argc, const char ** argv)
{
    try {
#ifndef WIN32
        // on unix we should ignore SIGPIPE so we don't go down when an
        // IPC client goes down abruptly (YIB-2568266)
        signal(SIGPIPE, SIG_IGN);
#endif

        // handle non-ascii args on win32
        APT::ARGVConverter conv;
        conv.convert(argc, argv);

        // create needed directories
        bp::paths::createDirectories();

        // the presence of only 2 command line arguments and the -runService
        // flag causes us to run a service, rather than running the Daemon
        if (argc > 1 && !std::string(argv[1]).compare("-runService"))
        {
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
            if (!ServiceRunner::runServiceProcess(argc, argv)) {
                return bp::exit::kCantRunServiceProcess;
            }
        }
        else
        {
            // BPDaemon does the heavy lifting...
            shared_ptr<BPDaemon> daemon(
                new BPDaemon(argc, (const char **) argv));
            daemon->run();
        }

        return bp::exit::kOk;
    }
    catch (const std::exception& exc) {
        BP_REPORTCATCH(exc);
        throw;
    }
    catch (...) {
        BP_REPORTCATCH_UNKNOWN;
        throw;
    }
}

