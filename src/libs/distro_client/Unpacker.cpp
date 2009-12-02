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
 * Unpacker.cpp
 *
 * Created by Gordon Durand on 07/23/07.
 * Copyright 2007 Yahoo! Inc.  All rights reservered.
 */

#include "api/Unpacker.h"
#include <sstream>
#include <vector>
#include "BPUtils/bpfile.h"
#include "ArchiveLib/ArchiveLib.h"


using namespace std;
using namespace bp::file;


Unpacker::Unpacker(const Path& bpkgFile,
                   const Path& destDir,
                   const Path& certFile)
: m_bpkg(bpkgFile), m_destDir(destDir),
  m_certFile(certFile), m_unpackError(false)
{
}


Unpacker::Unpacker(const std::vector<unsigned char> & buf,
                   const Path& destDir, 
                   const Path& certFile)
: m_buf(buf), m_destDir(destDir),
  m_certFile(certFile), m_unpackError(false)
{
}


Unpacker::~Unpacker()
{
    remove(m_tmpDir);
}


bool
Unpacker::unpack(string& errMsg)
{
    errMsg.clear();
    m_tmpDir = getTempPath(getTempDirectory(), "Unpacker");
    try {
        BPTime ts;
        if (m_buf.size() > 0) {
            // get an input iterator from the vector

            // TODO: can we directly construct an istream from
            // a vector (rather than copying into a string and
            // using a stringstream)? 
            std::string s;
            s.append((const char *) &(m_buf[0]), m_buf.size());
            std::stringstream ss(s, ios_base::in);
            if (!bp::pkg::unpackToDirectory(ss, m_tmpDir, ts, errMsg,
                                            m_certFile))
            {
                string s("unable to unpack package: " + errMsg);
                throw runtime_error(s);
            }
        } else {
            if (!bp::pkg::unpackToDirectory(m_bpkg, m_tmpDir, ts, errMsg,
                                            m_certFile))
            {
                string s("unable to unpack package: " + errMsg);
                throw runtime_error(s);
            }
        }
    } catch(const runtime_error& e) {  
        m_unpackError = true;
        errMsg = e.what();
    }
    return errMsg.empty();
}

