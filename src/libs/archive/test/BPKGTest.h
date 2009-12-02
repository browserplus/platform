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

/**
 * BPKGTest.cpp
 * Unit tests for round trip creation and extraction of browserplus
 * packaging format
 *
 * Created by Lloyd Hilaiel on 2/11/09.
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __BPKGTEST_H__
#define __BPKGTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpfile.h"

class BPKGTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(BPKGTest);
    CPPUNIT_TEST(testDirectoryRoundTrip);
    CPPUNIT_TEST(testFileRoundTrip);
    CPPUNIT_TEST(testStringRoundTrip);
    CPPUNIT_TEST_SUITE_END();
    
  public:
    void setUp();
    void tearDown();

  protected:
    void testDirectoryRoundTrip();
    void testFileRoundTrip();
    void testStringRoundTrip();
    
  private:
    bp::file::Path m_baseDirPath;
    bp::file::Path m_testDirPath;
    bp::file::Path m_testFilePath;
    bp::file::Path m_keyFile;
    bp::file::Path m_certFile;
    bp::file::Path m_bpkgPath;
    bp::file::Path m_unpackPath;
};

#endif
