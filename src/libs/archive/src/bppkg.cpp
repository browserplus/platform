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

/**
 * bpkg.cpp - an abstraction around BrowserPlus packaging format,
 *          Documented in /PackagingFormat.txt
 */

#include "api/bppkg.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include "api/bptar.h"
#include "api/bplzma.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bpstrutil.h"
#include "platform_utils/bpsign.h"

#ifdef WIN32
#pragma warning(disable:4512)
#endif

using namespace std;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;


static void
doPack(const bpf::Path& keyFile, 
       const bpf::Path& certFile,
       const string& password,
       const bpf::Path& contentsFile,
       const bpf::Path& outFile,
       bool isTar)
{
    bpf::Path sigFile;
    bpf::Path pkgtar;
    try {
        try {
            sigFile = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_sig");
            pkgtar = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_tar");
        } catch (const bpf::tFileSystemError& e) {
            throw string("unable to get temp path names: " + string(e.what()));
        }

        // get contentsFile signature and write to file
        //
        bp::sign::Signer* signer = bp::sign::Signer::get(certFile);
        if (!signer) {
            throw string("unable to get signer object");
        }
        string signature = signer->getSignature(keyFile, certFile, 
                                                password, contentsFile);
        if (!bp::strutil::storeToFile(sigFile, signature)) {
            throw string("unable to write signature file: " + sigFile.utf8());
        }

        // now add signature and contents to tarball
        //
        bp::tar::Create tar;
        if (!tar.open(pkgtar)) {
            throw string("unable to open " + pkgtar.utf8());
        }
        bpf::Path fname = isTar ? bp::pkg::contentsPath() 
            : bp::pkg::contentsDataPath();
        if (!tar.addFile(contentsFile, fname)) {
            throw string("unable to add " + contentsFile.utf8());
        }
        if (!tar.addFile(sigFile, bp::pkg::signaturePath())) {
            throw string("unable to add " + sigFile.utf8());
        }
        if (!tar.close()) {
            throw string("unable to close tar file: " + pkgtar.utf8());
        }

        // Now compress that tarball
        //
        bp::lzma::Compress compress;
        ifstream ifs;
        if (!bpf::openReadableStream(ifs, pkgtar, ifstream::binary))
        {
            throw string("unable to open stream for " + pkgtar.utf8());
        }
        ofstream ofs;
        if (!bpf::openWritableStream(ofs, outFile, ofstream::binary | ofstream::trunc))
        {
            throw string("unable to open stream for " + outFile.utf8());
        }
        compress.setInputStream(ifs);
        compress.setOutputStream(ofs);
        if (!compress.run()) {
            throw string("unable to compress " + pkgtar.utf8() 
                         + " to " + outFile.utf8());
        }
        ifs.close();
        ofs.close();
    } catch (const string& msg) {
        (void) remove(sigFile);
        (void) remove(pkgtar);
        throw msg;
    }
    (void) remove(sigFile);
    (void) remove(pkgtar);
}


static void
doUnpack(std::istream & is,
         std::ostream & os,
         const bpf::Path& certPath,
         bool isTar,
         BPTime& timestamp)
{
    // uncompress bpkg
    bp::lzma::Decompress decompress;
    std::stringstream ss;

    decompress.setInputStream(is);
    decompress.setOutputStream(ss);
    if (!decompress.run()) {
        throw string("unable to decompress bpkg");
    }

    // untar result

    // TODO: fix this potentially large memory to memory copy.

    std::stringstream contents, signature;
    {
        bp::tar::Extract untar;
        if (!untar.load(ss.str() /*tarFile*/)) {
            throw string("unable to open tar data");
        }
        // release memory
        ss.clear();

        // extract contents and signature
        bpf::Path contentsPath(isTar ? bp::pkg::contentsPath() 
                            : bp::pkg::contentsDataPath());
        bpf::Path sigFilePath(bp::pkg::signaturePath());

        if (!untar.extractSingle(sigFilePath, signature)) {
            throw string(sigFilePath.utf8() + " file missing");
        }
        if (!untar.extractSingle(contentsPath, contents)) {
            throw string(contentsPath.utf8() + " file missing");
        }
        if (!untar.close()) {
            throw string("unable to close untar session");
        }

        // release tar memory with explicit scope closing
    }
    // now let's validate the signature
    bp::sign::Signer* signer = bp::sign::Signer::get(certPath);
    if (!signer) {
        throw string("unable to get signer object");
    }

    if (!signer->verifyString(contents.str(), signature.str(), timestamp))
    {
        throw string("signature validation failed");
    }

    // write contents to final destination
    contents.seekg(0, std::ios_base::beg);    
    os << contents.str();
}

bpf::tString
bp::pkg::extension()
{
    return bpf::nativeFromUtf8(".bpkg");
}


bpf::Path 
bp::pkg::contentsPath()
{
    return bpf::Path("contents.tar");
}


bpf::Path 
bp::pkg::contentsDataPath()
{
    return bpf::Path("contents.data");
}
        

bpf::Path 
bp::pkg::signaturePath()
{
    return bpf::Path("signature.mime");
}


bool
bp::pkg::packDirectory(const bpf::Path& keyFile, 
                       const bpf::Path& certFile,
                       const string& password,
                       const bpf::Path& inDir,
                       const bpf::Path& outFile)
{
    class WriteVisitor : virtual public bpf::IVisitor
    {
    public:
        WriteVisitor(bp::tar::Create& tar,
                     const bpf::Path& top) : m_tar(tar), m_top(top) {
        }
        virtual ~WriteVisitor() {
        }
        virtual bpf::IVisitor::tResult visitNode(const bpf::Path& p,
                                                 const bpf::Path& relPath) {
            // we strip away our top-level dir name
            bpf::Path rel = relPath.relativeTo(m_top);
            if (rel.empty()) {
                return bpf::IVisitor::eOk;
            }
            if (!m_tar.addFile(p, rel)) {
                throw string("couldn't add " + p.utf8()
                             + " to tar as " + rel.utf8());
            }
            return bpf::IVisitor::eOk;
        }
    protected:
        bp::tar::Create& m_tar;
        bpf::Path m_top;
    };


    bool rval = true;
    bpf::Path tarFile;
    try {
        // First create tarball of inDir
        //
        if (!bpf::exists(inDir)) {
            throw string(inDir.utf8() + " does not exist");
        }

        try {
            tarFile = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_tarFile");
        } catch (const bpf::tFileSystemError& e) {
            throw string("unable to create temp file: " + string(e.what()));
        }

        bp::tar::Create tar;
        if (!tar.open(tarFile)) {
            throw string("unable to open " + tarFile.utf8());
        }

        // add children to tar
        WriteVisitor visitor(tar, inDir.filename());
        recursiveVisit(inDir, visitor, true);

        if (!tar.close()) {
            throw string("unable to close tar file: " + tarFile.utf8());
        }

        // sign, tar, and compress
        doPack(keyFile, certFile, password, tarFile, outFile, true);
        rval = true;
        
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        remove(outFile);
        rval = false;
    }

    remove(tarFile);
    return rval;
}


bool
bp::pkg::unpackToDirectory(std::istream & bpkgStrm,
                           const bpf::Path& destDir,
                           BPTime& timestamp,
                           std::string& oError,
                           const bpf::Path& certPath)
{
    bp::time::Stopwatch sw;
    sw.start();

    BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") unpacking stream");

    bool rval = true;
    try {
        stringstream contents;

        /* calculate the uncompressed size */
        bpkgStrm.seekg(0, std::ios_base::end);
        long long unsigned int len = bpkgStrm.tellg();
        bpkgStrm.seekg(0, std::ios_base::beg);    

        BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") decompressing "
                        << len << " bytes");
        doUnpack(bpkgStrm, contents, certPath, true, timestamp);

        // untar contents to final destination
        if (!remove(destDir)) {
            throw string("unable to remove " + destDir.utf8());
        }
        try {
            bfs::create_directories(destDir);
        } catch(const bpf::tFileSystemError&) {
            throw string("unable to create " + destDir.utf8());
        }
        BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") untarring "
                        << contents.str().length() << " bytes");
        bp::tar::Extract untar;
        if (!untar.load(contents.str())) {
            throw string("unable to open tar data");
        }
        if (!untar.extract(destDir)) {
            throw string("unable to extract tar file to " + destDir.utf8());
        }
        if (!untar.close()) {
            throw string("unable to close tar session");
        }

        // Whew!
        rval = true;

    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        oError = msg;
        rval = false;
    }

    BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") complete! ");

    return rval;
}


bool
bp::pkg::unpackToDirectory(const bpf::Path& bpkgPath,
                           const bpf::Path& destDir,
                           BPTime& timestamp,
                           string& oError,
                           const bpf::Path& certPath)
{
    ifstream ifs;
    if (!bpf::openReadableStream(ifs, bpkgPath, ifstream::binary))
    {
        oError = std::string("unable to open stream for ") + bpkgPath.utf8();
        return false;
    }

    return unpackToDirectory(ifs, destDir, timestamp, oError, certPath);
}


bool 
bp::pkg::packFile(const bpf::Path& keyFile, 
                  const bpf::Path& certFile,
                  const string& password,
                  const bpf::Path& inFile,
                  const bpf::Path& outFile)
{
    bool rval = true;
    try {
        // sign, tar, and compress
        doPack(keyFile, certFile, password, inFile, outFile, false);
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        remove(outFile);
        rval = false;
    }
    return rval;
}


bool 
bp::pkg::unpackToFile(const bpf::Path& bpkgPath,
                      const bpf::Path& destFile,
                      BPTime& timestamp,
                      string& oError,
                      const bpf::Path& certPath) 
{
    bool rval = true;
    bpf::Path tmpDir;
    try {
        try {
            tmpDir = bpf::getTempPath(bpf::getTempDirectory(), "bpkg");
        } catch(bpf::tFileSystemError& e) {
            throw string("unable to get tmpDir path: " + string(e.what()));
        }
        try {
            bfs::create_directories(tmpDir);
        } catch(bpf::tFileSystemError&) {
            throw string("unable to create temp dir: " + tmpDir.utf8());
        }
        ifstream ifs;
        if (!bpf::openReadableStream(ifs, bpkgPath, ifstream::binary))
        {
            throw string("unable to open stream for " + bpkgPath.utf8());
        }

        ofstream ofs;
        if (!bpf::openWritableStream(ofs, destFile, ifstream::binary))
        {
            throw string("unable to open stream for " + bpkgPath.utf8());
        }

        doUnpack(ifs, ofs, certPath, false, timestamp);
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        oError = msg;
        rval = false;
    }
    remove(tmpDir);
    return rval;
}


bool 
bp::pkg::packString(const bpf::Path& keyFile, 
                    const bpf::Path& certFile,
                    const string& password,
                    const string& inStr,
                    const bpf::Path& outFile)
{
    bool rval = true;
    bpf::Path inFile;
    try {
        try {
            inFile = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_string");
        } catch (const bpf::tFileSystemError& e) {
            throw string("unable to get temp path: " + string(e.what()));
        }
        if (!bp::strutil::storeToFile(inFile, inStr)) {
            throw string("unable to save string to " + inFile.utf8());
        }
        // sign, tar, and compress
        doPack(keyFile, certFile, password, inFile, outFile, false);
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        remove(outFile);
        rval = false;
    }
    remove(inFile);
    return rval;
}


bool 
bp::pkg::unpackToString(const bpf::Path& bpkgPath,
                        string& resultStr,
                        BPTime& timestamp,
                        string& oError,
                        const bpf::Path& certPath)
{
    bool rval = true;
    bpf::Path tmpDir;
    try {
        try {
            tmpDir = bpf::getTempPath(bpf::getTempDirectory(), "bpkg");
        } catch (const bpf::tFileSystemError& e) {
            throw string("unable to get temp path: " + string(e.what()));
        }
        try {
            bfs::create_directories(tmpDir);
        } catch(const bpf::tFileSystemError&) {
            throw string("unable to create temp dir: " + tmpDir.utf8());
        }
        bpf::Path destFile = tmpDir / "contents";

        ifstream ifs;
        if (!bpf::openReadableStream(ifs, bpkgPath, ifstream::binary))
        {
            throw string("unable to open stream for " + bpkgPath.utf8());
        }
        stringstream ostream;
        doUnpack(ifs, ostream, certPath, false, timestamp);
        resultStr = ostream.str();
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        oError = msg;
        rval = false;
    }
    remove(tmpDir);
    return rval;
}

