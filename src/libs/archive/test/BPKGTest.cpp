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
 * BPKGTest.cpp
 * Unit tests for round trip creation and extraction of browserplus
 * packaging format
 *
 * Created by Lloyd Hilaiel on 2/11/09.
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "BPKGTest.h"
#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"

namespace bpf = bp::file;
namespace bfs = boost::filesystem;

// test string
static std::string s_testString("Did I survive the round trip?");

// test public key
static std::string s_pubKey(
"-----BEGIN CERTIFICATE-----\n"
"MIIGrTCCBJWgAwIBAgIJALkuKC5GWBVhMA0GCSqGSIb3DQEBBQUAMIGVMQswCQYD\n"
"VQQGEwJVUzERMA8GA1UECBMIQ29sb3JhZG8xDzANBgNVBAcTBkRlbnZlcjEPMA0G\n"
"A1UEChQGWWFob28hMRQwEgYDVQQLEwtCcm93c2VyUGx1czEWMBQGA1UEAxMNTGxv\n"
"eWQgSGlsYWllbDEjMCEGCSqGSIb3DQEJARYUbGxveWRoQHlhaG9vLWluYy5jb20w\n"
"HhcNMDkwMjExMjE0MjE5WhcNMTExMTA4MjE0MjE5WjCBlTELMAkGA1UEBhMCVVMx\n"
"ETAPBgNVBAgTCENvbG9yYWRvMQ8wDQYDVQQHEwZEZW52ZXIxDzANBgNVBAoUBllh\n"
"aG9vITEUMBIGA1UECxMLQnJvd3NlclBsdXMxFjAUBgNVBAMTDUxsb3lkIEhpbGFp\n"
"ZWwxIzAhBgkqhkiG9w0BCQEWFGxsb3lkaEB5YWhvby1pbmMuY29tMIICIjANBgkq\n"
"hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA4JTCJRNRzvJTjLhLyVQlrB+AR0A9ltgM\n"
"lWLwxNGcRrLVOtpgpVgCa3EYD7m/DrL5/Iewz6BDHTD7pKOd+KhAiusGm7okWQUe\n"
"8lLrddAt2xk5rVuGJTuwVjJt3sAynxGkyRadu9OLVlUVWVbdy/2ipx+8oP5InUeM\n"
"46M8bpFNX8n9eDgqgsJj7ln984rcHnZEvzgJiuJT4mXDEvYj5sXhQ66MPvGoRFBR\n"
"jfa+3W0CXdKI/zVaIlt8ka+jk8yixTaW47+hN5ULPvcqZBLmFr/UJUu9nVm/CbSV\n"
"Wbk8J8YebmuEu7VItymc0q5goQcqZPpoj3dtg2BZQmxFqBwh5wdUNFEqUUzNFCyj\n"
"bFgCt05y5hMfCPhyct+owg4Mm81MixEl6N88JG7SxMFOPd4Zeuy8RJZtFpB0uw+B\n"
"SeRS0IWa7znRujD8H2J+RwjzynOVixrXbTuhdXHqpar9Rb/OUt837CQxhDoGDPL6\n"
"22HKAjj5931CNQy80kF6bgS19EBocLdmax6KSOq7CK5DV7r40RmB1UlkTYWqP6/s\n"
"lidXf+nz19tjCKZFoxKfBOc/1nO6hj+HiROIF+HPM11ei1uN6hk/dRiFSU26AeF3\n"
"HAeFJ2EJa4um1dDo7p3dX1tEV+pumGHlXxQSuXVQvHW0YxLuqb9Cb9Dmx2xFhb4R\n"
"ix496QwBWC0CAwEAAaOB/TCB+jAdBgNVHQ4EFgQUtWJU681jtf6u/0poRgVcYRNi\n"
"JPgwgcoGA1UdIwSBwjCBv4AUtWJU681jtf6u/0poRgVcYRNiJPihgZukgZgwgZUx\n"
"CzAJBgNVBAYTAlVTMREwDwYDVQQIEwhDb2xvcmFkbzEPMA0GA1UEBxMGRGVudmVy\n"
"MQ8wDQYDVQQKFAZZYWhvbyExFDASBgNVBAsTC0Jyb3dzZXJQbHVzMRYwFAYDVQQD\n"
"Ew1MbG95ZCBIaWxhaWVsMSMwIQYJKoZIhvcNAQkBFhRsbG95ZGhAeWFob28taW5j\n"
"LmNvbYIJALkuKC5GWBVhMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQADggIB\n"
"ALK4zt4485aujfToVmHF6iFQb0gpcWwb92f9oZnQEQeLeR5MKtS2SrMZWUXRCPbU\n"
"FZaRfADUr8N5bLr7T1QgMVbyze5puf5mDj/mf4yxyMXKKJkA7ckfIeE8PWjotXbc\n"
"sud3LIrNrVofUDdSwMef/JHunrxieUhu9hSAVu3bKDs8NLC1qyJSC3NCJ4l5lQ/1\n"
"F1RNkJc136SRdsmhSsoU9zVqYdTkyRI7S1Viy6ooIlWeapC6mz+c6goCJAXYNEh7\n"
"8vpkfUZCvpNYwRAXUgtVobf92yezTeNedplT9GQYZ0ZQAGZE8UkGrvwTX6cPwYMh\n"
"ZVZUYMfPYMBLCMOc1gjfhSA1gVvctX75R7P7fNz5b2L1uG4d5ApoeBOidaGQqtfk\n"
"ShpgqCIWZi584nc2mADJCGEQldypX419L/cgM2Fg8Bk1jq3eZRzZG8tqleaU+BRz\n"
"9yfZ7Pwg70YIKqBojH8N1/bWPpHkIPHvMDSAL07JTXHcLnp6MGiPwycMd+i84TFs\n"
"S8hOOx5W+WUH26UMBGkqgBwjyZh8QEnlJIddZ5L9pKB5m62QeEiMpjS2dzSojw7A\n"
"C0En2KH0C4qQm49Ig6pBWdwJW8sZrTix92JY5H7p3BHO1DzjzNXVGC1UPkMBNNV3\n"
"aXr8558T91UhD7eES6x+QUXq/6klt6iCt4o3HB7xoEaq\n"
"-----END CERTIFICATE-----\n");

// test matching private key
static std::string s_privKey(
"-----BEGIN RSA PRIVATE KEY-----\n"
"Proc-Type: 4,ENCRYPTED\n"
"DEK-Info: DES-EDE3-CBC,02B2F5F611823CC1\n"
"\n"
"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\n"
"-----END RSA PRIVATE KEY-----\n");

// test matching password
static std::string s_testPassword("BiteMeHard");

CPPUNIT_TEST_SUITE_REGISTRATION(BPKGTest);

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
    if (!bpf::makeReadOnly(readOnlyFilePath)) {
        return false;
    }

    // create a nested file
    bpf::Path nestedFilePath = dirPath/"levelone"/"leveltwo"/"levelthree"/"thefile.txt";
    if (!bfs::create_directories(nestedFilePath.parent_path())) {
        return false;
    }
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
    if (!bpf::isDirectory(dirPath)) return false;

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

        bpf::FileInfo fi;
        if (!bpf::statFile(readOnlyFilePath, fi)) {
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


static
bool verifyFileTestData(const bpf::Path & filePath)
{
    std::string str;
    if (!bp::strutil::loadFromFile(filePath, str)) {
        return false;
    }
    return (str.compare(s_testString) == 0);
}

static
bool verifyStringTestData(const std::string & str)
{
    return (str.compare(s_testString) == 0);
}


void BPKGTest::testDirectoryRoundTrip()
{
    CPPUNIT_ASSERT(verifyDirectoryTestData(m_testDirPath));    

    // now let's create a bpkg of the test data
    CPPUNIT_ASSERT(bp::pkg::packDirectory(m_keyFile, m_certFile, s_testPassword,
                                          m_testDirPath, m_bpkgPath));

    // now let's unpack the bpkg of the test data
    std::string err;
    BPTime ts;
    CPPUNIT_ASSERT(bp::pkg::unpackToDirectory(m_bpkgPath, m_unpackPath,
                                              ts, err, m_certFile));

    // validate we got out what we put in
    CPPUNIT_ASSERT(verifyDirectoryTestData(m_unpackPath));        
}


void BPKGTest::testFileRoundTrip()
{
    CPPUNIT_ASSERT(verifyFileTestData(m_testFilePath));    

    // now let's create a bpkg of the test data
    CPPUNIT_ASSERT(bp::pkg::packFile(m_keyFile, m_certFile, s_testPassword,
                                     m_testFilePath, m_bpkgPath));

    // now let's unpack the bpkg of the test data
    std::string err;
    BPTime ts;
    CPPUNIT_ASSERT(bp::pkg::unpackToFile(m_bpkgPath, m_unpackPath,
                                         ts, err, m_certFile));

    // validate we got out what we put in
    CPPUNIT_ASSERT(verifyFileTestData(m_unpackPath));        
}


void BPKGTest::testStringRoundTrip()
{
    CPPUNIT_ASSERT(verifyStringTestData(s_testString));    

    // now let's create a bpkg of the test data
    CPPUNIT_ASSERT(bp::pkg::packString(m_keyFile, m_certFile, s_testPassword,
                                       s_testString, m_bpkgPath));

    // now let's unpack the bpkg of the test data
    std::string err;
    std::string str;
    BPTime ts;
    CPPUNIT_ASSERT(bp::pkg::unpackToString(m_bpkgPath, str,
                                           ts, err, m_certFile));

    // validate we got out what we put in
    CPPUNIT_ASSERT(verifyStringTestData(str));
}


void
BPKGTest::setUp()
{
    m_baseDirPath = bpf::getTempPath(bpf::getTempDirectory(), "BPKG") 
                    / "не_е_англиски";

    m_testDirPath = m_baseDirPath / "testDir";
    
    m_testFilePath = m_baseDirPath / "testFile";

    (void) createDirectoryTestData(m_testDirPath);
    (void) createFileTestData(m_testFilePath);
    
    m_keyFile = bpf::getTempPath(bpf::getTempDirectory(), "BPKG");
    (void) bp::strutil::storeToFile(m_keyFile, s_privKey);

    m_certFile = bpf::getTempPath(bpf::getTempDirectory(), "BPKG");
    (void) bp::strutil::storeToFile(m_certFile, s_pubKey);

    m_bpkgPath = m_baseDirPath / "bpkgTestFile";
    m_bpkgPath.replace_extension(bp::pkg::extension());

    m_unpackPath = m_baseDirPath / "unpackTestDir";
}

void
BPKGTest::tearDown()
{
    (void) bpf::remove(m_testDirPath);
    (void) bpf::remove(m_testFilePath);
    (void) bpf::remove(m_keyFile);
    (void) bpf::remove(m_certFile);
    (void) bpf::remove(m_bpkgPath);
    (void) bpf::remove(m_unpackPath);
    (void) bpf::remove(m_baseDirPath.parent_path());
}

