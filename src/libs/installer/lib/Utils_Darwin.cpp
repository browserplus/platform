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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include "api/Utils.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpfile.h"

using namespace std;
using namespace bp::error;
using namespace bp::install;

// Make a std::string from a CFStringRef
//
static string
stringRefToUTF8(CFStringRef cfStr)
{
    std::string rval;
    CFIndex cfLen = CFStringGetLength(cfStr);
    
    if (cfLen > 0) {
        char stackBuffer[2048], *dynamicBuf = NULL;
        char * buf = stackBuffer;
        if ((size_t) (cfLen*4) >= sizeof(stackBuffer)) {
            dynamicBuf = (char*) malloc(cfLen*4 + 1);
            buf = dynamicBuf;
        }
        CFStringGetCString(cfStr, buf, cfLen*4 + 1,
                           kCFStringEncodingUTF8);
        rval.append(buf);
        if (dynamicBuf) free(dynamicBuf);
    }
    
    return rval;
}


// Get a path via FSFindFolder
//
boost::filesystem::path
bp::install::utils::getFolderPath(int selector)
{
    boost::filesystem::path rval;
    FSRef fref;
    OSErr err = FSFindFolder(kUserDomain, selector, kCreateFolder, &fref);
    if (err != noErr) {
        BP_THROW(lastErrorString("unable to get folder path for selector " + selector));
    }
    CFURLRef tmpUrl = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &fref);
    CFStringRef ctmpDir = CFURLCopyFileSystemPath(tmpUrl, kCFURLPOSIXPathStyle);
    rval = stringRefToUTF8(ctmpDir);
    CFRelease(ctmpDir);
    CFRelease(tmpUrl);
    BPLOG_DEBUG_STRM("getFolderPath(" << selector << ") returns " << rval);
    return rval;
}

