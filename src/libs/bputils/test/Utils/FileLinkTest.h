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
 * FileLinkTest.h
 * Unit tests for the bp::file link methods
 *
 * Created by Gordon Durand on 15 July 2008.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __FILELINKTEST_H__
#define __FILELINKTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "BPUtils/bpfile.h"

#include <string>

class FileLinkTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(FileLinkTest);
    CPPUNIT_TEST(createLink);
    CPPUNIT_TEST(brokenLink);
#ifndef WIN32
    // Not a valid test on windows, which cannot handle
    // links embedded in paths, e.g.
    // \dir\link_to_dir\file
    CPPUNIT_TEST(circularLink);
#endif
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    
  protected:
    void createLink();
    void brokenLink();
    void circularLink();

  private:
    bp::file::Path m_dir;
};

#endif
