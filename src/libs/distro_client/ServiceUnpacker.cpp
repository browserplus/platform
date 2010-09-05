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
#include "platform_utils/ProductPaths.h"

using namespace std;
using namespace bp::file;

ServiceUnpacker::ServiceUnpacker(const Path& pkgFile,
                                 const Path& destDir,
                                 const std::string& name,
                                 const std::string& version,
                                 const Path& certFile)
: Unpacker(pkgFile, destDir, certFile),  m_name(name), m_version(version)
{
}


ServiceUnpacker::ServiceUnpacker(const std::vector<unsigned char> & buf,
                                 const Path& destDir,
                                 const std::string& name,
                                 const std::string& version,
                                 const Path& certFile)
: Unpacker(buf, destDir, certFile), m_name(name), m_version(version)
{
}


ServiceUnpacker::~ServiceUnpacker()
{
}


bool
ServiceUnpacker::unpack(string& errMsg)
{
    bool rval = Unpacker::unpack(errMsg);
    if (!rval) {
        BPTime now;
        ofstream log;
        if (openWritableStream(log, bp::paths::getServiceLogPath(), 
                               std::ios_base::app | std::ios::binary)) {
            log << now.asString() << ": Error unpacking service " << m_name << " " 
                << m_version << ": " << errMsg << endl;
            BPLOG_WARN_STRM("Error unpacking service " << m_name << " " 
                            << m_version << ": " << errMsg);
        }
    }
    return rval;
}


bool
ServiceUnpacker::install(string& errMsg)
{
    errMsg.clear();
    bool rval = true;
    try {
        if (m_unpackError || !isDirectory(m_tmpDir)) {
            stringstream ss;
            ss << "error, m_unpackError = " << m_unpackError
               << ", m_tmpDir = " << m_tmpDir;
            throw ss.str();
        }

        // nuke existing service and move this one into place
        Path serviceTopDir = m_destDir / m_name;
        try {
            boost::filesystem::create_directories(serviceTopDir);
        } catch(const tFileSystemError&) {
            throw string("unable to create directory " 
                         + serviceTopDir.externalUtf8());
        }
        Path serviceDir = serviceTopDir / m_version;
        bool rval = remove(serviceDir) && move(m_tmpDir, serviceDir);
        if (rval) {
            BPTime now;
            ofstream log;

            if (openWritableStream(log, bp::paths::getServiceLogPath(), 
                                   std::ios_base::app | std::ios::binary)) {
                log << now.asString() << ": Installed " << m_name 
                    << " " << m_version << endl;
            } 
        } else {
            BPLOG_INFO_STRM("unable to delete " << serviceDir << " or move "
                            << m_tmpDir << " to " << serviceDir);
            remove(serviceDir);
        }
    } catch(const string& s) {
        errMsg = s;
        rval = false;
    }
    return rval;
}
