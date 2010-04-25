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
#include "BPUtils/bpexitcodes.h"
#include "BPUtils/bpexitcodes.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/bpstrutil.h"
#include "BPInstaller/BPInstaller.h"

#import <Cocoa/Cocoa.h>

using namespace std;
using namespace bp::file;
using namespace bp::localization;

int
main(int argc, const char** argv)
{
    try {
        // debug logging on be default
        Path logFile = getTempDirectory() / "BrowserPlusUninstaller.log";
        string logLevel = "debug";

        bool quiet = false;
        vector<string> args;
        for (int i = 1; i < argc; i++) {
            if (!strncmp(argv[i], "-quiet", 6)) {
                quiet = true;
                continue;
            }

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

        bp::log::Level bpLogLevel = bp::log::levelFromString(logLevel);
        
        if (!logLevel.empty()) {
            if (logFile.empty()) {
                bp::log::setupLogToConsole(bpLogLevel);
            } else {
                bp::log::setupLogToFile(logFile, bpLogLevel,
                                        bp::log::kSizeRollover);
            }
        }

        // gunk we may need for alert
        NSApplication* application = nil;
        NSAutoreleasePool* pool = nil;

        // perhaps prompt user for confirmation
        string locale, prompt, done, yes, no, ok;
        if (!quiet) {
            application = [NSApplication sharedApplication];
            pool = [[NSAutoreleasePool alloc] init];

            // Get the strings we need now, in case we need them after deleting
            // resource files.
            const string locale = getUsersLocale();
            getLocalizedString("uninstallPrompt", locale, prompt);
            getLocalizedString("uninstallNotification", locale, done);
            getLocalizedString("yes", locale, yes);
            getLocalizedString("no", locale, no);
            getLocalizedString("ok", locale, ok);

            // build the alert
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setAlertStyle: NSCriticalAlertStyle];
            [alert setMessageText: [NSString stringWithUTF8String: prompt.c_str()]];
            [alert addButtonWithTitle: [NSString stringWithUTF8String: yes.c_str()]];
            [alert addButtonWithTitle: [NSString stringWithUTF8String: no.c_str()]];

            // show the alert, making sure that it's topmost
            BPLOG_INFO( "Displaying uninstall prompt." );
            [[alert window] setFloatingPanel: YES];
            int ret = [alert runModal];
            [alert release];
            if (ret == NSAlertSecondButtonReturn) {
                // user canceled
                BPLOG_INFO( "User cancelled uninstall" );
                return bp::exit::kUserCancel;
            }
        }

        // Actually do the uninstall
        bp::install::Uninstaller unins;
        unins.run();

        // perhaps let user know we're done
        if (!quiet) {
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setAlertStyle: NSInformationalAlertStyle];
            [alert setMessageText: [NSString stringWithUTF8String: done.c_str()]];
            [alert addButtonWithTitle: [NSString stringWithUTF8String: ok.c_str()]];
            [[alert window] setFloatingPanel: YES];
	    BPLOG_INFO( "Displaying uninstall notification." );
            (void) [alert runModal];
            [alert release];
            [pool release];
        }

    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR_STRM("Uninstall failed: " << e.what());
    } catch (const bp::error::FatalException& e) {
        BPLOG_ERROR_STRM("Uninstall failed: " << e.what());
    }
}
