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
 *  ServiceLogging.cpp
 *
 *  Support routines to permit BPUtils logging by services that link
 *  with BPUtils.
 *  
 *  Created by David Grigsby on Mon August 03 2008.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 */
#include "ServiceLogging.h"
#include "BPUtils/BPLog.h"
#include "ServiceAPI/bpcfunctions.h"

using namespace bp::log;

namespace bp {
namespace service {


//////////////////////////////////////////////////////////////////////
// An appender which logs using the Service API log function.
//
class ServiceApiAppender : public Appender
{
public:
    ServiceApiAppender( LogFuncPtr pfnLog, LayoutPtr layout );
    virtual void append( LoggingEventPtr evt );

private:
    static unsigned int ServiceLevelFromBPLevel( bp::log::Level level );

    LogFuncPtr   m_pfnLog;
};

typedef std::tr1::shared_ptr<ServiceApiAppender> ServiceApiAppenderPtr;


ServiceApiAppender::ServiceApiAppender( LogFuncPtr pfnLog,
                                        LayoutPtr layout ) :
    Appender( layout ),
    m_pfnLog( pfnLog )
{
}


unsigned int ServiceApiAppender::ServiceLevelFromBPLevel( bp::log::Level level )
{
    switch (level)
    {
        case LEVEL_DEBUG:   return BP_DEBUG;
        case LEVEL_INFO:    return BP_INFO;
        case LEVEL_WARN:    return BP_WARN;
        case LEVEL_ERROR:   return BP_ERROR;
        case LEVEL_FATAL:   return BP_FATAL;
        default:            return BP_DEBUG;
    }
}


void ServiceApiAppender::append( LoggingEventPtr evt )
{
    std::string sMsg;
    m_layout->format( sMsg, evt );
    m_pfnLog( ServiceLevelFromBPLevel( evt->level() ), "%s", sMsg.c_str() );
}



//////////////////////////////////////////////////////////////////////
// Setup service logging system to use a ServiceApiAppender.
//
void setupBPUtilsLogging( LogFuncPtr pfnLog )
{
    // For now forward all events.
    // Config-based filtering will still occur later in the logging process.
    bp::log::Level threshold = LEVEL_DEBUG;

    // For now always use FM layout.
    // Other fields will be prepended "higher" in the logging process.
    LayoutPtr layout( layoutFromString( "FuncMsg" ) );
    
    ServiceApiAppenderPtr apdr( new ServiceApiAppender( pfnLog, layout ) );
    apdr->setThreshold( threshold );
    rootLogger().addAppender( apdr ); 
}


} // service
} // bp

