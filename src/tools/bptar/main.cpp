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

#include <string>
#include <iostream>
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpfile.h"


int
main(int argc, char ** argv)
{
    // let's determine the mode, usage is different for creation and
    // extraction modes.
    bool correctUsage = false;
    if (argc == 3 && *argv[1] == 'x') correctUsage = true;
    if (argc >  3 && *argv[1] == 'c') correctUsage = true;

    if (!correctUsage)
    {
        std::cout << "usage: " << argv[0] << " [ x | c ] <filename> "
                  << "[(create) files to include]" << std::endl;
        return 1;
    }
    bp::file::Path fname(argv[2]);
    
    if (*argv[1] == 'x') {
        // extract mode
        bp::tar::Extract ex;
        if (!ex.open(fname)) {
            std::cerr << "couldn't open file: " << fname << std::endl;
            exit(1);
        }

        if (!ex.extract(bp::file::Path("."))) {
            std::cerr << "couldn't extract file: " << fname << std::endl;
            exit(1);
        }

        if (!ex.close()) {
            std::cerr << "couldn't close tar file: " << fname << std::endl;
            exit(1);
        }
        
    } else {
        // create mode
        bp::tar::Create tar;        
        
        // argv[2] is a tarfile we should create, remaining args are
        // files or directories that should be included
        if (!tar.open(fname)) {
            std::cerr << "couldn't create file: " << fname << std::endl;
            exit(1);
        }

        // now add files and directories specified on command line
        for (int i = 3; i < argc; i++) {
            bp::file::Path path(argv[i]);
            std::vector<bp::file::Path> subPaths;
            if (!bp::file::exists(path)) {
                std::cerr << "no such file (skipping): " << path << std::endl;
                continue;
            }
            
            if (boost::filesystem::is_directory(path)) {
                // add the directory entry
                if (!tar.addFile(path, path)) {
                    std::cerr << "couldn't add directory: "
                              << path << std::endl;                    
                    exit(1);
                }
                
                try {
                    bp::file::tRecursiveDirIter end;
                    for (bp::file::tRecursiveDirIter it(path); it != end; ++it) {
                        subPaths.push_back(bp::file::Path(it->path()));
                    }
                } catch (const bp::file::tFileSystemError& e) {
                    std::cerr << "unable to iterate thru " << path.externalUtf8()
                              << ": " << e.what();
                    exit(1);
                }
            
            } else {
                subPaths.push_back(path);
            }

            // now add to tarfile
            for (unsigned int k = 0; k < subPaths.size(); k++) {
                if (!tar.addFile(subPaths[k], subPaths[k])) {
                    std::cerr << "couldn't add file to tar: "
                              << subPaths[k] << std::endl;
                    exit(1);
                }
            }
        }

        if (!tar.close()) {
            std::cerr << "couldn't close tar file: " << fname << std::endl;
            exit(1);
        }
    }

    return 0;
}
