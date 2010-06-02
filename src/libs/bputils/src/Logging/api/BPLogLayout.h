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
 *  BPLogLayout.h
 *
 *  Created by David Grigsby on 9/26/07.
 *  Portions based upon code from log4cxx.
 *  
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGLAYOUT_H_
#define _BPLOGLAYOUT_H_

#include "BPUtils/BPLogEvent.h"
#include "BPUtils/BPLogTime.h"


namespace bp {
namespace log {

class Layout
{
public:    
    Layout();
    virtual ~Layout();

    // Set the time format for this layout.
    void setTimeFormat( const TimeFormat& tf );
    
    // Layout-specific format event into string.
    virtual void format( std::string& sOut, const LoggingEventPtr& event ) = 0;

    // Append layout-specific header.
    virtual void appendHeader( std::string& /*sOut*/ ) {}

    // Append layout-specific footer.
    virtual void appendFooter( std::string& /*sOut*/ ) {}

    static void appendFile( std::string& sOut, const LoggingEventPtr& evt );

    static void appendFunc( std::string& sOut, const LoggingEventPtr& evt );

    static void appendLevel( std::string& sOut, const LoggingEventPtr& evt );

    static void appendLine( std::string& sOut, const LoggingEventPtr& evt );

    static void appendMessage( std::string& sOut, const LoggingEventPtr& evt );

    static void appendThread( std::string& sOut, const LoggingEventPtr& evt );

    static void appendPid( std::string& sOut, const LoggingEventPtr& evt );

    void appendTime( std::string& sOut, const LoggingEventPtr& evt );

    
private:
    TimeFormat m_timeFormat;
};

typedef std::tr1::shared_ptr<Layout> LayoutPtr;


} // log
} // bp


#endif // _BPLOGLAYOUT_H
