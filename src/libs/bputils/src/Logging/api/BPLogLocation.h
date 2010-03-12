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
 *  BPLogLocation.h
 *
 *  Created by David Grigsby on 9/18/07.
 *  Portions based upon code from log4cxx.
 *  
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGLOCATION_H_
#define _BPLOGLOCATION_H_

#include <string>

#include "bperrorutil.h"


namespace bp {
namespace log {

class LocationInfo
{
public:
    LocationInfo() :
        m_sFile( "unknown" ), m_sFunc( "unknown" ), m_nLine( 0 ) {}
   LocationInfo( const std::string& sFile, const std::string& sFunc,
                 int nLine ) :
        m_sFile( sFile ), m_sFunc( sFunc ), m_nLine( nLine ) {}
    LocationInfo( const std::string& sFunc ) :
        m_sFile( "unknown" ), m_sFunc( sFunc ), m_nLine( 0 ) {}

    std::string file()  { return m_sFile; }
    std::string func()  { return m_sFunc; }
    int line()          { return m_nLine; }
            
            
private:    
    std::string m_sFile;
    std::string m_sFunc;
    int m_nLine;
};

} // log
} // bp


#define BPLOG_LOCATION bp::log::LocationInfo(__FILE__, __BP_FUNC__, __LINE__)


#endif // _BPLOGLOCATION_H
