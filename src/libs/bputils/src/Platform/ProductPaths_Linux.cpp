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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  bppaths_Darwin.cpp
 *
 *  Created by David Grigsby on 8/01/07.
 *  Based on code by Ashit Gandhi and Gordon Durand
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "api/ProductPaths.h"

#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"

#include <sys/stat.h>
#include <dirent.h>
#include <iostream>

using namespace std;
using namespace bp::file;
namespace bfs = boost::filesystem;

Path
bp::paths::getProductTopDirectory()
{
    Path prodDir = getenv("HOME");
    if (prodDir.empty()) {
        BP_THROW_FATAL("unable to get $HOME");
    }
    prodDir /= Path("." + getCompanyName()) / getProductName();
    try {
        bfs::create_directories(prodDir);
    } catch(const tFileSystemError&) {
        BP_THROW_FATAL("unable to create " + prodDir.utf8());
    }
    return prodDir;
}


Path
bp::paths::getPluginWritableDirectory(int major,
                                      int minor,
                                      int micro)
{
    // no sandbox restrictions (yet)
    return getProductDirectory(major, minor, micro);
}


string 
bp::paths::getIPCName()
{
    Path p =  getProductDirectory() / "bp.pipe";
    return p.utf8();
}


string 
bp::paths::getIPCLockName(int major,
                          int minor,
                          int micro)
{
    return getDaemonPath(major, minor, micro).utf8();
}


vector<Path> 
bp::paths::getPluginPaths(int major,
                          int minor,
                          int micro)
{
    vector<Path> rval;
    // TODO: implement
    return rval;
}


