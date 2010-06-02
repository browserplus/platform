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

/*
 *  bptimer.h
 *
 *  an abstraction which can asynchronously call back into client code
 *  on the same thread as the timer was set on.
 *
 *  Created by Lloyd Hilaiel on 8/21/08.
 */

#include "api/bptimer.h"
#include "api/bperrorutil.h"
#include <Cocoa/Cocoa.h>

using namespace bp::time;

@interface MY_TIMER_FIRED_CLASS : NSObject {
@public
    ITimerListener * listener;
    Timer * t;
};

- (void)myTimerFireMethod:(NSTimer*)theTimer;
@end;
@implementation MY_TIMER_FIRED_CLASS
- (void)myTimerFireMethod:(NSTimer*)theTimer
{
    if (listener) listener->timesUp(t);
}
@end;
struct DarwinTimerData
{
    MY_TIMER_FIRED_CLASS * recipient;
    NSTimer * currentTimer;
};

Timer::Timer() : m_osSpecific(NULL), m_listener(NULL)
{
    DarwinTimerData * dtd = new DarwinTimerData;
    dtd->recipient = [ [ MY_TIMER_FIRED_CLASS alloc ] init ];
    dtd->recipient->t = this;
    dtd->currentTimer = NULL;
    m_osSpecific = (void *) dtd;
}

void
Timer::setListener(ITimerListener * listener)
{
    DarwinTimerData * dtd = (DarwinTimerData *) m_osSpecific;
    dtd->recipient->listener = listener;    
}


Timer::~Timer()
{
    DarwinTimerData * dtd = (DarwinTimerData *) m_osSpecific;
    cancel();
    [dtd->recipient release];
    delete dtd;
}

void
Timer::setMsec(unsigned int timeInMilliseconds)
{
    DarwinTimerData * dtd = (DarwinTimerData *) m_osSpecific;

    cancel();

    BPASSERT( dtd->recipient != NULL );
    BPASSERT( dtd->currentTimer == NULL );    

    // convert ms to double seconds representation
    double ti = timeInMilliseconds;
    ti /= 1000.0;
    
    dtd->currentTimer =
        [NSTimer scheduledTimerWithTimeInterval: ti
                                         target: dtd->recipient
                                       selector: @selector(myTimerFireMethod:)
                                       userInfo: nil
                                        repeats: NO];
    [dtd->currentTimer retain];
    
    BPASSERT( dtd->currentTimer != nil );
}

void
Timer::cancel()
{
    DarwinTimerData * dtd = (DarwinTimerData *) m_osSpecific;

    if (dtd->currentTimer != nil) {
        if ([dtd->currentTimer isValid]) {
            [dtd->currentTimer invalidate];
        }
        [dtd->currentTimer release];
        dtd->currentTimer = nil;
    }
}
