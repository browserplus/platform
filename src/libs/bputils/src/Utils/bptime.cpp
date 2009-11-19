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

/*
 *  bptime.cpp
 *
 *  Created by Gordon Durand on 10/29/07
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "api/bptime.h"
#include <time.h>
#include <string.h>

BPTime::BPTime() : m_time(time(NULL))
{
}
    

BPTime::BPTime(long t) : m_time(t)
{
}

#ifdef WIN32    
BPTime::BPTime(time_t t) : m_time(t)
{
}
#endif


BPTime::BPTime(const std::string& s) // throw(std::runtime_error)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
#ifndef WIN32
    if (strptime(s.c_str(), "%Y-%m-%d %H:%M:%SZ", &tm) == NULL) {
#else
    char tz;
    int num = sscanf_s(s.c_str(), "%4d-%2d-%2d %2d:%2d:%2d%c", 
                       &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                       &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &tz);
    tm.tm_year -= 1900;
    tm.tm_mon--;
    if (num != 7 || tz != 'Z') {
#endif
        throw std::runtime_error("illegal BPTime string format");
    }
#ifndef WIN32
    m_time = timegm(&tm);
#else
    m_time = _mkgmtime(&tm);
#endif

    if (m_time == -1) {
       throw std::runtime_error("unable to create BPTime");
    }
}


BPTime::~BPTime()
{
}


void 
BPTime::set(time_t t)
{
    m_time = t;
}


void 
BPTime::set(const BPTime & t)
{
    m_time = t.m_time;
}


time_t
BPTime::get() const
{
    return m_time;
}


BPTime::BPTime(const BPTime& other)
{
    m_time = other.m_time;
}
    

std::string 
BPTime::asString() const
{
    char buf[80];
#if defined(_MSC_VER )
    tm tmGmt;
    gmtime_s(&tmGmt, &m_time);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%SZ", &tmGmt);
#else
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%SZ", gmtime(&m_time));
#endif
    return std::string(buf);
}
    

int 
BPTime::compare(const BPTime& other) const
{
    long diff = diffInSeconds(other);
    if (diff < 0) {
        return -1;
    } else if (diff > 0) {
        return 1;
    }
    return 0;
}


long 
BPTime::diffInSeconds(const BPTime& other) const
{
    return((unsigned long)m_time - (unsigned long)other.m_time);
}


//////////////////////////////
// Utility functions
//
namespace bp {
namespace time {

/**
 * Sleep for the specified number of seconds.
 */
void sleepSec(double sec)
{
#ifdef WIN32
    Sleep(sec==-1 ? INFINITE: DWORD(sec*1000)); 
#else
    sleep(sec);
#endif
}

}
}

