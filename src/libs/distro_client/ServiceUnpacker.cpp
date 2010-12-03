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

/*
 * ServiceUnpacker.cpp
 *
 * Created by Gordon Durand on 07/23/07.
 * Copyright 2007 Yahoo! Inc.  All rights reservered.
 */

#include <iostream>
#include <fstream>

#include "api/ServiceUnpacker.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bptime.h"
#include "BPUtils/bpprocess.h"
#include "platform_utils/ProductPaths.h"

using namespace std;
using namespace bp::file;
namespace bfs = boost::filesystem;

ServiceUnpacker::ServiceUnpacker(const bfs::path& pkgFile,
                                 const bfs::path& certFile)
    : Unpacker(pkgFile, certFile)
{
}


ServiceUnpacker::ServiceUnpacker(const std::vector<unsigned char>& buf,
                                 const bfs::path& certFile)
    : Unpacker(buf, certFile)
{
}


ServiceUnpacker::ServiceUnpacker(const bfs::path& dir,
                                 int)
    : Unpacker(bfs::path(), bfs::path()), m_dir(dir)
{
}


ServiceUnpacker::~ServiceUnpacker()
{
    if (!m_tmpDir.empty()) {
        safeRemove(m_tmpDir);
    }
}


bool
ServiceUnpacker::unpack(string& errMsg)
{
    m_tmpDir = getTempPath(getTempDirectory(), "ServiceUnpacker");
    return Unpacker::unpackTo(m_tmpDir, errMsg);
}


bool
ServiceUnpacker::install(string& errMsg)
{
    bfs::path dir = m_dir.empty() ? m_tmpDir : m_dir;
    BPLOG_DEBUG_STRM("install service from " << dir);
    errMsg.clear();
    bool rval = true;
    try {
        if (m_unpackError || !isDirectory(dir)) {
            stringstream ss;
            ss << "error, m_unpackError = " << m_unpackError
               << ", dir = " << dir;
            throw ss.str();
        }

        // install by invoking service installer
        bfs::path serviceInstaller = bp::paths::getServiceInstallerPath();
        if (serviceInstaller.empty()) {
            throw "Unable to get service installer path";
        }
        vector<string> args;
        args.push_back("-f");
        args.push_back("-v");
        args.push_back("-t");
        args.push_back("-log");
        args.push_back("debug");
        args.push_back("-logfile");
        args.push_back(bp::paths::getDaemonLogPath().string());
        args.push_back(dir.string());
        stringstream ss;
        ss << serviceInstaller;
        for (size_t i = 0; i < args.size(); i++) {
            ss << " " << args[i];
        }
        string cmdLine = ss.str();
        BPLOG_DEBUG_STRM("install service via '" << cmdLine << "'");
        bp::process::spawnStatus s;
        if (!bp::process::spawn(serviceInstaller, args, &s)) {
            throw string("Unable to spawn ") + cmdLine;
        }
        int exitCode = 0;
        (void) bp::process::wait(s, true, exitCode);
        if (exitCode != 0) {
            stringstream ss;
            ss << cmdLine << " failed, exitCode = " << exitCode;
            throw ss.str();
        }
    } catch(const string& s) {
        errMsg = s;
        rval = false;
    }
    return rval;
}


bool
ServiceUnpacker::unpackTo(const bfs::path& dir,
                          string& errMsg)
{
    return Unpacker::unpackTo(dir, errMsg);
}




