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

bfs::path
bp::paths::getProductTopDirectory()
{
    bfs::path prodDir = getenv("HOME");
    if (prodDir.empty()) {
        BP_THROW_FATAL("unable to get $HOME");
    }
    prodDir /= bfs::path("." + getCompanyName()) / getProductName();
    return prodDir;
}


bfs::path
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
    bfs::path p =  getProductDirectory() / "bp.pipe";
    return p.string();
}


string 
bp::paths::getEphemeralIPCName()
{
    bfs::path p = getTempPath(getTempDirectory(), "BPIPC");
    return p.string();
}


string 
bp::paths::getIPCLockName(int major,
                          int minor,
                          int micro)
{
    return utf8(getDaemonPath(major, minor, micro));
}


vector<bfs::path> 
bp::paths::getPluginPaths(int major,
                          int minor,
                          int micro)
{
    vector<bfs::path> rval;
    // TODO: implement
    return rval;
}


