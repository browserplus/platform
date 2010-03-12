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

#include "ProcessLock.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <windows.h>
#include "BPUtils/bpstrutil.h"
#include "BPUtils/ProductPaths.h"

struct bp::ProcessLock_t {
    HANDLE lock;
};

bp::ProcessLock bp::acquireProcessLock(bool block,
                                       const std::string& lockName)
{
    HANDLE lock;
    DWORD lastError;
    
    std::string name = lockName.empty() 
                        ? bp::paths::getIPCLockName() 
                        : lockName;
    std::wstring wname = bp::strutil::utf8ToWide(name);
    lock = CreateMutexW(NULL, TRUE, wname.c_str());
    lastError = GetLastError();
    if (lastError == ERROR_ALREADY_EXISTS) {
        // lock exists, should we wait?
        if (block) {
            DWORD waitResult = WaitForSingleObject(lock, INFINITE);
            if (waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED) {
                CloseHandle(lock);
                return NULL;
            }
        } else {
            CloseHandle(lock);
            return NULL;
        }
    }
    ProcessLock pfhand = (ProcessLock) malloc(sizeof(struct ProcessLock_t));
    pfhand->lock = lock;

    return pfhand;
}

void bp::releaseProcessLock(bp::ProcessLock hand)
{
	if (hand == NULL) return;
    CloseHandle(hand->lock);
    hand->lock = NULL;
	free(hand);
}
