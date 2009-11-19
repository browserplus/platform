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
 * BPTARTest.cpp
 * Unit tests for round trip creation and extraction of tarfiles
 *
 * Created by Lloyd Hilaiel on 3/04/09.
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __BPTARTEST_H__
#define __BPTARTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpfile.h"

class BPTARTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(BPTARTest);
    CPPUNIT_TEST(testDirectoryRoundTrip);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    void setUp();
    void tearDown();

  protected:
    void testDirectoryRoundTrip();
    
  private:
    bp::file::Path m_testDirPath;
    bp::file::Path m_testFilePath;
    bp::file::Path m_tarPath;
    bp::file::Path m_unpackPath;
};

#endif
