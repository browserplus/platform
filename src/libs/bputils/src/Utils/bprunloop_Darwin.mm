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

#include "api/bprunloop.h"
#include "api/bperrorutil.h"

#include <stdlib.h>
#include <iostream>

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

struct AppleRunLoopData 
{
    // queue of events
    NSAutoreleasePool * pool;
    CFRunLoopSourceRef source;
    CFRunLoopSourceContext cx;
    CFRunLoopRef runLoop;
    // queue of events
    std::vector<bp::runloop::Event> * m_workQueue;
    // pointer to lock used by runloop object
    bp::sync::Mutex * lock;
    // who to deliver the event to
    bp::runloop::eventCallBack onEventFunc;
    void * onEventCookie;
};

static void
appleGotEventCallback(void * info)
{
	if (info == NULL) return;

    AppleRunLoopData * arld = (AppleRunLoopData *) info;
    arld->lock->lock();
    while (arld->m_workQueue->size() > 0) {
        bp::runloop::Event work(*(arld->m_workQueue->begin()));
        arld->m_workQueue->erase(arld->m_workQueue->begin());
        if (arld->onEventFunc) {
            arld->lock->unlock();
            arld->onEventFunc(arld->onEventCookie, work);
            arld->lock->lock();
        }
    }
    arld->lock->unlock();
}

void
bp::runloop::RunLoop::init()
{
    [NSApplication sharedApplication];

    AppleRunLoopData * arld = new AppleRunLoopData;
    arld->pool = [[NSAutoreleasePool alloc] init];

    arld->runLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];

    arld->m_workQueue = &m_workQueue;
    arld->onEventFunc = m_onEvent;
    arld->onEventCookie = m_onEventCookie;

    arld->lock = &m_lock;
    memset((void *) &(arld->cx), 0, sizeof(arld->cx));
    arld->cx.info = (void *) arld;
    arld->cx.perform = appleGotEventCallback;

    arld->source = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &(arld->cx));
    BPASSERT(arld->source != NULL);
    
    CFRunLoopAddSource(arld->runLoop, arld->source, kCFRunLoopCommonModes);

    m_osSpecific = (void *) arld;
}

void
bp::runloop::RunLoop::shutdown()
{
    BPASSERT(m_osSpecific != NULL);
    AppleRunLoopData * arld = (AppleRunLoopData *) m_osSpecific;

    CFRunLoopRemoveSource([[NSRunLoop currentRunLoop] getCFRunLoop],
                          arld->source,
                          kCFRunLoopCommonModes);

    CFRunLoopSourceInvalidate(arld->source);
    CFRelease(arld->source);
    arld->source = NULL;

    [arld->pool release];
    delete arld;
    m_osSpecific = NULL;
}

void
bp::runloop::RunLoop::setCallBacks(eventCallBack onEvent, void * onEventCookie)
{
    AppleRunLoopData * arld = (AppleRunLoopData *) m_osSpecific;

    m_onEvent = onEvent;
    m_onEventCookie = onEventCookie;
    if (arld) {
        arld->onEventFunc = onEvent;
        arld->onEventCookie = onEventCookie;
    }
}

void
bp::runloop::RunLoop::run()
{
    BPASSERT(m_osSpecific != NULL);

    CFRunLoopRun();

    // process all pending events before shutting down
    appleGotEventCallback(m_osSpecific);
}

bool
bp::runloop::RunLoop::sendEvent(Event e)
{
    m_lock.lock();
    BPASSERT(m_osSpecific != NULL);

    AppleRunLoopData * arld = (AppleRunLoopData *) m_osSpecific;

    m_workQueue.push_back(e);

    CFRunLoopSourceSignal(arld->source);
    CFRunLoopWakeUp(arld->runLoop);
    
    m_lock.unlock();
    return true;
}

void
bp::runloop::RunLoop::stop()
{
    AppleRunLoopData * arld = (AppleRunLoopData *) m_osSpecific;
    BPASSERT(arld != NULL && arld->runLoop != NULL);
    CFRunLoopStop(arld->runLoop);
}
