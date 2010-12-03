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
    static Signer* get(const boost::filesystem::path& publicKeyPath = boost::filesystem::path());
    
    // sign a file and return the signature
    std::string getSignature(const boost::filesystem::path& privateKeyPath,
                             const boost::filesystem::path& publicKeyPath,
                             const std::string& password,
                             const boost::filesystem::path& inFile);

    // verify signature with content in a file
    bool verifyFile(const std::string& signature,
                    const boost::filesystem::path& contentPath,
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
    
    Signer(const boost::filesystem::path& publicKeyPath); // throw(SSLException)
    ~Signer();
    void createStore(); // throw(SSLException)
    void readSignerFile();
    bool doSign(const boost::filesystem::path& keyFile, 
                const boost::filesystem::path& certFile,
                const std::string& password,
                const boost::filesystem::path& inFile,
                BIO* outBio,
                bool detached);
    bool doVerify(BIO* contentBio,
                  const std::string& signature,
                  BPTime& timestamp);
    bool getTimestamp(PKCS7* pkcs7,
                      BPTime& t);
    static int verify_callback(int ok, 
                               X509_STORE_CTX* store);
        
    boost::filesystem::path m_publicKeyPath;
    X509_STORE* m_store;
    STACK_OF(X509)* m_certStack;
    static std::map<boost::filesystem::path, Signer*> s_singletons;
};

}};
#endif
