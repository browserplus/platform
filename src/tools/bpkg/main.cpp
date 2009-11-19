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


#include <string>
#include <iostream>
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/APTArgParse.h"
#include "BPUtils/bpfile.h"


static APTArgDefinition s_packArgs[] =
{
    {
        "publicKey", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path to public key to use during signing"
    },
    {
        "privateKey", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path to private key to use during signing"
    },
    {
        "password", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "signing password."
    },
    {
        "in", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path to a directory or file that will be packages."
    },
    {
        "out", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path where the .bpkg file should be placed (by convention, this"
        " should have a .bpkg extension.  This is a .tar.lz (lzip) file).  If"
        " not provided, output file will be named 'content.bpkg')"
    }
};

static APTArgDefinition s_unpackArgs[] =
{
    {
        "publicKey", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path to public key to use during signing"
    },
    {
        "in", APT::TAKES_ARG, APT::NO_DEFAULT, APT::REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path to a .bpkg file to unpack."
    },
    {
        "out", APT::TAKES_ARG, APT::NO_DEFAULT, APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "path where the contents of the bpkg will be placed.  This will "
        "either be a directory or a file depending on the contents of the "
        "bpkg file."
    }
};


int
main(int argc, const char ** argv)
{
    int uArgsRv, pArgsRv;
    APTArgParse packArgParser(" pack <options> (pack a .bpkg file)");
    pArgsRv = packArgParser.parse(
        sizeof(s_packArgs)/sizeof(s_packArgs[0]), s_packArgs,
        (argc-1), (argv+1));
        
    APTArgParse unpackArgParser(" unpack <options> (unpack a .bpkg file)");
    uArgsRv = unpackArgParser.parse(
        sizeof(s_unpackArgs)/sizeof(s_unpackArgs[0]), s_unpackArgs,
        (argc-1), (argv+1));

    if (argc < 2 ||
        (std::string("pack").compare(argv[1]) &&
         std::string("unpack").compare(argv[1])))
    {
        // second argument was bogus printf out both pack and unpack usage  
        std::cerr << packArgParser.usage();
        std::cerr << unpackArgParser.usage();        
        return 1;
    }
    else if (!std::string("pack").compare(argv[1]))
    {
        if (pArgsRv < 0 || pArgsRv != argc-1) {
            std::cerr << packArgParser.error();
            return 1;
        }
        
        bp::file::Path inPath(packArgParser.argument("in"));
        bp::file::Path outPath(packArgParser.argument("out"));
        if (outPath.empty()) outPath = "contents.bpkg";
        bp::file::Path publicKey(packArgParser.argument("publicKey"));
        bp::file::Path privateKey(packArgParser.argument("privateKey"));

        if (boost::filesystem::is_regular(packArgParser.argument("in"))) 
        {
            if (!bp::pkg::packFile(privateKey, publicKey, 
                                   packArgParser.argument("password"), 
                                   inPath, outPath))
            {
                std::cerr << "failed to pack" << std::endl;
                return 1;
            }
        }
        else if (boost::filesystem::is_directory(inPath))
        {
            if (!bp::pkg::packDirectory(privateKey, publicKey,
                                        packArgParser.argument("password"),
                                        inPath, outPath))
            {
                std::cerr << "failed to pack" << std::endl;
                return 1;
            }
        }
        else
        {
            std::cerr << "input path doesn't exist: "
                      << inPath << std::endl;
            return 1;
        }
    }
    else if (!std::string("unpack").compare(argv[1]))
    {
        if (uArgsRv < 0 || uArgsRv != argc-1) {
            std::cerr << unpackArgParser.error();
            return 1;
        }

        bp::file::Path inPath(packArgParser.argument("in"));
        bp::file::Path outPath(packArgParser.argument("out"));
        if (outPath.empty()) outPath = "contents";
        bp::file::Path publicKey(packArgParser.argument("publicKey"));

        std::string ign;
        BPTime timestamp;
        if (!bp::pkg::unpackToFile(inPath, outPath, timestamp, ign, publicKey))
        {
            if (!bp::pkg::unpackToDirectory(inPath, outPath, timestamp,
                                            ign, publicKey))
            {
                std::cerr << "failed to unpack" << std::endl;
                return 1;
            }
        }
    }

    return 0;
}
