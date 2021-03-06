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
 * Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

#include "FileLinkTest.h"
#include <iostream>
#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"

using namespace std;
using namespace bp::file;
namespace bfs = boost::filesystem;


CPPUNIT_TEST_SUITE_REGISTRATION(FileLinkTest);

void 
FileLinkTest::createLink()
{
	// create target file 
	bfs::path target = m_dir / "target.txt";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(target, "this is a target file\n"));

	// create link to target
	bfs::path src = m_dir / "link_to_target.lnk";
    CPPUNIT_ASSERT(bp::file::createLink(src, target));

    // ensure that link resolves to target 
    bfs::path resolved;
    CPPUNIT_ASSERT(bp::file::resolveLink(src, resolved));
    CPPUNIT_ASSERT(boost::filesystem::equivalent(resolved, target));
}


void
FileLinkTest::brokenLink()
{
    bfs::path target = m_dir / "non_existent";
    bfs::path src = m_dir / "broken_link.lnk";
    bp::file::createLink(src, target);
    bfs::path resolved;
    bool pathExists = resolveLink(src, resolved);
    CPPUNIT_ASSERT_MESSAGE(resolved.string().c_str(), !pathExists && resolved.empty());
}


void 
FileLinkTest::circularLink()
{
    // make dir1 containing file1
    bfs::path dir1 = m_dir / "dir1";
    bfs::create_directory(dir1);
    bfs::path file1 = dir1 / "file1";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(file1, "I am file 1"));
    CPPUNIT_ASSERT(bp::file::pathExists(file1));
    
    // make dir2 containing dir2
    bfs::path dir2 = m_dir / "dir2";
    bfs::create_directory(dir2);
    bfs::path file2 =dir2 / "file2";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(file2, "I am file 2"));
    CPPUNIT_ASSERT(bp::file::pathExists(file2));
    
    // now make links from dir1 to dir2 and dir2 to dir1
    bfs::path link1 = dir1 / "link1.lnk";
    bp::file::createLink(link1, dir2);
    CPPUNIT_ASSERT(isLink(link1));
    bfs::path link2 = dir2 / "link2.lnk";
    bp::file::createLink(link2, dir1);
    CPPUNIT_ASSERT(isLink(link2));
    
    // make sure that a recursive visitor will detect this evil
    size_t limit = 6;
    class MyVisitor : virtual public IVisitor {
    public:
        MyVisitor(size_t limit) : m_numChecked(0), m_limit(limit) {}
        virtual ~MyVisitor() {}
        virtual tResult visitNode(const bfs::path& p,
                                  const bfs::path& /*relativePath*/) {
            if (m_numChecked >= m_limit) {
                return eStop;
            }
            m_numChecked++;
            return eOk;
        }
        size_t m_numChecked;
        size_t m_limit;
    };
    MyVisitor v(limit);
    CPPUNIT_ASSERT(recursiveVisit(dir1, v, true));
    CPPUNIT_ASSERT(v.m_numChecked < limit);
}


void 
FileLinkTest::setUp()
{
	m_dir = getTempPath(getTempDirectory(), "FileLinkTest");
    CPPUNIT_ASSERT(safeRemove(m_dir));
	CPPUNIT_ASSERT(bfs::create_directories(m_dir));
}


void 
FileLinkTest::tearDown()
{
    CPPUNIT_ASSERT(safeRemove(m_dir));
}

