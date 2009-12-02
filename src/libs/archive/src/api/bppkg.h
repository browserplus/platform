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
 * bpkg.h - an abstraction around BrowserPlus packaging format,
 *          Documented in /PackagingFormat.txt
 */

#ifndef __BPPKG_H__
#define __BPPKG_H__

#include <string>
#include <vector>
#include "BPUtils/bptime.h"
#include "BPUtils/bpfile.h"

namespace bp { namespace pkg {

        // get the file extension to be used for browserplus package
        // files
        bp::file::tString extension();
        
        // get the content tarball name
        bp::file::Path contentsPath();

        // get the single file content name
        bp::file::Path contentsDataPath();
        
        // get the signature file name
        bp::file::Path signaturePath();
        
        // Given a directory as input (inDir) and a path to write a file
        // to, create a browserplus package.
        // NOTE, by convention the outFile should have bp::pkg::extension()
        // appended to it.
        bool packDirectory(const bp::file::Path& keyFile, 
                           const bp::file::Path& certFile,
                           const std::string& password,
                           const bp::file::Path& inDir,
                           const bp::file::Path& outFile);

        // Given a path to a .bpkg file as input (bpkgPath), validate
        // and extract a signed browserplus package bundle
        // empty certPath uses installed certificate store
        bool unpackToDirectory(const bp::file::Path& bpkgPath,
                               const bp::file::Path& destDir,
                               BPTime& timestamp,
                               std::string & oError,
                               const bp::file::Path& certPath = bp::file::Path());

        // Given an istream to .bpkg data in memory (bpkgStrm), validate
        // and extract a signed browserplus package bundle
        // empty certPath uses installed certificate store
        bool unpackToDirectory(std::istream & bpkgStrm,
                               const bp::file::Path& destDir,
                               BPTime& timestamp,
                               std::string & oError,
                               const bp::file::Path& certPath = bp::file::Path());

        // Given a single file as input (inFile) and a path to write a file
        // to, create a browserplus package.
        // NOTE, by convention the outFile should have bp::pkg::extension()
        // appended to it.
        bool packFile(const bp::file::Path& keyFile, 
                      const bp::file::Path& certFile,
                      const std::string& password,
                      const bp::file::Path& inFile,
                      const bp::file::Path& outFile);

        // Given a path to a .bpkg file as input (bpkgPath), validate
        // and extract a signed browserplus package bundle
        // empty certPath uses installed certificate store
        bool unpackToFile(const bp::file::Path& bpkgPath,
                          const bp::file::Path& destFile,
                          BPTime& timestamp,
                          std::string & oError,
                          const bp::file::Path& certPath = bp::file::Path());

        // Given a string as input (inStr) and a path to write a file
        // to, create a browserplus package.
        // NOTE, by convention the outFile should have bp::pkg::extension()
        // appended to it.
        bool packString(const bp::file::Path& keyFile, 
                        const bp::file::Path& certFile,
                        const std::string& password,
                        const std::string& inStr,
                        const bp::file::Path& outFile);

        // Given a path to a .bpkg file as input (bpkgPath), validate
        // and extract a signed browserplus package bundle into a string
        // empty certPath uses installed certificate store
        bool unpackToString(const bp::file::Path& bpkgPath,
                            std::string& resultStr,
                            BPTime& timestamp,
                            std::string& oError,
                            const bp::file::Path& certPath = bp::file::Path());

}; };

#endif
