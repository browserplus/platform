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
 * LZMATest.cpp
 * Unit tests for LZMA compresion
 *
 * Created by Lloyd Hilaiel on 2/06/09.
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "LZMATest.h"
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(LZMATest);

void LZMATest::testRoundTrip()
{
    std::string orig("this is the original string");
    std::string compressed;
    std::string decompressed;
    
    // first let's compress
    {
        bp::lzma::Compress c;
        std::istringstream in(orig);
		std::ostringstream out(std::ios_base::binary | std::ios_base::out);        
        c.setInputStream(in);
        c.setOutputStream(out);        
        CPPUNIT_ASSERT(c.run());
        compressed = out.str();
    }
    
    // verify compressed and orig are different
    CPPUNIT_ASSERT(0 != orig.compare(compressed));

    // now decompress
    {
        bp::lzma::Decompress d;
        std::istringstream in(compressed);
        std::stringstream out;        
        d.setInputStream(in);
        d.setOutputStream(out);        
        CPPUNIT_ASSERT(d.run());
        decompressed = out.str();
    }
}
