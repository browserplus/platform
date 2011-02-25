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

#include "LogPlugletFactory.h"
#include "LogPluglet.h"

BPArgumentDefinition s_logArguments[] =
{
    {
        (BPString) "location",
        (BPString) "The location (e.g. method name) from which the log event is emitted.",
        BPTString,
        true
    },
    {
        (BPString) "message",
        (BPString) "The message to be logged.",
        BPTString,
        true
    }
};


BPFunctionDefinition s_logFunctions[] =
{
    {
        (BPString) "Fatal",
        (BPString) "Log a fatal error to the Browserplus plugin logging facility.",
        sizeof(s_logArguments)/sizeof(s_logArguments[0]),
        s_logArguments
    },
    {
        (BPString) "Error",
        (BPString) "Log an error to the Browserplus plugin logging facility.",
        sizeof(s_logArguments)/sizeof(s_logArguments[0]),
        s_logArguments
    },
    {
        (BPString) "Warn",
        (BPString) "Log a warning to the Browserplus plugin logging facility.",
        sizeof(s_logArguments)/sizeof(s_logArguments[0]),
        s_logArguments
    },
    {
        (BPString) "Info",
        (BPString) "Log an informational message to the Browserplus plugin logging facility.",
        sizeof(s_logArguments)/sizeof(s_logArguments[0]),
        s_logArguments
    },
    {
        (BPString) "Debug",
        (BPString) "Log a debug message to the Browserplus plugin logging facility.",
        sizeof(s_logArguments)/sizeof(s_logArguments[0]),
        s_logArguments
    }
};


// a description of this pluglet.
static BPServiceDefinition s_logPlugletDef =
{
    (BPString) "Log",
    1, 0, 1,
    (BPString) "Log to BrowserPlus's logfile on disk.  The available levels in order of "
    "severity are Fatal, Error, Warn, Info, Debug.",
    sizeof(s_logFunctions)/sizeof(s_logFunctions[0]),
    s_logFunctions
};


LogPlugletFactory::LogPlugletFactory()
{
    m_descriptions.push_back(bp::service::Description());
    m_descriptions.back().fromBPServiceDefinition(&s_logPlugletDef);
}


std::list<Pluglet*>
LogPlugletFactory::createPluglets(BPPlugin* plugin)
{
    std::list<Pluglet*> rval;
    std::list<bp::service::Description>::const_iterator it;
    for (it = m_descriptions.begin(); it != m_descriptions.end(); ++it) {
        rval.push_back(new LogPluglet(plugin, *it));
    }
    return rval;
}

