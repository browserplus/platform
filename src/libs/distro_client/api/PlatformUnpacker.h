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
 * PlatformUnpacker.h
 *
 * Created by Gordon Durand on 03/18/08.
 * Copyright 2008 Yahoo! Inc.  All rights reservered.
 */

#ifndef __PLATFORMUNPACKER_H__
#define __PLATFORMUNPACKER_H__

#include <string>
#include <map>
#include "Unpacker.h"

class PlatformUnpacker : virtual public Unpacker
{
 public:
    /** 
     * Create an instance to unpack a buffer containing
     * a platform update bpkg.
     */
    PlatformUnpacker(const std::vector<unsigned char> & buf,
                     const boost::filesystem::path& destDir,
                     const std::string& version,
                     const boost::filesystem::path& certFile = boost::filesystem::path());
                     
    /** 
     * Create an instance to unpack a file containing
     * a platform update bpkg.
     */
    PlatformUnpacker(const boost::filesystem::path& pkgFile,
                     const boost::filesystem::path& destDir,
                     const std::string& version,
                     const boost::filesystem::path& certFile = boost::filesystem::path());
                     
    virtual ~PlatformUnpacker(); 

    // unpacks, returns true on success
    // On error, error msg returned in errMsg    
    bool unpack(std::string& errMsg);

    // after an unpack(), installs platform update to cache            
    // On error, error msg returned in errMsg    
    bool install(std::string& errMsg);

 private:
    std::string m_version;
    boost::filesystem::path m_destDir;
    boost::filesystem::path m_tmpDir;
};

#endif
