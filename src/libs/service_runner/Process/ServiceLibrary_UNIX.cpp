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

/**
 * ServiceLibrary unix specific implementations.
 */

#include "ServiceLibrary.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpfile.h"

#include <dlfcn.h>

using namespace ServiceRunner;

void * 
ServiceLibrary::dlopenNP(const boost::filesystem::path & path)
{
    int flag = RTLD_NOW | RTLD_LOCAL;
#ifdef RTLD_DEEPBIND
    flag |= RTLD_DEEPBIND;
#endif
    void * hand = dlopen(path.c_str(), flag);
    // if dlopen fails, lets' try to log some helpful hints for potentially
    // befuddled service engineers.
    if (hand == NULL) {
        const char * errorString = dlerror();
        if (errorString) {
            BPLOG_ERROR_STRM("couldn't dlopen '" << path << "': "
                             << errorString);
        }
    }
    return hand;
}

void
ServiceLibrary::dlcloseNP(void * handle)
{
    dlclose(handle);
}

void *
ServiceLibrary::dlsymNP(void * handle, const char * sym)
{
    return dlsym(handle, sym);
}
