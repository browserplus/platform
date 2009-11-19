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

/**
 * CoreletExecutionContext.cpp
 * 
 * Default implementation returns empty strings and always generates
 * UserDeny events. 
 *
 * Created by Gordon Durand on Fri Nov 16 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */
 
#include "api/CoreletExecutionContext.h"
#include "BPUtils/bptypeutil.h"

using namespace std;
using namespace std::tr1;


CoreletExecutionContext::CoreletExecutionContext()
{
}


CoreletExecutionContext::~CoreletExecutionContext()
{
}
 

std::string
CoreletExecutionContext::locale()
{
    return "";
}


std::string 
CoreletExecutionContext::URI()
{
    return "";
}

std::string 
CoreletExecutionContext::userAgent()
{
    return "";
}

long
CoreletExecutionContext::clientPid()
{
    return 0;
}

void
CoreletExecutionContext::promptUser(
        weak_ptr<ICoreletExecutionContextListener> l,
        unsigned int id,
        const bp::file::Path&,
        const bp::Object *)  
{
    if(shared_ptr<ICoreletExecutionContextListener> r = l.lock()) {
        r->onUserResponse(id, bp::Null());
    }
}


void 
CoreletExecutionContext::invokeCallback(unsigned int, const bp::Map*)
{
}
