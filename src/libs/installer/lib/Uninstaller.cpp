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
#include "BPUtils/bpprocess.h"
#include "platform_utils/ProductPaths.h"


using namespace std;
using namespace bp::install;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;


namespace bp {
    namespace install {

        void
        Uninstaller::removeServices()
        {
            BPLOG_DEBUG("begin service uninstall");
            bfs::path serviceInstaller = bp::paths::getServiceInstallerPath();
            bfs::path serviceDir = bp::paths::getServiceDirectory();
            try {
                bfs::directory_iterator end;
                for (bfs::directory_iterator iter(serviceDir); iter != end; ++iter) {
                    bfs::path service(iter->path());
                    string serviceName = service.filename().string();
                    try {
                        bfs::directory_iterator vend;
                        for (bfs::directory_iterator viter(service); viter != vend; ++viter) {
                            string serviceVersion = viter->path().filename().string();
                            bp::SemanticVersion v;
                            if (v.parse(serviceVersion)) {
                                vector<string> args;
                                args.push_back("-u");
                                args.push_back("-v");
                                args.push_back("-t");
                                args.push_back("-log");
                                args.push_back(bp::log::levelToString(m_logLevel));
                                if (!m_logFile.empty()) {
                                    args.push_back("-logfile");
                                    args.push_back(m_logFile.string());
                                }
                                args.push_back(serviceName);
                                args.push_back(serviceVersion);
                                stringstream ss;
                                ss << serviceInstaller;
                                for (size_t i = 0; i < args.size(); i++) {
                                    ss << " " << args[i];
                                }
                                string cmdLine = ss.str();
                                BPLOG_DEBUG_STRM("purge service via '" << cmdLine << "'");
                                bp::process::spawnStatus s;
                                if (!bp::process::spawn(serviceInstaller, args, &s)) {
                                    BPLOG_DEBUG_STRM("Unable to spawn " << cmdLine);
                                    continue;
                                }
                                int exitCode = 0;
                                (void) bp::process::wait(s, true, exitCode);
                                if (exitCode != 0) {
                                    BPLOG_DEBUG_STRM(cmdLine << " failed, exitCode = " << exitCode);
                                }
                            }                            
                        }
                    } catch (const bfs::filesystem_error& e) {
                        BPLOG_WARN_STRM("unable to iterate thru " << service
                                        << ": " << e.what());
                    }
                }
            } catch (const bfs::filesystem_error& e) {
                BPLOG_WARN_STRM("unable to iterate thru " << serviceDir
                                << ": " << e.what());
            }
            BPLOG_DEBUG("end service uninstall");
        }

    }
}



