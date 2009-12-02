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

/*
 *  bptime.h
 *
 *  Created by Gordon Durand on 10/29/07
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPTIME_H__
#define __BPTIME_H__

#include <string>
#include <stdexcept>

class BPTime 
{
public:
    /**
     * Construct BPTime from current time
     */
    BPTime();
    
    /**
     * Construct BPTime from specified time
     * \param t - number of seconds since Jan 1 1970
     */
    BPTime(long t);

#ifdef WIN32    
    /**
     * Construct BPTime from a time_t struct
     * \param t - number of seconds since Jan 1 1970
     */
    BPTime(time_t t);
#endif
    
    /**
     * Construct BPTime from an ISO8601 string
     * yyyy-mm-dd hh:mm:ssZ
     * \param s - string
     */
    BPTime(const std::string& s); /*throw(std::runtime_error)*/
    
    /**
     * Construct BPTime from another BPTime
     * \param other - other BPTime
     */
    BPTime(const BPTime& other);
    
    /**
     * Destructor
     */
    virtual ~BPTime();
    
    /**
     * set the time
     * \param t - number of seconds since Jan 1 1970
     */
    void set(time_t t);
    void set(const BPTime & t);  
    
    /**
     * Return as number of seconds since 1/1/1970
     */
    time_t get() const;
    
    /**
     * Return an ISO8601 string (yyyy-mm-dd hh:mm:ssZ)
     */
    std::string asString() const;
    
    /** 
     * Compare this BPTime to another.
     * \param other - other BPTime to compare against
     * \returns  -1 if this < other
     *            0 if this == other
     *            1 if this > other
     */
    int compare(const BPTime& other) const;

    /**
     * Return difference between this and other (this - other)
     */
    long diffInSeconds(const BPTime& other) const;
    
private:
    time_t m_time;
};


namespace bp {
namespace time {

/**
 * Sleep for the specified number of seconds.
 * You may provide -1 for sleep infinite.
 */
void sleepSec(double sec);

}
}

#endif
