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
 *  bpuuid_Windows.cpp
 *
 *  Created by David Grigsby on 4/29/08.
 *  
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpuuid.h"
#include <Rpc.h>

namespace bp {
namespace uuid {


bool generate( std::string& sUuid )
{
    UUID uuid;
    if (UuidCreate( &uuid ) != RPC_S_OK)
        return false;

    unsigned char* pch = NULL;
    if (UuidToStringA( &uuid, &pch ) != RPC_S_OK)
        return false;

    if (pch)
        sUuid = reinterpret_cast<char*>(pch);
    
    RpcStringFreeA( &pch );
    return true;
}

   
} // uuid
} // bp

