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
 * Unpacker.h
 *  A protected base class for subclasses which need to unpack and install bpkgs
 *
 * Created by Gordon Durand on 07/23/07.
 * Copyright 2007 Yahoo! Inc.  All rights reservered.
 */

#ifndef __UNPACKER_H__
#define __UNPACKER_H__

#include <vector>
#include <string>
#include <map>
#include "BPUtils/bpfile.h"

class Unpacker
{
 protected:
    /** 
     * Create an instance to unpack a bpkg package
     */
    Unpacker(const bp::file::Path& bpkgFile,
             const bp::file::Path& destDir,
             const bp::file::Path& certFile = bp::file::Path());
    
    /** 
     * Create an instance to unpack a bpkg package
     */
    Unpacker(const std::vector<unsigned char> & buf,
             const bp::file::Path& destDir,
             const bp::file::Path& certFile = bp::file::Path());
                
    // cleans tmpdir     
    virtual ~Unpacker(); 

    // unpacks to tmpdir, returns true on success
    // On error, error msg returned in errMsg    
    virtual bool unpack(std::string& errMsg);

    // installs            
    // On error, error msg returned in errMsg    
    virtual bool install(std::string& errMsg) = 0;

 protected:
    bp::file::Path m_bpkg;
    std::vector<unsigned char> m_buf;
    bp::file::Path m_tmpDir;
    bp::file::Path m_destDir;
    bp::file::Path m_certFile;
    bool m_unpackError;
};

#endif
