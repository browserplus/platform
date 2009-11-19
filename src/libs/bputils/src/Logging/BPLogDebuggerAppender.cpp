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
 *  BPLogDebuggerAppender.cpp
 *
 *  Created by David Grigsby on 9/19/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogDebuggerAppender.h"
#include <iostream>

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include "bpstrutil.h"

using namespace std;


namespace bp {
namespace log {



DebuggerAppender::DebuggerAppender( LayoutPtr layout ) :
    Appender( layout )
{

}


DebuggerAppender::~DebuggerAppender()
{

}


void DebuggerAppender::append( LoggingEventPtr event )
{
#if defined(_MSC_VER)    
    std::string sMsg;
    m_layout->format( sMsg, event );
    sMsg.append( "\r\n" );
    wstring wsMsg = bp::strutil::utf8ToWide( sMsg );
    
    OutputDebugStringW( wsMsg.c_str() );
#else
    
#endif
}


} // log
} // bp
