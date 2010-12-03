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
        Uninstaller::Uninstaller(const bfs::path& logFile,
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

            // XXX

            BPLOG_DEBUG("complete uninstall");
        }


        void
        Uninstaller::removeDirIfEmpty(const bfs::path& dir)
        {
            if (dir.empty()) {
                try {
                    BPLOG_DEBUG_STRM("remove " << dir);
                    bfs::remove(dir);
                } catch(const bfs::filesystem_error&) {
                    BPLOG_WARN_STRM("unable to remove empty dir " << dir);
                }
            }
        }

    }
}



