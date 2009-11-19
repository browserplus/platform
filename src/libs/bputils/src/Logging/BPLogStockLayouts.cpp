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
 *  BPLogStockLayouts.cpp
 *
 *  Implements built-in layouts.
 *  
 *  Created by David Grigsby on 6/16/09.
 *  
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogStockLayouts.h"


namespace bp {
namespace log {


void RawLayout::format( std::string& sOut, const LoggingEventPtr& evt )
{
    // Format is: just the raw message.
    appendMessage( sOut, evt );
}


void SourceLayout::format( std::string& sOut, const LoggingEventPtr& evt )
{
    // Format is: file(line): func - message
    appendFile( sOut, evt );
    sOut.append( "(" );
    appendLine( sOut, evt );
    sOut.append( "): " );
    appendFunc( sOut, evt );
    sOut.append( " - " );
    appendMessage( sOut, evt );
}


void StandardLayout::format( std::string& sOut, const LoggingEventPtr& evt )
{
    // Format is: time [pid:tid] level - func: message
    appendTime( sOut, evt );
    sOut.append( " [" );
    appendPid( sOut, evt );
    sOut.append( ":" );
    appendThread( sOut, evt );
    sOut.append( "] ");
    appendLevel( sOut, evt );
    sOut.append( " - " );
    appendFunc( sOut, evt );
    sOut.append( ": " );
    appendMessage( sOut, evt );
}


void ThrdLvlFuncMsgLayout::format( std::string& sOut, const LoggingEventPtr& evt )
{
    // Format is: [pid:tid] level - func: message
    sOut.append( "[" );
    appendPid( sOut, evt );
    sOut.append( ":" );
    appendThread( sOut, evt );
    sOut.append( "] ");
    appendLevel( sOut, evt );
    sOut.append( " - " );
    appendFunc( sOut, evt );
    sOut.append( ": " );
    appendMessage( sOut, evt );
}


void TimeLvlMsgLayout::format( std::string& sOut, const LoggingEventPtr& evt )
{
    // Format is: time level - message
    appendTime( sOut, evt );
    sOut.append( " " );
    appendLevel( sOut, evt );
    sOut.append( " - " );
    appendMessage( sOut, evt );
}


void FuncMsgLayout::format( std::string& sOut, const LoggingEventPtr& evt )
{
    // Format is: func: message
    appendFunc( sOut, evt );
    sOut.append( ": " );
    appendMessage( sOut, evt );
}


} // log
} // bp
