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
doPack(const bfs::path& keyFile, 
       const bfs::path& certFile,
       const string& password,
       const bfs::path& contentsFile,
       const bfs::path& outFile,
       bool isTar)
{
    bfs::path sigFile;
    bfs::path pkgtar;
    try {
        try {
            sigFile = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_sig");
            pkgtar = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_tar");
        } catch (const bfs::filesystem_error& e) {
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
            throw string("unable to write signature file: " + sigFile.string());
        }

        // now add signature and contents to tarball
        //
        bp::tar::Create tar;
        if (!tar.open(pkgtar)) {
            throw string("unable to open " + pkgtar.string());
        }
        bfs::path fname = isTar ? bp::pkg::contentsPath() 
                                : bp::pkg::contentsDataPath();
        if (!tar.addFile(contentsFile, fname)) {
            throw string("unable to add " + contentsFile.string());
        }
        if (!tar.addFile(sigFile, bp::pkg::signaturePath())) {
            throw string("unable to add " + sigFile.string());
        }
        if (!tar.close()) {
            throw string("unable to close tar file: " + pkgtar.string());
        }

        // Now compress that tarball
        //
        bp::lzma::Compress compress;
        ifstream ifs;
        if (!bpf::openReadableStream(ifs, pkgtar, ifstream::binary)) {
            throw string("unable to open stream for " + pkgtar.string());
        }
        ofstream ofs;
        if (!bpf::openWritableStream(ofs, outFile, ofstream::binary | ofstream::trunc))  {
            throw string("unable to open stream for " + outFile.string());
        }
        compress.setInputStream(ifs);
        compress.setOutputStream(ofs);
        if (!compress.run()) {
            throw string("unable to compress " + pkgtar.string()
                         + " to " + outFile.string());
        }
        ifs.close();
        ofs.close();
    } catch (const string& msg) {
        (void) bpf::safeRemove(sigFile);
        (void) bpf::safeRemove(pkgtar);
        throw msg;
    }
    (void) bpf::safeRemove(sigFile);
    (void) bpf::safeRemove(pkgtar);
}


static void
doUnpack(istream & is,
         ostream & os,
         const bfs::path& certPath,
         bool isTar,
         BPTime& timestamp)
{
    // uncompress bpkg
    bp::lzma::Decompress decompress;
    stringstream ss;

    decompress.setInputStream(is);
    decompress.setOutputStream(ss);
    if (!decompress.run()) {
        throw string("unable to decompress bpkg");
    }

    // untar result

    // TODO: fix this potentially large memory to memory copy.

    stringstream contents, signature;
    {
        bp::tar::Extract untar;
        if (!untar.load(ss.str() /*tarFile*/)) {
            throw string("unable to open tar data");
        }
        // release memory
        ss.clear();

        // extract contents and signature
        bfs::path contentsPath(isTar ? bp::pkg::contentsPath() 
                                     : bp::pkg::contentsDataPath());
        bfs::path sigFilePath(bp::pkg::signaturePath());

        if (!untar.extractSingle(sigFilePath, signature)) {
            throw string(sigFilePath.string() + " file missing");
        }
        if (!untar.extractSingle(contentsPath, contents)) {
            throw string(contentsPath.string() + " file missing");
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

    if (!signer->verifyString(contents.str(), signature.str(), timestamp)) {
        throw string("signature validation failed");
    }

    // write contents to final destination
    contents.seekg(0, ios_base::beg);    
    os << contents.str();
}

string
bp::pkg::extension()
{
    return string(".bpkg");
}


bfs::path 
bp::pkg::contentsPath()
{
    return bfs::path("contents.tar");
}


bfs::path 
bp::pkg::contentsDataPath()
{
    return bfs::path("contents.data");
}
        

bfs::path 
bp::pkg::signaturePath()
{
    return bfs::path("signature.mime");
}


bool
bp::pkg::packDirectory(const bfs::path& keyFile, 
                       const bfs::path& certFile,
                       const string& password,
                       const bfs::path& inDir,
                       const bfs::path& outFile)
{
    class WriteVisitor : virtual public bpf::IVisitor
    {
    public:
        WriteVisitor(bp::tar::Create& tar,
                     const bfs::path& top) : m_tar(tar), m_top(top) {
        }
        virtual ~WriteVisitor() {
        }
        virtual bpf::IVisitor::tResult visitNode(const bfs::path& p,
                                                 const bfs::path& relPath) {
            // we strip away our top-level dir name
            bfs::path rel = bpf::relativeTo(relPath, m_top);
            if (rel.empty()) {
                return bpf::IVisitor::eOk;
            }
            if (!m_tar.addFile(p, rel)) {
                throw string("couldn't add " + p.string()
                             + " to tar as " + rel.string());
            }
            return bpf::IVisitor::eOk;
        }
    protected:
        bp::tar::Create& m_tar;
        bfs::path m_top;
    };


    bool rval = true;
    bfs::path tarFile;
    try {
        // First create tarball of inDir
        //
        if (!bpf::pathExists(inDir)) {
            throw string(inDir.string() + " does not exist");
        }

        try {
            tarFile = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_tarFile");
        } catch (const bfs::filesystem_error& e) {
            throw string("unable to create temp file: " + string(e.what()));
        }

        bp::tar::Create tar;
        if (!tar.open(tarFile)) {
            throw string("unable to open " + tarFile.string());
        }

        // add children to tar
        WriteVisitor visitor(tar, inDir.filename());
        recursiveVisit(inDir, visitor, true);

        if (!tar.close()) {
            throw string("unable to close tar file: " + tarFile.string());
        }

        // sign, tar, and compress
        doPack(keyFile, certFile, password, tarFile, outFile, true);
        rval = true;
        
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        bpf::safeRemove(outFile);
        rval = false;
    }

    bpf::safeRemove(tarFile);
    return rval;
}


bool
bp::pkg::unpackToDirectory(istream & bpkgStrm,
                           const bfs::path& destDir,
                           BPTime& timestamp,
                           string& oError,
                           const bfs::path& certPath)
{
    bp::time::Stopwatch sw;
    sw.start();

    BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") unpacking stream");

    bool rval = true;
    try {
        stringstream contents;

        /* calculate the uncompressed size */
        bpkgStrm.seekg(0, ios_base::end);
        long long unsigned int len = bpkgStrm.tellg();
        bpkgStrm.seekg(0, ios_base::beg);    

        BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") decompressing "
                        << len << " bytes");
        doUnpack(bpkgStrm, contents, certPath, true, timestamp);

        // untar contents to final destination
        if (!bpf::safeRemove(destDir)) {
            throw string("unable to remove " + destDir.string());
        }
        try {
            bfs::create_directories(destDir);
        } catch(const bfs::filesystem_error&) {
            throw string("unable to create " + destDir.string());
        }
        BPLOG_INFO_STRM("(" << sw.elapsedSec() << ") untarring "
                        << contents.str().length() << " bytes");
        bp::tar::Extract untar;
        if (!untar.load(contents.str())) {
            throw string("unable to open tar data");
        }
        if (!untar.extract(destDir)) {
            throw string("unable to extract tar file to " + destDir.string());
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
bp::pkg::unpackToDirectory(const bfs::path& bpkgPath,
                           const bfs::path& destDir,
                           BPTime& timestamp,
                           string& oError,
                           const bfs::path& certPath)
{
    ifstream ifs;
    if (!bpf::openReadableStream(ifs, bpkgPath, ifstream::binary)) {
        oError = "unable to open stream for " + bpkgPath.string();
        return false;
    }
    return unpackToDirectory(ifs, destDir, timestamp, oError, certPath);
}


bool 
bp::pkg::packFile(const bfs::path& keyFile, 
                  const bfs::path& certFile,
                  const string& password,
                  const bfs::path& inFile,
                  const bfs::path& outFile)
{
    bool rval = true;
    try {
        // sign, tar, and compress
        doPack(keyFile, certFile, password, inFile, outFile, false);
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        bpf::safeRemove(outFile);
        rval = false;
    }
    return rval;
}


bool 
bp::pkg::unpackToFile(const bfs::path& bpkgPath,
                      const bfs::path& destFile,
                      BPTime& timestamp,
                      string& oError,
                      const bfs::path& certPath) 
{
    bool rval = true;
    bfs::path tmpDir;
    try {
        try {
            tmpDir = bpf::getTempPath(bpf::getTempDirectory(), "bpkg");
        } catch(bfs::filesystem_error& e) {
            throw string("unable to get tmpDir path: " + string(e.what()));
        }
        try {
            bfs::create_directories(tmpDir);
        } catch(bfs::filesystem_error&) {
            throw string("unable to create temp dir: " + tmpDir.string());
        }
        ifstream ifs;
        if (!bpf::openReadableStream(ifs, bpkgPath, ifstream::binary)) {
            throw string("unable to open stream for " + bpkgPath.string());
        }

        ofstream ofs;
        if (!bpf::openWritableStream(ofs, destFile, ifstream::binary)) {
            throw string("unable to open stream for " + bpkgPath.string());
        }

        doUnpack(ifs, ofs, certPath, false, timestamp);
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        oError = msg;
        rval = false;
    }
    bpf::safeRemove(tmpDir);
    return rval;
}


bool 
bp::pkg::packString(const bfs::path& keyFile, 
                    const bfs::path& certFile,
                    const string& password,
                    const string& inStr,
                    const bfs::path& outFile)
{
    bool rval = true;
    bfs::path inFile;
    try {
        try {
            inFile = bpf::getTempPath(bpf::getTempDirectory(), "bpkg_string");
        } catch(bfs::filesystem_error& e) {
            throw string("unable to get temp path: " + string(e.what()));
        }
        if (!bp::strutil::storeToFile(inFile, inStr)) {
            throw string("unable to save string to " + inFile.string());
        }
        // sign, tar, and compress
        doPack(keyFile, certFile, password, inFile, outFile, false);
        rval = true;
    } catch (const string& msg) {
        BPLOG_ERROR(msg);
        bpf::safeRemove(outFile);
        rval = false;
    }
    bpf::safeRemove(inFile);
    return rval;
}


bool 
bp::pkg::unpackToString(const bfs::path& bpkgPath,
                        string& resultStr,
                        BPTime& timestamp,
                        string& oError,
                        const bfs::path& certPath)
{
    bool rval = true;
    bfs::path tmpDir;
    try {
        try {
            tmpDir = bpf::getTempPath(bpf::getTempDirectory(), "bpkg");
        } catch(bfs::filesystem_error& e) {
            throw string("unable to get temp path: " + string(e.what()));
        }
        try {
            bfs::create_directories(tmpDir);
        } catch(bfs::filesystem_error&) {
            throw string("unable to create temp dir: " + tmpDir.string());
        }
        bfs::path destFile = tmpDir / "contents";

        ifstream ifs;
        if (!bpf::openReadableStream(ifs, bpkgPath, ifstream::binary)) {
            throw string("unable to open stream for " + bpkgPath.string());
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
    bpf::safeRemove(tmpDir);
    return rval;
}

