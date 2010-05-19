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

#pragma once

namespace bp {
namespace time {

class ITimerListener 
{
  public:
    virtual void timesUp(class Timer * t) = 0;
    virtual ~ITimerListener() { }
};

class Timer
{
public:
    Timer();
    ~Timer();
    // who will receive events when the timer expires
    void setListener(ITimerListener * listener);
    // start the timer, canceling any previous configuration
    // upon expiration ITimerListener::timesUp() will be invoked.
    void setMsec(unsigned int timeInMilliseconds);
    // stop the timer if running
    void cancel();

  private:
    void * m_osSpecific;
    ITimerListener * m_listener;

    // no copy/assignment
    Timer(const Timer &);
    Timer & operator=(const Timer &);
};

} // namespace time
} // namespace bp
