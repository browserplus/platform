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

#include "bpthreadhopper.h"
#include "bpsync.h"
#include "Carbon/Carbon.h"
#include "BPUtils/BPLog.h"


#include <iostream>

struct MacThreadHopperData {
    CFRunLoopSourceRef rlSource;
    CFRunLoopRef rl;
    bp::sync::Mutex mtx;
    std::vector<std::pair<bp::thread::Hopper::InvokeFuncPtr, void *> > work;
};

static void
invokeClientCallbacks(MacThreadHopperData * mthd)
{
    std::vector<std::pair<bp::thread::Hopper::InvokeFuncPtr, void *> > work;

    mthd->mtx.lock();
    work = mthd->work;
    mthd->work.clear();
    mthd->mtx.unlock();

    // guarantee we don't use this anymore
    mthd = NULL;

    // now outside of our lock, relying only on local data, invoke all of
    // our client callbacks.  This allows one of these invoked callbacks
    // to delete us without ill effect.
    while (work.size() > 0) {
		bp::thread::Hopper::InvokeFuncPtr invokeFunc = work.front().first;
        void * context = work.front().second;
        work.erase(work.begin());
        invokeFunc(context);
    }
}

bool
bp::thread::Hopper::initializeOnCurrentThread()
{
    // don't double init, please
    if (m_osSpecific != NULL) return false;

    CFRunLoopRef rl = CFRunLoopGetCurrent();
    if (rl == NULL) return false;

    MacThreadHopperData * mthd = new MacThreadHopperData;

    // initialization is really nothing more than grabbing the
    // runloop associated with the current thread of control
    m_osSpecific = (void *) mthd;

    // create a source so the thing runs
    CFRunLoopSourceContext cx;
    memset((void *) &cx, 0, sizeof(cx));
    cx.info = (void *) mthd;
    cx.perform = (void (*)(void *)) ::invokeClientCallbacks;
    mthd->rlSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &cx);
    BPASSERT(mthd->rlSource != NULL);
    mthd->rl = rl;
    CFRunLoopAddSource(rl, mthd->rlSource, kCFRunLoopCommonModes);

    return true;
}

bool
bp::thread::Hopper::invokeOnThread(InvokeFuncPtr invokeFunc, void * context)
{
    MacThreadHopperData * mthd = (MacThreadHopperData *) m_osSpecific;

    mthd->mtx.lock();
    
    CFRunLoopSourceSignal(mthd->rlSource);
    CFRunLoopWakeUp(mthd->rl);

    mthd->work.push_back(std::pair<InvokeFuncPtr, void *>(invokeFunc, context));

    mthd->mtx.unlock();

    return true;
}

bp::thread::Hopper::~Hopper()
{
    MacThreadHopperData * mthd = (MacThreadHopperData *) m_osSpecific;
    mthd->mtx.lock();
    CFRunLoopSourceInvalidate(mthd->rlSource);
    CFRelease(mthd->rlSource);
    mthd->mtx.unlock();
    delete mthd;
    m_osSpecific = NULL;
}

void
bp::thread::Hopper::processOutstandingRequests()
{
    MacThreadHopperData * mthd = (MacThreadHopperData *) m_osSpecific;

    mthd->mtx.lock();

    while (mthd->work.size() > 0) {
        bp::thread::Hopper::InvokeFuncPtr invokeFunc = mthd->work.front().first;
        void * context = mthd->work.front().second;
        mthd->work.erase(mthd->work.begin());
        mthd->mtx.unlock();
        // call client callback outside of lock.
        invokeFunc(context);
        mthd->mtx.lock();
    }

    mthd->mtx.unlock();
}
