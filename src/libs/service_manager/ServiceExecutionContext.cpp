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

/**
 * ServiceExecutionContext.cpp
 * 
 * Default implementation returns empty strings and always generates
 * UserDeny events. 
 *
 * Created by Gordon Durand on Fri Nov 16 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */
 
#include "ServiceExecutionContext.h"
#include "BPUtils/bptypeutil.h"

using namespace std;
using namespace std::tr1;


ServiceExecutionContext::ServiceExecutionContext()
{
}


ServiceExecutionContext::~ServiceExecutionContext()
{
}
 

std::string
ServiceExecutionContext::locale()
{
    return "";
}


std::string 
ServiceExecutionContext::URI()
{
    return "";
}

std::string 
ServiceExecutionContext::userAgent()
{
    return "";
}

long
ServiceExecutionContext::clientPid()
{
    return 0;
}

void
ServiceExecutionContext::promptUser(
        weak_ptr<IServiceExecutionContextListener> l,
        unsigned int id,
        const boost::filesystem::path&,
        const bp::Object *)  
{
    if(shared_ptr<IServiceExecutionContextListener> r = l.lock()) {
        r->onUserResponse(id, bp::Null());
    }
}


void 
ServiceExecutionContext::invokeCallback(unsigned int, const bp::Map*)
{
}
