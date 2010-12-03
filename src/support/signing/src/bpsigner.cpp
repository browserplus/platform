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

/*
 *  bpsigner.cpp
 *
 *  Created by Gordon Durand on 10/12/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
 
// Sign/verify stuff using bp::smime


#include <iostream>
#include <fstream>
#include <vector>

#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "platform_utils/bpsign.h"

using namespace std;
using namespace bp::file;
namespace bfs = boost::filesystem;

void 
usage(const char* s)
{
    bfs::path exe(s);
    fprintf(stderr, "usage: OR %s sign -publicKey=<path> -privateKey=<path> -in=<contentPath> -out=<signaturePath> [-password=<password>]\n",
            exe.filename().c_str());
    fprintf(stderr, "usage: OR %s verify [-certStore=<path>] -in=<contentPath> -signature=<signaturePath>\n",
            exe.filename().c_str());
    exit(-1);
}


int
main(int argc, const char* argv[])
{
    if (argc < 2) {
        usage(argv[0]);
    }

    string cmd(argv[1]);
    bfs::path input;
    bfs::path output;
    bfs::path publicKey;
    bfs::path privateKey;
    bfs::path certStore;
    string password;
    bfs::path signature;
    for (int i = 2; i < argc; i++) {
        vector<string> args = bp::strutil::splitAndTrim(argv[i], "=");
        if (args[0].compare("-in") == 0) {
            input = bfs::path(args[1]);
        } else if (args[0].compare("-out") == 0) {
            output = bfs::path(args[1]);
        } else if (args[0].compare("-publicKey") == 0) {
            publicKey = bfs::path(args[1]);
        } else if (args[0].compare("-privateKey") == 0) {
            privateKey = bfs::path(args[1]);
        } else if (args[0].compare("-certStore") == 0) {
            certStore = bfs::path(args[1]);
        } else if (args[0].compare("-password") == 0) {
            password = args[1];
        } else if (args[0].compare("-signature") == 0) {
            signature = bfs::path(args[1]);
        } else {
            usage(argv[0]);
        }
    }
    
    bp::log::setupLogToConsole(bp::log::levelFromString("debug"));

    bp::sign::Signer* signer = bp::sign::Signer::get(certStore);
    if (!signer) {
        fprintf(stderr, "unable to get Signer!\n");
        exit(-1);
    }

    if (cmd.compare("sign") == 0) {
        if (publicKey.empty() || privateKey.empty() || input.empty() || output.empty()) {
            usage(argv[0]);
        }
        string sig = signer->getSignature(privateKey, publicKey, password, input);
        if (sig.empty()) {
            fprintf(stderr, "signing failed\n");
        } else {
            if (!bp::strutil::storeToFile(output, sig)) {
                BP_THROW("unable to store to file" + output.string());
            }
            fprintf(stderr, "signing succeeded\n");
        }
    } else if (cmd.compare("verify") == 0) {
        if (input.empty() || signature.empty()) {
            usage(argv[0]);
        }
        BPTime signingTime;
        string sig;
        if (!bp::strutil::loadFromFile(signature, sig)) {
            BP_THROW("unable to read signature path " + signature.string());
        }
        bool ok = signer->verifyFile(sig, input, signingTime);
        fprintf(stderr, "verification %s\n", ok ? "succeeded" : "failed");
        fprintf(stderr, "timestamp: %s\n", signingTime.asString().c_str());
    } else {
        usage(argv[0]);
    }
}
