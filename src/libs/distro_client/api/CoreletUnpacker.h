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
 * CoreletUnpacker.h
 *
 * Created by Gordon Durand on 07/23/07.
 * Copyright 2007 Yahoo! Inc.  All rights reservered.
 */

#ifndef __CORELETUNPACKER_H__
#define __CORELETUNPACKER_H__

#include <string>
#include <map>
#include "Unpacker.h"

class CoreletUnpacker : virtual public Unpacker
{
 public:
    /** 
     * Create an instance to unpack a bpkg file
     */
    CoreletUnpacker(const bp::file::Path& pkgFile,
                    const bp::file::Path& destDir,
                    const std::string& name,
                    const std::string& version,
                    const bp::file::Path& certFile = bp::file::Path());
    
    /** 
     * Create an instance to unpack a buffer containing
     * a bpkg
     */
    CoreletUnpacker(const std::vector<unsigned char> & buf,
                    const bp::file::Path& destDir,
                    const std::string& name,
                    const std::string& version,
                    const bp::file::Path& certFile = bp::file::Path());
                
    virtual ~CoreletUnpacker(); 

    // unpacks, returns true on success
    bool unpack(std::string& errMsg);

    // installs to coreletdir            
    // On error, error msg returned in errMsg    
    bool install(std::string& errMsg);

 private:
    std::string m_name;
    std::string m_version;
};

#endif
