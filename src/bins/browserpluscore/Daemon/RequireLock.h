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
 * one session at a time is allowed to require services.
 * (YIB-1770124)
 *
 * Created by Lloyd Hilaiel on Mon Feb 25th 2008.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __REQUIRELOCK_H__
#define __REQUIRELOCK_H__

#include "BPUtils/bptr1.h"


namespace RequireLock
{
    /**
     * initialize the singleton RequireLock mechanism.  Must be called
     * once per process before it is used.  Should also be called from
     * the correct thread (main runloop thread).  This module is
     * not threadsafe and there are assertions to ensure correct usage.
     */
    void initialize();

    /**
     * shutdown the require lock mechanism.  Should be called after all
     * clients are torn down.
     */
    void shutdown();

    /**
     * a listener for events pertaining to acquisition of require lock
     */
    class ILockListener 
    {
      public:
        virtual ~ILockListener() { }
        virtual void gotRequireLock() = 0;
    };

    /**
     * Attempt to attain the exclusive require lock.  
     */
    void attainLock(std::tr1::weak_ptr<ILockListener> listener);

    /**
     * Release the exclusive require lock.  This must be called once
     * per recieved LockAttainedEvent, lest all requires are infinitily
     * blocked.  The listener argument is used by internal checks which
     * assert correct interface usage
     */
    void releaseLock(std::tr1::weak_ptr<ILockListener> listener);
};

#endif
