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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include "api/Uninstaller.h"
#include "api/Utils.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"


using namespace std;
using namespace bp::install;
using namespace bp::file;
namespace bfs = boost::filesystem;


namespace bp {
    namespace install {

        Uninstaller::Uninstaller(const bp::file::Path& logFile,
                                 bp::log::Level logLevel)
        : m_logFile(logFile), m_logLevel(logLevel), m_error(false)
        {
            // empty
        }


        Uninstaller::~Uninstaller() 
        {
            // empty
        }


        void
        Uninstaller::run(bool)
        {
            BPLOG_DEBUG("begin uninstall");

            // Remove all services
            removeServices();

            // Remove all BrowserPlus plugins
            Path dir = utils::getFolderPath(kInternetPlugInFolderType);
            try {
                tDirIter end;
                for (tDirIter iter(dir); iter != end; ++iter) {
                    Path p(iter->path());
                    if (p.filename().find("BrowserPlus_") == 0) {
                        try {
                            BPLOG_DEBUG_STRM("remove " << p);
                            remove(p);
                        } catch(const tFileSystemError&) {
                            BPLOG_WARN_STRM("unable to remove " << p);
                        }
                    }
                }
            } catch (const tFileSystemError& e) {
                BPLOG_WARN_STRM("unable to iterate thru " << dir
                                << ": " << e.what());
            }

            // Remove preference panel
            dir = utils::getFolderPath(kPreferencePanesFolderType);
            Path path = dir / "BrowserPlusPrefs.prefPane";
            BPLOG_DEBUG_STRM("remove " << path);
            (void) remove(path);

            // Remove platform
            dir = utils::getFolderPath(kApplicationSupportFolderType);
            path = dir / "Yahoo!" / "BrowserPlus";
            BPLOG_DEBUG_STRM("remove " << path);
            (void) remove(path);
            removeDirIfEmpty(path.parent_path());

            // remove uninstaller
            dir = utils::getFolderPath(kApplicationsFolderType);
            path = dir / "Yahoo!" / "BrowserPlus";
            BPLOG_DEBUG_STRM("remove " << path);
            (void) remove(path);
            removeDirIfEmpty(path.parent_path());

            // remove temp dir
            Path tempDir = getTempDirectory();
            if (tempDir.filename() == "BrowserPlus") {
                (void) remove(tempDir);
            }

            // remove receipts from old packagemaker installer
            dir = utils::getFolderPath(kInstallerReceiptsFolderType);
            path = dir / "BrowserPlus.pkg";
            BPLOG_DEBUG_STRM("remove " << path);
            (void) remove(path);

            BPLOG_DEBUG("complete uninstall");
        }


        void
        Uninstaller::removeDirIfEmpty(const Path& dir)
        {
            if (dir.empty()) {
                try {
                    BPLOG_DEBUG_STRM("remove " << dir);
                    bfs::remove(dir);
                } catch(const tFileSystemError&) {
                    BPLOG_WARN_STRM("unable to remove empty dir " << dir);
                }
            }
        }

    }
}



