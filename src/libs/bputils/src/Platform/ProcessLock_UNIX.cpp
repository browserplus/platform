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

#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "BPUtils/bperrorutil.h"
#include "BPUtils/ProductPaths.h"


#ifdef LINUX
// linux doesn't have semaphore access control?
#define SEM_A 0
#define SEM_R 0
#endif

struct bp::ProcessLock_t {
    int lck;
};

#define FILE_PERMS S_IRUSR
#define BPLUS_LOCK_NAME "BrowserPlus.lock"

#if 0
// maybe we will need to expose manual resetting of the IPC lock at
// some point.
static void bp::resetProcessLock()
{
    int x;
    std::string processLock = bp::paths::getIPCLockName();
    key_t semkey = ftok(processLock.c_str(), 0);
    int s = semget(semkey, 1, SEM_A | SEM_R);
    if (s < 0) return;
    x = semctl(s, 0, IPC_RMID);
    
}
#endif

bp::ProcessLock bp::acquireProcessLock(bool block,
                                       const std::string& lockName)
{
    key_t semkey = 0;

    // get the path which we'll convert into a semaphore "key"
    std::string processLock = lockName.empty() ? 
                                bp::paths::getIPCLockName() 
                                : lockName;
    if (processLock.empty()) return NULL;
    semkey = ftok(processLock.c_str(), 0);

    // attempt to create the semaphore
    int s = semget(semkey, 1, IPC_CREAT | IPC_EXCL | SEM_A | SEM_R);
    if (s >= 0) {
        // after creation, set initial value to 1
        struct sembuf sb = { 0, 1, 0 };        
        if (semop(s, &sb, 1) != 0) {
            std::cerr << "can't initialize semaphore value: " << errno << ": "
                      << bp::error::lastErrorString().c_str()
                      << std::endl;
            return NULL;
        }
    } else if (errno == EEXIST) {
        // creation failed, grab existing semaphore.
        // if already exists, we do not modify the value.
        s = semget(semkey, 1, SEM_A | SEM_R);
    }

    if (s < 0) {
        // should never happen 
        std::cerr << "Can't allocate semaphore: "
                  << bp::error::lastErrorString().c_str()
                  << std::endl;
        return NULL;
    }

    // now try to exclusively "lock" the semaphore, all atomic.  SEM_UNDO
    // causes the count to be bumped back to 1 at process exit
    struct sembuf sb = { 0, -1, SEM_UNDO };
    if (!block) {
        sb.sem_flg |= IPC_NOWAIT;
    }
    int x = semop(s, &sb, 1);
    if (x != 0) {
        if (block) {
            if (x != EIDRM) {
                // only allowed failure is semaphore set removed from system
                return NULL;
            }
        } else {
            // Failure case when another process is running.
            return NULL;
        }
    }

    ProcessLock pfhand = (ProcessLock) malloc(sizeof(struct ProcessLock_t));
    pfhand->lck = s;

    return pfhand;
}


void bp::releaseProcessLock(bp::ProcessLock hand)
{
	if (hand == NULL) return;
	free(hand);
    // the operating system will bump the semaphore back up to 1 because
    // of the SEM_UNDO flag
}
