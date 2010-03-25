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
 *  bprandom.cpp
 *
 *  Created by Gordon Durand on 3/22/2010.
 *  
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include "bprandom.h"
#include "BPLog.h"

#ifdef LINUX
// on linux we use openssl for true random numbers
#include <openssl/rand.h>
#endif

namespace bp {
namespace random {


int
generate()
{
    unsigned int i = 0;
#ifdef WIN32
    if (::rand_s(&i) != 0) {
        BPLOG_WARN("::rand_s() failed, reverting to ::rand()");
        i = (unsigned int) ::rand();
    }
#elif defined(MACOSX)
    i = ::arc4random();
#else
    RAND_bytes((unsigned char *) &i, sizeof(i));
    // XXX: failure?  should we throw fatal in case of failure?
#endif
    return(i % ((unsigned)RAND_MAX + 1));
}

   
} // random
} // bp

