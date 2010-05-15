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


#include "BPUtils/ARGVConverter.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPInstaller/BPInstaller.h"

using namespace std;
using namespace bp::file;

int
main(int argc, const char** argv)
{
    try {
        // debug logging on be default
        Path logFile = getTempDirectory() / "BrowserPlusUninstaller.log";
        string logLevel = "debug";

        vector<string> args;
        for (int i = 1; i < argc; i++) {
            // skip args starting with -psn which deliver the "Process Serial
            // Number" and are added by the OSX launcher
            if (!strncmp(argv[i], "-psn", 4)) continue;

            args = bp::strutil::split(argv[i], "=");
            if (!args[0].compare("-logfile")) {
                if (!args[1].compare("console")) {
                    logFile.clear();
                } else {
                    logFile = args[1];
                }
            } else if (!args[0].compare("-log")) {
                logLevel = args[1];
            }
        }
    
        if (!logLevel.empty()) {
            if (logFile.empty()) {
                bp::log::setupLogToConsole(logLevel);
            } else {
                bp::log::setupLogToFile(logFile, logLevel,
                                        bp::log::kSizeRollover);
            }
        }
        bp::install::Uninstaller unins;
        unins.run();
        return bp::exit::kOk;
    } catch (const std::exception& exc) {
        BP_REPORTCATCH(exc);
        return bp::exit::kCaughtException;
    } catch (...) {
        BP_REPORTCATCH_UNKNOWN;
        return bp::exit::kCaughtException;
    }
}
