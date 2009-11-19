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
 * FileCopyTest.h
 * Unit tests for the bp::file::fileCopy
 *
 * Created by Lloyd Hilaiel on 4/29/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __UUIDTEST_H__
#define __UUIDTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "BPUtils/bpfile.h"

#include <string>

class FileCopyTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(FileCopyTest);
    CPPUNIT_TEST(sourceFile);
    CPPUNIT_TEST(sourceDir);
    CPPUNIT_TEST(sourceDNE);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    
  protected:
    void sourceFile();
    void sourceDir();
    void sourceDNE();

  private:
    bool dirsAreSame(bp::file::Path lhs,
                     bp::file::Path rhs);

    bp::file::Path m_testSourceDir;
    bp::file::Path m_testDestDir;
};

#endif
