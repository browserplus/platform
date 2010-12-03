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
 * PlatformUnpacker.cpp
 *
 * Created by Gordon Durand on 03/18/08.
 * Copyright 2008 Yahoo! Inc.  All rights reservered.
 */

#include "api/PlatformUnpacker.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"

using namespace std;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;


PlatformUnpacker::PlatformUnpacker(const std::vector<unsigned char> & buf,
                                   const bfs::path& destDir,
                                   const std::string& version,
                                   const bfs::path& certFile)
    : Unpacker(buf, certFile), m_version(version), m_destDir(destDir)
{
}


PlatformUnpacker::PlatformUnpacker(const bfs::path& pkgFile,
                                   const bfs::path& destDir,
                                   const std::string& version,
                                   const bfs::path& certFile)
    : Unpacker(pkgFile, certFile), m_version(version), m_destDir(destDir)
{
}


PlatformUnpacker::~PlatformUnpacker()
{
    if (!m_tmpDir.empty()) {
        (void) bpf::safeRemove(m_tmpDir);
    }
}


bool
PlatformUnpacker::unpack(string& errMsg)
{
    m_tmpDir = bpf::getTempPath(bpf::getTempDirectory(), "PlatformUnpacker");
    return Unpacker::unpackTo(m_tmpDir, errMsg);
}


bool
PlatformUnpacker::install(string& errMsg)
{
    errMsg.clear();
    bfs::path platformDir = m_destDir / m_version;
    try {
        if (m_unpackError || !bpf::isDirectory(m_tmpDir)) {
            throw string("unpack error or no tmp dir");
        }
        
        // nuke existing platform update and move this one into place
        try {
            bfs::create_directories(m_destDir);
        } catch(const bfs::filesystem_error&) {
            string s("unable to create " + m_destDir.string());
            throw bp::error::lastErrorString(s.c_str());
        }
        if (!bpf::safeRemove(platformDir)) {
            string s("unable to delete existing " + platformDir.string());
            throw bp::error::lastErrorString(s.c_str());
        }
        if (!bpf::safeMove(m_tmpDir, platformDir)) {
            string s("unable to move " + m_tmpDir.string()
                     + " to " + platformDir.string());
            throw bp::error::lastErrorString(s.c_str());
        }
        BPLOG_INFO_STRM("Platform update " << m_version 
                        << " installed to " << m_destDir.string());
    } catch (const string& s) {
        BPLOG_ERROR_STRM("Error installing platform update "
                         << m_version << " to " << m_destDir
                         << ": " << s);
        bpf::safeRemove(platformDir);
        errMsg = s;
        return false;
    }
    return true;
}
