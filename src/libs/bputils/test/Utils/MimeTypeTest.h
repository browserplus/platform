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
 * MimeTypeTest.h
 * A test of the bp::mimetype component
 *
 * Created by Gordon Durand on Tue June 24 2008
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __MIMETYPETEST_H__
#define __MIMETYPETEST_H__

#include "TestingFramework/TestingFramework.h"
//#include "BPUtils/BPUtils.h"

class MimeTypeTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(MimeTypeTest);
    CPPUNIT_TEST(goodExt);
    CPPUNIT_TEST(badExt);
    CPPUNIT_TEST(noExt);
    CPPUNIT_TEST(goodType);
    CPPUNIT_TEST(badType);
    CPPUNIT_TEST(noType);
    CPPUNIT_TEST(chaseLink);
    CPPUNIT_TEST_SUITE_END();
    
  protected:
    void goodExt();
    void badExt();
    void noExt();
    void goodType();
    void badType();
    void noType();
    void chaseLink();
};

#endif
