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
 *  LogPluglet.cpp
 *
 *  Pluglet that allows js to access the plugin logging facility.
 *  
 *  Created by David Grigsby on 12/12/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "LogPluglet.h"
#include "BPUtils/BPLog.h"

using std::string;

bool getArguments( const bp::Object& oArguments,
                   string& sLocation, string& sMessage );


LogPluglet::LogPluglet(BPPlugin * plugin,
                       const bp::service::Description & desc) 
    :Pluglet(plugin, desc)
{
}


LogPluglet::~LogPluglet()
{
}


void
LogPluglet::execute( unsigned int /*nTid*/,
                     const char* cszFunction,
                     const bp::Object* poArguments,
                     bool /*bSyncInvocation*/,
                     plugletExecutionSuccessCB /*cbSuccess*/,
                     plugletExecutionFailureCB /*cbFailure*/,
                     plugletInvokeCallbackCB /*cbCallback*/,
                     void* /*pvCallbackArgument*/ )
{
    if (!cszFunction || !poArguments) {
        BPLOG_WARN_STRM("execute called will NULL function or arguments");
        return;
    }

    string sLocation, sMessage;
    if (!getArguments( *poArguments, sLocation, sMessage ))
    {
        return;
    }

    string sFunction = cszFunction;

    if (sFunction == "Fatal")
    {
        bp::log::rootLogger()._conditionalLog( bp::log::LEVEL_FATAL, sMessage,
                                               bp::log::LocationInfo(sLocation));
    }
    else if (sFunction == "Error")
    {
        bp::log::rootLogger()._conditionalLog( bp::log::LEVEL_ERROR, sMessage,
                                               bp::log::LocationInfo(sLocation));
    }
    else if (sFunction == "Warn")
    {
        bp::log::rootLogger()._conditionalLog( bp::log::LEVEL_WARN, sMessage,
                                               bp::log::LocationInfo(sLocation));
    }
    else if (sFunction == "Info")
    {
        bp::log::rootLogger()._conditionalLog( bp::log::LEVEL_INFO, sMessage,
                                               bp::log::LocationInfo(sLocation));
    }
    else if (sFunction == "Debug")
    {
        bp::log::rootLogger()._conditionalLog( bp::log::LEVEL_DEBUG, sMessage,
                                               bp::log::LocationInfo(sLocation));
    }
    else
    {
        // TODO
    }    
}


bool getArguments( const bp::Object& oArgs, string& sLocation, string& sMessage )
{
    if (!oArgs.has( "location", BPTString ) ||
        !oArgs.has( "message", BPTString ))
    {
        return false;
    }
    
    sLocation = ((bp::String*) oArgs.get("location"))->value();
    sMessage = ((bp::String*) oArgs.get("message"))->value();

    return true;
}
