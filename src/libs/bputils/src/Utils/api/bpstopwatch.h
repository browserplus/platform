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

#ifndef BPSTOPWATCH_H_
#define BPSTOPWATCH_H_

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef WIN32
#include "Windows.h"
#endif


namespace bp {
namespace time {


class Stopwatch
{
public:
    Stopwatch();

    bool running();
    
    void start();
    void stop();
    void reset();

    double elapsedSec();
    
private:
    unsigned int m_cur_ms;
    unsigned int m_cur_s;
    unsigned int m_tot_ms;
    unsigned int m_tot_s;
};


#ifdef WIN32
class PerfStopwatch
{
public:
    PerfStopwatch() {
        QueryPerformanceFrequency( &m_liFreq );
        QueryPerformanceCounter( &m_liStartTime );
    }

    void restart() {
        QueryPerformanceCounter( &m_liStartTime );
    }

    double elapsedSec(void) const {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        return (double)(t.QuadPart - m_liStartTime.QuadPart) /
               (double)m_liFreq.QuadPart;
    }

private:
    LARGE_INTEGER m_liStartTime;
    LARGE_INTEGER m_liFreq;
};
#endif


} // namespace time
} // namespace bp

#endif