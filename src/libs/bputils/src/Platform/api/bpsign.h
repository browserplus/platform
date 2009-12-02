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

/*
 *  bpsign.h
 *
 *  Created by Gordon Durand on 10/02/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPSIGN_H__
#define __BPSIGN_H__

#include <string>
#include <vector>
#include <set>
#include <map>

#include <openssl/pem.h>
#include <openssl/pkcs7.h>
#include <openssl/err.h>
#include <openssl/x509_vfy.h>
#include <openssl/opensslv.h>

#include "BPUtils/bptime.h"
#include "BPUtils/bpfile.h"

// bp::sign::Signer is a singleton to deal with signing
// Each certFile has a separate singleton.
// An empty certFile uses the platform certificates

namespace bp {
namespace sign {

class Signer {
public:
    static Signer* get(const bp::file::Path& publicKeyPath = bp::file::Path());
    
    // sign a file and return the signature
    std::string getSignature(const bp::file::Path& privateKeyPath,
                             const bp::file::Path& publicKeyPath,
                             const std::string& password,
                             const bp::file::Path& inFile);

    // verify signature with content in a file
    bool verifyFile(const std::string& signature,
                    const bp::file::Path& contentPath,
                    BPTime& timestamp);

    // verify signature with content in a string
    bool verifyString(const std::string& in,
                      const std::string& signature,
                      BPTime& timestamp);
        
private:
    class SSLException : public std::exception {
      public: 
        SSLException(const std::string& str);
        virtual ~SSLException() throw() {};
        virtual const char* what() const throw();
      private:
        std::string m_str;
    };
    
    Signer(const bp::file::Path& publicKeyPath); // throw(SSLException)
    ~Signer();
    void createStore(); // throw(SSLException)
    void readSignerFile();
    bool doSign(const bp::file::Path& keyFile, 
                const bp::file::Path& certFile,
                const std::string& password,
                const bp::file::Path& inFile,
                BIO* outBio,
                bool detached);
    bool doVerify(BIO* contentBio,
                  const std::string& signature,
                  BPTime& timestamp);
    bool getTimestamp(PKCS7* pkcs7,
                      BPTime& t);
    static int verify_callback(int ok, 
                               X509_STORE_CTX* store);
        
    bp::file::Path m_publicKeyPath;
    X509_STORE* m_store;
    STACK_OF(X509)* m_certStack;
    static std::map<bp::file::Path, Signer*> s_singletons;
};

}};
#endif
