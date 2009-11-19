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

#include "MimeTypeTest.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpmimetype.h"
#include "BPUtils/bpstrutil.h"


CPPUNIT_TEST_SUITE_REGISTRATION(MimeTypeTest);

void
MimeTypeTest::goodExt()
{
    bp::file::Path p("/Users/foo/bar.jpe");
    std::set<std::string> t = bp::mimetype::fromPath(p);
    CPPUNIT_ASSERT(t.count("image/jpeg") > 0); 
    p = "/Users/foo/bar.jpeg";
    t = bp::mimetype::fromPath(p);
    CPPUNIT_ASSERT(t.count("image/jpeg") > 0);
    p = "/Users/foo/bar.jpeg";
    t = bp::mimetype::fromPath(p);
    CPPUNIT_ASSERT(t.count("image/jpeg") > 0);
    p = "/Users/foo/bar.jPeG";
    t = bp::mimetype::fromPath(p);
    CPPUNIT_ASSERT(t.count("image/jpeg") > 0);
}


void
MimeTypeTest::badExt()
{
    bp::file::Path p("/Users/foo/bar.wtf");
    std::set<std::string> t = bp::mimetype::fromPath(p);
    CPPUNIT_ASSERT(t.count("application/unknown") > 0); 
}


void
MimeTypeTest::noExt()
{
    bp::file::Path p("/Users/foo/bar");
    std::set<std::string> t = bp::mimetype::fromPath(p);
    CPPUNIT_ASSERT(t.count("application/unknown") > 0); 
}


void
MimeTypeTest::goodType()
{
    std::vector<std::string> ext = bp::mimetype::extensionsFromMimeType("image/jpeg");
    CPPUNIT_ASSERT(ext.size() == 5);
    CPPUNIT_ASSERT(ext[0].compare("jfif") == 0);
    CPPUNIT_ASSERT(ext[1].compare("jfif-tbnl") == 0);
    CPPUNIT_ASSERT(ext[2].compare("jpe") == 0);
    CPPUNIT_ASSERT(ext[3].compare("jpeg") == 0);
    CPPUNIT_ASSERT(ext[4].compare("jpg") == 0);
}


void
MimeTypeTest::badType()
{
    std::vector<std::string> ext = bp::mimetype::extensionsFromMimeType("i-am-bogus");
    CPPUNIT_ASSERT(ext.size() == 0);
}


void
MimeTypeTest::noType()
{
    std::vector<std::string> ext = bp::mimetype::extensionsFromMimeType("");
    CPPUNIT_ASSERT(ext.size() == 0);
}


void 
MimeTypeTest::chaseLink()
{
    bp::file::Path dir = bp::file::getTempPath(bp::file::getTempDirectory(), "MimeTypeTest");
    CPPUNIT_ASSERT(bp::file::remove(dir));
    CPPUNIT_ASSERT(boost::filesystem::create_directories(dir));

    bp::file::Path linkPath = dir / "myLink";

    // link to jpeg should be image/jpeg
    bp::file::Path path = dir / "foo.jpg";
    bp::strutil::storeToFile(path, "hello world");
    bp::file::createLink(linkPath, path);
    std::set<std::string> t = bp::mimetype::fromPath(linkPath);
    CPPUNIT_ASSERT(t.count("image/jpeg") > 0);

    // link to folder should be application/x-folder
    bp::file::Path newDir = dir / "myDir";
    boost::filesystem::create_directories(newDir);
    bp::file::remove(linkPath);
    bp::file::createLink(linkPath, newDir);
    t = bp::mimetype::fromPath(linkPath);
    CPPUNIT_ASSERT(t.count("application/x-folder") > 0); 
    bp::file::remove(dir);
}
