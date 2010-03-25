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
using namespace bp::file;
namespace bfs = boost::filesystem;


PlatformUnpacker::PlatformUnpacker(const std::vector<unsigned char> & buf,
                                   const Path& destDir,
                                   const std::string& version,
                                   const Path& certFile)
: Unpacker(buf, destDir, certFile), m_version(version)
{
}


PlatformUnpacker::PlatformUnpacker(const Path& pkgFile,
                                   const Path& destDir,
                                   const std::string& version,
                                   const Path& certFile)
: Unpacker(pkgFile, destDir, certFile), m_version(version)
{
}


PlatformUnpacker::~PlatformUnpacker()
{
}


bool
PlatformUnpacker::unpack(string& errMsg)
{
    bool rval = Unpacker::unpack(errMsg);
    if (!rval) {
        BPLOG_ERROR_STRM("Error unpacking platform update "
                         << m_version << ": " << errMsg);
    }
    return rval;
}


bool
PlatformUnpacker::install(string& errMsg)
{
    errMsg.clear();
    Path platformDir = m_destDir / m_version;
    try {
        if (m_unpackError || !isDirectory(m_tmpDir)) {
            throw string("unpack error or no tmp dir");
        }
        
        // nuke existing platform update and move this one into place
        try {
            bfs::create_directories(m_destDir);
        } catch(const tFileSystemError&) {
            string s("unable to create " + m_destDir.externalUtf8());
            throw bp::error::lastErrorString(s.c_str());
        }
        if (!remove(platformDir)) {
            string s("unable to delete existing " + platformDir.externalUtf8());
            throw bp::error::lastErrorString(s.c_str());
        }
        if (!move(m_tmpDir, platformDir)) {
            string s("unable to move " + m_tmpDir.externalUtf8() 
                      + " to " + platformDir.externalUtf8());
            throw bp::error::lastErrorString(s.c_str());
        }
        BPLOG_INFO_STRM("Platform update " << m_version 
                        << " installed to " << m_destDir.externalUtf8());
    } catch (const string& s) {
        BPLOG_ERROR_STRM("Error installing platform update "
                         << m_version << " to " << m_destDir.externalUtf8()
                         << ": " << s);
        remove(platformDir);
        errMsg = s;
        return false;
    }
    return true;
}
