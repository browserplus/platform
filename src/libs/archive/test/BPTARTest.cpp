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
 * BPTARTest.cpp
 * Unit tests for round trip creation and extraction of browserplus
 * packaging format
 *
 * Created by Lloyd Hilaiel on 2/11/09.
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "BPTARTest.h"
#include <sstream>
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"

namespace bpf = bp::file;
namespace bfs = boost::filesystem;

// test string
static std::string s_testString("Did I survive the round trip?");

CPPUNIT_TEST_SUITE_REGISTRATION(BPTARTest);

static std::string s_singleFileContents("this is a test file.  It's"
                                        "my friend.\n\n");

// create a directory and hierarchy under it with test data files
static
bool createDirectoryTestData(const bpf::Path & dirPath)
{
    // create the directory
    if (!bfs::create_directories(dirPath)) {
        return false;
    }
    
    // create a single file underneath
    bpf::Path singleFilePath = dirPath / "singlefile";
    if (!bp::strutil::storeToFile(singleFilePath, s_singleFileContents)) {
        return false;
    }

    // create a read only file
    bpf::Path readOnlyFilePath = dirPath / "readonly.foo";
    if (!bp::strutil::storeToFile(readOnlyFilePath, std::string("foo"))) {
        return false;
    }

    // toggle bits to make this thing read only
    if (!bp::file::makeReadOnly(readOnlyFilePath)) {
        return false;
    }

    // create a nested file
    bpf::Path nestedFilePath = dirPath/"levelone"/"leveltwo"/"levelthree";
    if (!bfs::create_directories(nestedFilePath)) {
        return false;
    }
    nestedFilePath /= "thefile.txt";
    if (!bp::strutil::storeToFile(nestedFilePath, std::string("bar"))) {
        return false;
    }
        
    return true;
}

// validate that a directory hierarchy contains data properly generated
// by createTestData
static
bool verifyDirectoryTestData(const bpf::Path & dirPath)
{
    if (!bfs::is_directory(dirPath)) return false;

    // verify existence and contents of top level file
    {
        bpf::Path singleFilePath = dirPath / "singlefile";
        std::string fileContents;
        if (!bp::strutil::loadFromFile(singleFilePath, fileContents)) {
            return false;
        }
        if (0 != fileContents.compare(s_singleFileContents)) {
            return false;
        }
    }

    // verify existence and perms on  read only file
    {
        bpf::Path readOnlyFilePath = dirPath / "readonly.foo";

        bp::file::FileInfo fi;
        if (!bp::file::statFile(readOnlyFilePath, fi)) {
            return false;
        }
        
        if (fi.mode & 0200 || fi.mode & 020 || fi.mode & 02) {
            return false;
        }
    }

    // verify existence of nested file
    {
        bpf::Path nestedFilePath = dirPath/"levelone"/"leveltwo"/"levelthree"/"thefile.txt";

        if (!bpf::exists(nestedFilePath)) {
            return false;
        }
    }
    
    return true;
}

static
bool createFileTestData(const bpf::Path & filePath)
{
    return bp::strutil::storeToFile(filePath, s_testString);
}


void 
BPTARTest::testDirectoryRoundTrip()
{
    CPPUNIT_ASSERT(verifyDirectoryTestData(m_testDirPath));    

    // now let's create a tarfile of the test data
    {
        bp::tar::Create tc;
        CPPUNIT_ASSERT(tc.open(m_tarPath));

        bpf::tRecursiveDirIter end;
        for (bpf::tRecursiveDirIter it(m_testDirPath); it != end; ++it) 
        {
            bpf::Path rel = bpf::Path(it->path()).relativeTo(m_testDirPath);
            CPPUNIT_ASSERT(tc.addFile(bpf::Path(*it), rel));
        }
        
        CPPUNIT_ASSERT(tc.close());
    }

    // now let's test content enumeration, single file extraction,
    // and finally extract the tarfile
    {
        bp::tar::Extract te;        
        CPPUNIT_ASSERT(te.open(m_tarPath));
        std::vector<bpf::Path> contents = te.enumerateContents();
        bool haveTheFileTxt = false;
        bool haveSinglefile = false;        
        bpf::Path thefile("levelone/leveltwo/levelthree/thefile.txt");
        
        for (unsigned int i = 0; i < contents.size(); i++) {
            if (thefile == contents[i]) 
            {
                haveTheFileTxt = true;
                continue;
            }

            if (bpf::Path("singlefile") == contents[i]) 
            {
                haveSinglefile = true;
                continue;
            }
        }
        CPPUNIT_ASSERT(haveTheFileTxt);
        CPPUNIT_ASSERT(haveSinglefile);

        // now let's extract the contents of something
        std::stringstream ss;
        CPPUNIT_ASSERT(te.extractSingle(thefile, ss));
        CPPUNIT_ASSERT(!ss.str().compare(std::string("bar")));

        // and extract everything
        CPPUNIT_ASSERT(te.extract(m_unpackPath));
    }

    // validate we got out what we put in
    CPPUNIT_ASSERT(verifyDirectoryTestData(m_unpackPath));        
}

void
BPTARTest::setUp()
{
    m_testDirPath = bp::file::getTempPath(bp::file::getTempDirectory(), "BPTAR")/"не_е_англиски";
    
    m_testFilePath =
        bp::file::getTempPath(bp::file::getTempDirectory(), "BPTAR_file");

    (void) createDirectoryTestData(m_testDirPath);
    (void) createFileTestData(m_testFilePath);
    
    m_tarPath = bp::file::getTempPath(bp::file::getTempDirectory(), "BPTAR");
    m_tarPath.replace_extension(bp::pkg::extension());

    m_unpackPath = 
        bp::file::getTempPath(bp::file::getTempDirectory(), "BPTAR");
}

void
BPTARTest::tearDown()
{
    (void) bp::file::remove(m_testDirPath.parent_path());
    (void) bp::file::remove(m_testFilePath);
    (void) bp::file::remove(m_tarPath);
    (void) bp::file::remove(m_unpackPath);
}

