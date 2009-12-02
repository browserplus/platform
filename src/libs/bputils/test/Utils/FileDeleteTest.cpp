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

#include "FileDeleteTest.h"
#include <iostream>
#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"

CPPUNIT_TEST_SUITE_REGISTRATION(FileDeleteTest);

using namespace bp::file;
namespace bfs = boost::filesystem;


void 
FileDeleteTest::deleteDir()
{
    CPPUNIT_ASSERT(remove(m_testDir));
}


void 
FileDeleteTest::deleteReadOnlyDir()
{
    // set readonly on foo.txt and subdir, then try to delete
    Path path = m_testDir / "foo.txt";
    Path dir = m_testDir / "subdir";

    FileInfo fi;
    CPPUNIT_ASSERT(statFile(path, fi));
    fi.mode &= ~0200;
    CPPUNIT_ASSERT(setFileProperties(path, fi));

    CPPUNIT_ASSERT(statFile(dir, fi));
    fi.mode &= ~0200;
    CPPUNIT_ASSERT(setFileProperties(dir, fi));

    CPPUNIT_ASSERT(remove(m_testDir));
}


void 
FileDeleteTest::deleteFileInReadOnlyDir()
{
    // set readonly on and subdir, then try to delete subdir/bar.txt
    Path dir = m_testDir / "subdir";

    FileInfo fi;
    CPPUNIT_ASSERT(statFile(dir, fi));
    fi.mode &= ~0200;
    CPPUNIT_ASSERT(setFileProperties(dir, fi));

    Path file = m_testDir / "subdir" / "bar.txt";
    CPPUNIT_ASSERT(remove(file));
}


void
 FileDeleteTest::setUp()
{
    // create our test directory
    // <m_testDir>/
    // <m_testDir>/foo.txt
    // <m_testDir>/subdir
    // <m_testDir>/subdir/bar.txt

    Path path = getTempPath(getTempDirectory(), "FileDeleteTest");

    m_testDir = path;
    CPPUNIT_ASSERT(remove(m_testDir));
    CPPUNIT_ASSERT(bfs::create_directories(m_testDir));

    // now let's build up the test directory structure
    // m_testDir/foo.txt  
    path /= "foo.txt";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(path, "this is a test file\n"));

    // m_testDir/subdir/bar.txt
    path = m_testDir / "subdir";
    CPPUNIT_ASSERT(bfs::create_directory(path));
    path /= "bar.txt";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(path, "this is bar.txt\n"));    
}


void
FileDeleteTest::tearDown()
{
    CPPUNIT_ASSERT(remove(m_testDir));
}
