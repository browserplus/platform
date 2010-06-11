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

#include <algorithm>
#include "MimeTypeTest.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"


CPPUNIT_TEST_SUITE_REGISTRATION(MimeTypeTest);

void
MimeTypeTest::goodExt()
{
    bp::file::Path p("/Users/foo/bar.jpe");
    std::vector<std::string> t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "image/jpeg") > 0);
    p = "/Users/foo/bar.jpeg";
    t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "image/jpeg") > 0);
    p = "/Users/foo/bar.jpeg";
    t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "image/jpeg") > 0);
    p = "/Users/foo/bar.jPeG";
    t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "image/jpeg") > 0);
}


void
MimeTypeTest::badExt()
{
    bp::file::Path p("/Users/foo/bar.wtf");
    std::vector<std::string> t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "application/unknown") > 0);
}


void
MimeTypeTest::noExt()
{
    bp::file::Path p("/Users/foo/bar");
    std::vector<std::string> t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "application/unknown") > 0);
}


void
MimeTypeTest::goodType()
{
    std::vector<std::string> ext = bp::file::extensionsFromMimeType("image/jpeg");
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
    std::vector<std::string> ext = bp::file::extensionsFromMimeType("i-am-bogus");
    CPPUNIT_ASSERT(ext.size() == 0);
}


void
MimeTypeTest::noType()
{
    std::vector<std::string> ext = bp::file::extensionsFromMimeType("");
    CPPUNIT_ASSERT(ext.size() == 0);
}


void 
MimeTypeTest::chaseLink()
{
    bp::file::Path dir = bp::file::getTempPath(bp::file::getTempDirectory(), "MimeTypeTest");
    CPPUNIT_ASSERT(bp::file::remove(dir));
    CPPUNIT_ASSERT(boost::filesystem::create_directories(dir));

    bp::file::Path linkPath = dir / "myLink.lnk";

    // link to jpeg should be application/x-link,
    // chasing link should give image/jpeg
    bp::file::Path path = dir / "foo.jpg";
    CPPUNIT_ASSERT(bp::strutil::storeToFile(path, "hello world"));
    CPPUNIT_ASSERT(bp::file::createLink(linkPath, path));
    std::vector<std::string> t = bp::file::mimeTypes(linkPath);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), bp::file::kLinkMimeType) > 0);
    bp::file::Path resolved;
    CPPUNIT_ASSERT(bp::file::resolveLink(linkPath, resolved));
    CPPUNIT_ASSERT(boost::filesystem::equivalent(resolved, path));
    t = bp::file::mimeTypes(resolved);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), "image/jpeg") > 0);
    
    // link to folder should be application/x-link,
    // chasing link should give application/x-folder
    bp::file::Path newDir = dir / "myDir";
    boost::filesystem::create_directories(newDir);
    CPPUNIT_ASSERT(bp::file::remove(linkPath));
    CPPUNIT_ASSERT(bp::file::createLink(linkPath, newDir));
    t = bp::file::mimeTypes(linkPath);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), bp::file::kLinkMimeType) > 0);
    CPPUNIT_ASSERT(bp::file::resolveLink(linkPath, resolved));
    CPPUNIT_ASSERT(boost::filesystem::equivalent(resolved, newDir));
    t = bp::file::mimeTypes(resolved);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), bp::file::kFolderMimeType) > 0);
    
    // broken link should be application/x-badlink
    bp::file::remove(linkPath);
    CPPUNIT_ASSERT(bp::file::createLink(linkPath, dir/"i_do_not_exist"));
    t = bp::file::mimeTypes(linkPath);
    CPPUNIT_ASSERT(count(t.begin(), t.end(), bp::file::kBadLinkMimeType) > 0);
    
    bp::file::remove(dir);
}


void
MimeTypeTest::registeredType()
{
    bp::file::Path p("/Users/foo/foo.xls");
    std::vector<std::string> t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(t[0] == "application/vnd.ms-excel");
    p = "/Users/foo/foo.ppt";
    t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(t[0] == "application/vnd.ms-powerpoint");
    p = "/Users/foo/foo.pcl";
    t = bp::file::mimeTypes(p);
    CPPUNIT_ASSERT(t[0] == "application/vnd.hp-pcl");
}

