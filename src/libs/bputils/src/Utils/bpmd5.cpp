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
 *  bpmd5.cpp
 *
 *  Created by David Grigsby on 04/29/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpmd5.h"
#include <iomanip>
#include <sstream>
#include <string>

#include <openssl/md5.h>

using namespace std;


namespace bp {
namespace md5 {


string hash( const std::string& sIn )
{
    MD5_CTX ctx;
    MD5_Init(&ctx);

    MD5_Update(&ctx, sIn.c_str(), sIn.length() );

    unsigned char chBuf[MD5_DIGEST_LENGTH];
    MD5_Final( chBuf, &ctx );

    stringstream ss;
    for( int i=0; i<MD5_DIGEST_LENGTH; i++)
    {
        ss << setw(2) << setfill('0') << hex << (unsigned int)chBuf[i];
    }

    return ss.str();
}


} // md5
} // bp

