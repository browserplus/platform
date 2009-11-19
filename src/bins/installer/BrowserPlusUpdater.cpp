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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * BrowserPlusUpdater - a simple binary to manage the update process.
 *                      Takes two args
 *                          - a pathname to the directory containing 
 *                            the update "stuff".
 *                          - [optional] a pathname to the daemon lock 
 *                            file for the daemon which invoked us.
 */

#include <string>
#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ARGVConverter.h"
#include "BPInstaller/BPInstaller.h"

using namespace std;
using namespace bp::file;
using namespace bp::install;

int
main(int argc, const char** argv)
{
	// on win32, may have non-ascii chars in args.  deal with it
	APT::ARGVConverter conv;
    conv.convert(argc, argv);

    // set the appropriate locale for strings generated from the Installer
    string locale = bp::localization::getUsersLocale();
    Path stringsPath = canonicalPath(Path(argv[0])).parent_path() / "strings.json";
    Installer::setLocalizedStringsPath(stringsPath, locale);

    // setup logging
    Path logFile = getTempDirectory() / "BrowserPlusUpdater.log";
    (void) remove(logFile);
    bp::log::setupLogToFile(logFile,
                            bp::log::levelToString(bp::log::LEVEL_ALL),
                            true);        

    Path dir;
    switch(argc) {
    case 3:
        // we no longer need lockfile arg
        // fall thru
        BPLOG_DEBUG_STRM("ignoring lockfile = " << argv[2]);
    case 2:
        dir = Path(argv[1]);
        BPLOG_DEBUG_STRM("dir = " << argv[1]);
        break;
    default:
		BPLOG_ERROR("usage: BrowserPlusUpdater dir [lockfile]");
		cout << "usage: BrowserPlusUpdater dir [lockfile]" << endl; 
        return -1;
    }

    int rval = 0;
    try {
        // install out of dir
        Installer inst(dir, true);
        inst.run();
    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR(e.what());
        rval = -1;
    }
    exit(rval);
}
