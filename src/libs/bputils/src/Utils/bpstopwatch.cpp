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

#ifdef WIN32
#include <Windows.h>
#include <MMSystem.h>
#else
#include <sys/time.h>
#endif

#include "api/bpstopwatch.h"

bp::time::Stopwatch::Stopwatch()
{
    m_cur_ms = m_cur_s = m_tot_ms = m_tot_s = 0;
}

bool
bp::time::Stopwatch::running()
{
    return (m_cur_ms != 0 || m_cur_s != 0);
}

void
bp::time::Stopwatch::start()
{
#ifdef WIN32
    m_cur_ms = timeGetTime();
#else
    struct timeval tv;  
    gettimeofday(&tv, NULL);    
    m_cur_s = tv.tv_sec;
    m_cur_ms = tv.tv_usec / 1000;
#endif
}

void
bp::time::Stopwatch::stop()
{
#ifdef WIN32
  unsigned int now = timeGetTime();	
  m_tot_ms += (now - m_cur_ms);
  m_tot_s += m_tot_ms / 1000;
  m_tot_ms %= 1000;
#else
  struct timeval tv;  
  gettimeofday(&tv, NULL);    
  m_tot_ms += (tv.tv_sec - m_cur_s) * 1000 +
		((tv.tv_usec / 1000) - m_cur_ms);
  m_tot_s += m_tot_ms / 1000;
  m_tot_ms %= 1000;
#endif
  m_cur_s = m_cur_ms = 0;
}

void
bp::time::Stopwatch::reset()
{
    m_cur_ms = m_cur_s = m_tot_ms = m_tot_s = 0;
}

double
bp::time::Stopwatch::elapsedSec()
{
    // stop and resume watch if it's already running 
    bool running = this->running();
    if (running) this->stop();
    double t = ((double) m_tot_s) + ((double) m_tot_ms / 1000.0);
    if (running) this->start();    
    return t;
}
    
