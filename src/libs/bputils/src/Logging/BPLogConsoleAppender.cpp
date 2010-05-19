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
 *  BPLogConsoleAppender.cpp
 *
 *  Created by David Grigsby on 9/19/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogConsoleAppender.h"
#include "bpstrutil.h"
#include "bprandom.h"

#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif

using namespace std;


namespace bp {
namespace log {


// Use scenarios:
// 1) Console app
// 2) Console app that spawns a windowless console app (latter s/b able to log)
// 3) Gui app (should bring up a console).

// TODO
// * cout vs. cerr
// * immediateFlush?
// * Should a spawned process write to parent process console or a separate one?

ConsoleAppender::ConsoleAppender( LayoutPtr layout,
                                  const std::string& sConsoleTitle ) :
    Appender( layout ),
    m_sConsoleTitle( sConsoleTitle )
{
#ifdef WIN32
    // This is for scenario 3 above (gui app).
    // It should fail benignly for the other scenarios.
    (void) AllocConsole();

    // For Scenario 2 above (console app spawns windowless console app
    // (using CREATE_NO_WINDOW)), the child process will have an
    // invisible console that cannot be made visible (because no window).
    // In that scenario, attach to parent console.
    // Note: Another approach would be to do this kind of work in the
    //       ConsoleAppender *client* rather than in the appender
    //       itself, because the client will better know its console status.
    //       The disadvant of course, is requiring clients to do that work.
    if (!GetConsoleWindow())
    {
        FreeConsole();
        AttachConsole( ATTACH_PARENT_PROCESS );
    }
    
    m_hConsoleBuff = GetStdHandle( STD_OUTPUT_HANDLE );
    m_hwndConsole = NULL;
#endif
}


ConsoleAppender::~ConsoleAppender()
{
    
}


// IOStreams have internal thread sync.
// To more easily demonstrate threading issues, uncomment this version.
//void ConsoleAppender::append( LoggingEventPtr event )
//{
//    std::string sMsg;
//    m_layout->format( sMsg, event );
//
//    for (unsigned int i=0; i<sMsg.length(); ++i)
//    {
//        printf( "%c", sMsg[i] );
//        Sleep( bp::random::generate() % 5 );
//    }
//    printf( "%c", '\n' );
//}

#ifdef WIN32
void ConsoleAppender::append( LoggingEventPtr event )
{
    std::string sMsg;
    m_layout->format( sMsg, event );

    sMsg.append("\r\n");
    std::wstring sMsgW = bp::strutil::utf8ToWide(sMsg);

    // Note: using wcout, the output is visible but in multiprocess
    // scenarios interleaving doesn't work well and some garbling often occurs.
    DWORD numWritten = 0;
    WriteConsoleW( m_hConsoleBuff, (void *) sMsgW.c_str(),
                   sMsgW.length(), &numWritten, NULL);
}
#else
void ConsoleAppender::append( LoggingEventPtr event )
{
    std::string sMsg;
    m_layout->format( sMsg, event );

    std::cout << sMsg << std::endl;
}
#endif

} // log
} // bp
