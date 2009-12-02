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


/**
 * The "ProcessLock" mechanism can ensure that multiple instances of a
 * given process are not allowed to run, as well as providing a
 * mechanism to know when the daemon has exited (used by platform update);
 * On UNIX this is done using inter-process semaphores (see semget(2))
 * On Windows this is done by using inter-process Mutexes (see CreateMutex()).
 *
 * The mechanism is designed in a way that regardless of the way the
 * process in question exits, the lock will be released cleanly.  This
 * means that aside from cleaning up a couple resources, failure to
 * call releaseProcessLock() before process exit will not prevent the
 * startup of subsequent processes.
 */

#ifndef __PROCESSLOCK_H__
#define __PROCESSLOCK_H__

#include <string>

namespace bp {

typedef struct ProcessLock_t * ProcessLock;

/**
 *  Capture an exclusive process scoped lock.
 *    \param block - if true, block until lock aquired, else return immediately
 *    \param lockName - pathname of IPC lockname.  default value
 *                      results in current process's lock.
 *    \returns ProcessLock for acquired lock, or NULL on failure.
 */
ProcessLock acquireProcessLock(bool block,
                               const std::string& lockName = "");

/** Clean up some resources, should be done before shutting down */
void releaseProcessLock(ProcessLock hand);

};

#endif
