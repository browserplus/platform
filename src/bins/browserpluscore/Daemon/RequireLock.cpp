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

/**
 * RequireLock, a singleton mechanism which helps gaurantee that only
 * one session at a time is allowed to require corelets. (YIB-1770124)
 *
 * Created by Lloyd Hilaiel on Mon Feb 25th 2008.
 * Copyright (c) 2008-2009 Yahoo!, Inc. All rights reserved.
 */

#include "RequireLock.h"
#include <assert.h>
#include <list>
#include "BPUtils/BPLog.h"
#include "BPUtils/bpthreadhopper.h"

using namespace std;
using namespace std::tr1;


static bool s_initialized = false;
std::list<weak_ptr<RequireLock::ILockListener> > s_lockQueue;

void
RequireLock::initialize()
{
    assert(!s_initialized);
    s_initialized = true;    
    BPLOG_DEBUG("initialized");
}

void
RequireLock::shutdown()
{
    if (s_lockQueue.size() > 0) { 
        BPLOG_DEBUG_STRM("s_lockQueue.size() = " << s_lockQueue.size());
    }
    s_lockQueue.clear();
    s_initialized = false;    

    BPLOG_DEBUG("shutdown");
}

static void invokeCallbackFunction(void * context)
{
    weak_ptr<RequireLock::ILockListener> * lWeak =
        (weak_ptr<RequireLock::ILockListener> *) context;

    assert(lWeak != NULL);

    shared_ptr<RequireLock::ILockListener> l = (*lWeak).lock();

    if (l) {
        // call client callback
        l->gotRequireLock();
    } else {
        // uh oh, client's gone.  let's call the next one
        RequireLock::releaseLock(*lWeak);
    }

    delete lWeak;
}

static void
postLockReadyEvent(weak_ptr<RequireLock::ILockListener> listener)
{
    // we need a thread hopper here to guarantee that a callback is
    // delivered after function return
    static bool initialized = false;
    static bp::thread::Hopper hopper;

    if (!initialized) {
        hopper.initializeOnCurrentThread();
    }
    weak_ptr<RequireLock::ILockListener> * copy =
        new weak_ptr<RequireLock::ILockListener>(listener);
    
    hopper.invokeOnThread(invokeCallbackFunction, (void *) copy);
}

void
RequireLock::attainLock(weak_ptr<ILockListener> listener)
{
    s_lockQueue.push_back(listener);

    if (s_lockQueue.size() == 1) {
        postLockReadyEvent(s_lockQueue.front());
    }
}

void
RequireLock::releaseLock(weak_ptr<ILockListener> listener)
{
    // if the caller of release lock doesn't own the lock, then
    // ignore this call
    if (s_lockQueue.empty() ||
        (listener < s_lockQueue.front() || s_lockQueue.front() < listener))
    {
        return;
    }
    BPLOG_DEBUG("Require lock released");
    s_lockQueue.pop_front();

    if (s_lockQueue.size() > 0)
    {
        postLockReadyEvent(s_lockQueue.front());
    }
}

