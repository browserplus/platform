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
 *  BPLogConsoleAppender.h
 *
 *  Created by David Grigsby on 9/19/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGCONSOLEAPPENDER_H_
#define _BPLOGCONSOLEAPPENDER_H_

#include "BPLogAppender.h"
#ifdef WIN32
#include <Windows.h>
#endif

namespace bp {
namespace log {


class ConsoleAppender : public Appender
{
public:
    ConsoleAppender( LayoutPtr layout,
                     const std::string& sConsoleTitle = "" );
    virtual ~ConsoleAppender();

    virtual void append( LoggingEventPtr evt );
    
private:
    std::string m_sConsoleTitle;
#ifdef WIN32    
    HWND        m_hwndConsole;
    HANDLE      m_hConsoleBuff;
#endif
    
    ConsoleAppender( const ConsoleAppender& );
    ConsoleAppender& operator=( const ConsoleAppender& );
};

typedef std::tr1::shared_ptr<ConsoleAppender> ConsoleAppenderPtr;


} // bp
} // log

#endif // _BPLOGCONSOLEAPPENDER_H
