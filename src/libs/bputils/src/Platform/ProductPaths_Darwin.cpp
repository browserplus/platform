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
 *  ProductPaths_Darwin.cpp
 *
 *  Created by David Grigsby on 8/01/07.
 *  Based on code by Ashit Gandhi and Gordon Durand
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "api/ProductPaths.h"

#include <sys/stat.h>
#include <dirent.h>
#include <iostream>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"

using namespace std;
using namespace bp::file;
namespace bfs = boost::filesystem;

// Make a string from a CFStringRef
//
static string
stringRefToUTF8(CFStringRef cfStr)
{
    string rval;
    CFIndex cfLen = CFStringGetLength(cfStr);

    if (cfLen > 0) {
        char stackBuffer[2048], *dynamicBuf = NULL;
        char * buf = stackBuffer;
        if ((size_t) (cfLen*4) >= sizeof(stackBuffer)) {
            buf = (char*) malloc(cfLen*4 + 1);
            buf = dynamicBuf;
        }
        CFStringGetCString(cfStr, buf, cfLen*4 + 1,
                           kCFStringEncodingUTF8);
        rval.append(buf);
        if (dynamicBuf) free(dynamicBuf);
    }

    return rval;
}


Path
bp::paths::getProductTopDirectory()
{
    // Get application support dir
    FSRef fref;
    OSErr err = FSFindFolder(kUserDomain, kApplicationSupportFolderType,
                             kCreateFolder, &fref);
    if (err != noErr) {
        return string();
    }
    CFURLRef tmpUrl = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &fref);
    CFStringRef ctmpDir = CFURLCopyFileSystemPath(tmpUrl, kCFURLPOSIXPathStyle);
    Path productDir(stringRefToUTF8(ctmpDir));
    CFRelease(ctmpDir);
    CFRelease(tmpUrl);
    if (productDir.empty()) {
        BP_THROW_FATAL("Unable to get product top directory");
    }

    // append Yahoo!/BrowserPlus
    productDir /= getCompanyName();
    productDir /= getProductName();
    try {
        bfs::create_directories(productDir);
    } catch(const tFileSystemError&) {
        BP_THROW_FATAL("Unable to create " + productDir.utf8());
    }
    return productDir;
}


Path
bp::paths::getPluginWritableDirectory(int major,
                                      int minor,
                                      int micro)
{
    // no sandbox restrictions on mac (yet)
    return getProductDirectory(major, minor, micro);
}


string 
bp::paths::getIPCName()
{
    Path p = getProductDirectory() / "bp.pipe";
    return p.utf8();
}


string 
bp::paths::getEphemeralIPCName()
{
    Path p = getTempPath(getTempDirectory(), "BPIPC");
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
    FSRef fref;
    OSErr err = FSFindFolder(kUserDomain, kInternetPlugInFolderType,
                             kDontCreateFolder, &fref);
    if (err != noErr) {
        return rval;
    }
    CFURLRef tmpUrl = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &fref);
    CFStringRef ctmpDir = CFURLCopyFileSystemPath(tmpUrl, kCFURLPOSIXPathStyle);
    Path pluginDir = stringRefToUTF8(ctmpDir);
    CFRelease(ctmpDir);
    CFRelease(tmpUrl);
    
    // append BrowserPlus_version.plugin
    if (major == -1 && minor == -1 && micro == -1) {
        // cmake defines these
        major = BP_VERSION_MAJOR;
        minor = BP_VERSION_MINOR;
        micro = BP_VERSION_MICRO; 
    }
    pluginDir /= "BrowserPlus_" + versionString(major, minor, micro) + ".plugin";
    rval.push_back(pluginDir);
    return rval;
}


Path
bp::file::getTempDirectory()
{
    Path tempDir;
    FSRef fref;
    OSErr err = FSFindFolder(kUserDomain, kTemporaryFolderType, 
                             kCreateFolder, &fref);
    if (err == noErr) {
        CFURLRef tmpUrl = CFURLCreateFromFSRef(kCFAllocatorSystemDefault,
                                               &fref);
        if (tmpUrl != NULL) {
            CFStringRef ctmpDir = CFURLCopyFileSystemPath(tmpUrl,
                                                          kCFURLPOSIXPathStyle);
            tempDir = stringRefToUTF8(ctmpDir);
            CFRelease(ctmpDir);
            CFRelease(tmpUrl);
        }
        else 
        {
            BP_THROW_FATAL("Can't get temp dir");
        }
    }

    return tempDir;
}
