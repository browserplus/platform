/**
 * ***** BEGIN LICENSE BLOCK *****
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a bpf::safeCopy of the License at
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

#include "FileCopyTest.h"
#include <iostream>
#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"

namespace bpf = bp::file;
namespace bfs = boost::filesystem;

CPPUNIT_TEST_SUITE_REGISTRATION(FileCopyTest);

//  From bpfile.h
//  if the source path is a file and...
//    1) the source path ends with '/' the operation will fail
//    2) the destination path is an existing file, the operation
//       will fail.  overwriting doesn't occur implicitly.
//    3) the destination path is a directory, the source file will
//       be copied to the destination directory with filename
//       preserved.
//    4) the destination path has a non-existent basename who's
//       parent is a directory, the source file will be copied to
//       the new path and the original filename will be lost.
//
void
FileCopyTest::sourceFile()
{
    // case 4
    bfs::path src = m_testSourceDir / "foo.txt";
    bfs::path dst = m_testDestDir / "foo.txt";
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    CPPUNIT_ASSERT(bpf::pathExists(dst));

    // case 2
    CPPUNIT_ASSERT(!bpf::safeCopy(src, dst));

    // case 1
    src = src.string() + "/";
    dst /= "unique";
    CPPUNIT_ASSERT(!bpf::safeCopy(src, dst));

    // case 4:  a bit redunant, but ensure renaming works taking
    // the basename from the destination as the new filename
    src = m_testSourceDir / "foo.txt";
    dst = m_testDestDir / "bar.txt";
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    CPPUNIT_ASSERT(bpf::pathExists(dst));

    // case 3
    src = m_testSourceDir / "foo.txt";
    dst = m_testDestDir / "testDir1";
    bfs::create_directories(dst);
    CPPUNIT_ASSERT(bpf::isDirectory(dst));
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    dst /= "foo.txt";
    CPPUNIT_ASSERT(bpf::pathExists(dst));

    // trailing / on dst dir shouldn't affect operation
    src = m_testSourceDir / "foo.txt";
    dst = m_testDestDir / "testDir2";
    CPPUNIT_ASSERT(bfs::create_directories(dst));
    dst = dst.string() + "/";
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    dst /= "foo.txt";
    CPPUNIT_ASSERT(bpf::pathExists(dst));

    // source is a file, destination dir and it's parent don't exist (fail)
    dst = m_testDestDir / "doesnt" / "exist";
    CPPUNIT_ASSERT(!bpf::safeCopy(src, dst));
}


// from bpfile.h
//  if the source path is a directory...
//    1) the destination path is an existing file the operation
//       will fail.
//    2) the destination path is an existing directory, the source
//       path will be recursively copied into the destination
//       directory, creating a new directory named with the basename
//       of source
//    3) the destination path has a non-existent basename who's
//       parent is a directory, the source dir will be copied to
//       the new path and the original basename will be lost.
//
void
FileCopyTest::sourceDir()
{
    // case 1: first try to bpf::safeCopy a directory hierarchy into a file (fail)
    bfs::path src = m_testSourceDir / "fooDir";
    bfs::path dst = m_testDestDir / "exists.txt";

    // make the file
    CPPUNIT_ASSERT(bp::strutil::storeToFile(dst, "this is a test file\n"));

    // ensure the bpf::safeCopy fails
    CPPUNIT_ASSERT(!bpf::safeCopy(src, dst));

    // case 3: now obliterate the target file and ensure that the bpf::safeCopy succeeds
    CPPUNIT_ASSERT(bpf::safeRemove(dst));
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    CPPUNIT_ASSERT(dirsAreSame(src, dst));    

    // ensure a bpf::safeCopy works with a trailing slash on source and destination
    dst = m_testDestDir / "dirCopyTest2";
    dst = dst.string() + "/";
    src = src.string() + "/";
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    CPPUNIT_ASSERT(dirsAreSame(src, dst));    

    // case 2: test copying source dir into an existing dest dir
    src = m_testSourceDir / "fooDir";
    dst = m_testDestDir / "dirCopyTest3";    

    bfs::create_directories(dst);
    CPPUNIT_ASSERT(bpf::isDirectory(dst));
    CPPUNIT_ASSERT(bpf::safeCopy(src, dst));
    dst /= "fooDir";        
    CPPUNIT_ASSERT(dirsAreSame(src, dst));    

    // copying to non-existent path should fail
    dst = m_testDestDir / "doesnt" / "exist";
    CPPUNIT_ASSERT(!bpf::safeCopy(src, dst));
}


void
FileCopyTest::sourceDNE()
{
    // first try to bpf::safeCopy a directory hierarchy into a file (fail)
    bfs::path src = m_testSourceDir / "yerMomma.DNE";
    bfs::path dst = m_testDestDir;

    // ensure the bpf::safeCopy fails
    CPPUNIT_ASSERT(!bpf::safeCopy(src, dst));
}


bool
FileCopyTest::dirsAreSame(bfs::path lhs, bfs::path rhs)
{
    CPPUNIT_ASSERT(bpf::isDirectory(lhs));
    CPPUNIT_ASSERT(bpf::isDirectory(rhs));

    // get contents of lhs relative to lhs
    std::vector<bfs::path> lhsKids;
    bfs::recursive_directory_iterator end;
    for (bfs::recursive_directory_iterator it(lhs); it != end; ++it) {
        lhsKids.push_back(bpf::relativeTo(it->path(), lhs));
    }

    // get contents of rhs relative to rhs
    std::vector<bfs::path> rhsKids;
    for (bfs::recursive_directory_iterator it(rhs); it != end; ++it) {
        rhsKids.push_back(bpf::relativeTo(it->path(), rhs));
    }

    CPPUNIT_ASSERT(lhsKids.size() == rhsKids.size());

    // build a set of RHS entries, and 
    std::set<bfs::path> rhsSet;
    rhsSet.insert(rhsKids.begin(), rhsKids.end());

    for (unsigned int i = 0; i < lhsKids.size(); i++) {
        std::set<bfs::path>::iterator it;
        it = rhsSet.find(lhsKids[i]);
        CPPUNIT_ASSERT(it != rhsSet.end());

        // build up absolute paths for the two
        bfs::path src = lhs / lhsKids[i];
        bfs::path dst = rhs / *it;

        // both should be of same type, craft a nice
        // message for failure
        std::stringstream ss;
        ss << "paths should be of the same type: " << std::endl
           << src << std::endl
           << dst << std::endl;
        CPPUNIT_ASSERT_MESSAGE(ss.str().c_str(), 
            bfs::status(src).type() == bfs::status(dst).type());

        // erase this entry from the rhs set
        rhsSet.erase(it);
    }

    // should be 1 to 1 lhs to rhs
    CPPUNIT_ASSERT( rhsSet.size() == 0 );

    return true;
}


void
FileCopyTest::setUp()
{
    // now create our test source directory and our target destination
    // directory, both which will be deleted upon test teardown
    // <m_testSourceDir>/
    // <m_testSourceDir>/foo.txt
    //
    // <m_testDestDir>/

    bfs::path src = bpf::getTempDirectory();
    bfs::path dst = bpf::getTempPath(src, "FileCopyTest");
    src = bpf::getTempPath(src, "FileCopyTest");

    // now src and dst have dirnames.  create them anew
    CPPUNIT_ASSERT(bpf::safeRemove(src));
    CPPUNIT_ASSERT(bpf::safeRemove(dst));
    bfs::create_directories(src);
    bfs::create_directories(dst);
    CPPUNIT_ASSERT(bpf::isDirectory(src));
    CPPUNIT_ASSERT(bpf::isDirectory(dst));

    m_testSourceDir = src;
    m_testDestDir = dst;    

    // now let's build up the test directory structure
    // m_testSourceDir/foo.txt  
    src /= "foo.txt";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(src, "this is a test file\n"));

    // m_testSourceDir/fooDir/bar/baz
    src = m_testSourceDir / "fooDir" / "bar" / "baz";
    CPPUNIT_ASSERT(bfs::create_directories(src));
    
    // m_testSourceDir/fooDir/bar/baz/fiddle.txt 
    bfs::path baz = src / "fiddle.txt";
    CPPUNIT_ASSERT( bp::strutil::storeToFile(baz, "this is fiddle\n") );    
    // m_testSourceDir/fooDir/bar/baz/faddle.txt 
    baz = src / "faddle.txt";
    CPPUNIT_ASSERT( bp::strutil::storeToFile(baz, "this is faddle\n") );    

    // m_testSourceDir/fooDir/ooga.txt
    src = m_testSourceDir / "fooDir";
    baz = src / "ooga.txt";
    CPPUNIT_ASSERT( bp::strutil::storeToFile(baz, "this is ooga\n") );    

    baz = src / "booga.txt";
    CPPUNIT_ASSERT( bp::strutil::storeToFile(baz, "this is booga\n") );    
}


void
FileCopyTest::tearDown()
{
    CPPUNIT_ASSERT(bpf::safeRemove(m_testSourceDir));
    CPPUNIT_ASSERT(bpf::safeRemove(m_testDestDir));
}

