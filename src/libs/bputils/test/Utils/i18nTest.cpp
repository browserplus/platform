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

#include "i18nTest.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"


CPPUNIT_TEST_SUITE_REGISTRATION(i18nTest);

void
i18nTest::storeLoadFileToString()
{
    bp::file::Path bgFilePath("булгарски_FileName.txt");
    std::string fileText("hello\nthere\nworld\n");
    std::string fileTextRead;
    bp::file::Path path = bp::file::getTempDirectory() / bgFilePath;

    CPPUNIT_ASSERT(bp::strutil::storeToFile(path, fileText));
    CPPUNIT_ASSERT(bp::strutil::loadFromFile(path, fileTextRead));
    CPPUNIT_ASSERT(!fileText.compare(fileTextRead));    
    CPPUNIT_ASSERT(boost::filesystem::exists(path));
    CPPUNIT_ASSERT(bp::file::remove(path));
    CPPUNIT_ASSERT(!boost::filesystem::exists(path));
}
