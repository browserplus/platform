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

/*
 *  bpsign.cpp
 *
 *  Created by Gordon Durand on 10/02/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpsign.h"
#include "ProductPaths.h"
#include "bpfile.h"
#include "bpstrutil.h"
#include "BPLog.h"

#include <map>
#include <iostream>
#include <fstream>

#ifdef WIN32
#define snprintf _snprintf_s
#define sscanf sscanf_s
#endif

using namespace std;
using namespace bp::file;

namespace bp {
namespace sign {

static BIO*
openReadBIOFromPath(const Path & path)
{
    BIO * b = NULL;
#ifdef WIN32
	FILE * f = NULL;
	(void) ::_wfopen_s(&f, path.external_file_string().c_str(), L"rb");
    if (f != NULL) {
        b = BIO_new_fp(f, BIO_CLOSE);
    }
#else
    b = BIO_new_file(path.external_file_string().c_str(), "rb");
#endif
    return b;
}

map<Path, Signer*> Signer::s_singletons;

Signer*
Signer::get(const Path& publicKeyPath) 
{
    Signer* rval = NULL;
    Path path = publicKeyPath;
    if (path.empty()) {
        path = bp::paths::getCertFilePath();
    }
    map<Path, Signer*>::const_iterator it;
    it = s_singletons.find(path);
    if (it == s_singletons.end()) {
        rval = new Signer(path);
        s_singletons[path] = rval;
    } else {
        rval = it->second;
    }
    return rval;
}


string
Signer::getSignature(const Path& privateKeyPath, 
                     const Path& publicKeyPath,
                     const string& password,
                     const Path& inFile)
{
    string rval;
    BIO* outBio = NULL;
    try {
        outBio = BIO_new(BIO_s_mem());
        if (outBio == NULL) {
            string s("unable to open output buffer");
            throw SSLException(s);
        }
        (void) BIO_set_close(outBio, BIO_CLOSE);

        if (doSign(privateKeyPath, publicKeyPath, password, inFile, outBio, true)) {
            // put signature into return string
            char* buf;
            long len = BIO_get_mem_data(outBio, &buf);
            if (len < 0) {
                throw SSLException("signature length < 0");
            }
            rval.append(buf, len);
        }
    } catch (SSLException& e) {
        BPLOG_ERROR("Error creating signature");
        BPLOG_ERROR(e.what());
        rval.clear();
    }
    if (outBio) BIO_free(outBio);
    return rval;
}


bool 
Signer::verifyFile(const string& signature,
                   const Path& contentPath,
                   BPTime& timestamp)
{
    BIO* content = openReadBIOFromPath(contentPath);
    (void) BIO_set_close(content, BIO_CLOSE);
    bool rval = doVerify(content, signature, timestamp);
    BIO_free(content);
    return rval;
}


bool 
Signer::verifyString(const string& strIn,
                     const string& signature,
                     BPTime& timestamp)
{
    BIO* content = BIO_new_mem_buf((void*)strIn.c_str(), strIn.length());
    bool rval = doVerify(content, signature, timestamp);
    BIO_free(content);
    return rval;
}


void
Signer::readSignerFile()
{
    BIO* cbio = openReadBIOFromPath(m_publicKeyPath);
    if (!cbio) {
        throw SSLException("Error loading the browserplus certificate");
    }
    X509* x = NULL;
    m_certStack = sk_X509_new_null();
    while ((x = PEM_read_bio_X509(cbio, NULL, NULL, NULL)) != NULL) {
        sk_X509_push(m_certStack, x);
    }
    BIO_free(cbio);
}


Signer::SSLException::SSLException(const string& str) 
: m_str(str) 
{
    BIO* mem = BIO_new(BIO_s_mem());
    ERR_print_errors(mem);
    BUF_MEM* bptr = NULL;
    BIO_get_mem_ptr(mem, &bptr);
    m_str.append("\n");
    m_str.append(bptr->data, bptr->length);
    BIO_free(mem);
}


const char* 
Signer::SSLException::what() const throw() 
{
    return m_str.c_str();
}


Signer::Signer(const Path& publicKeyPath) 
    : m_publicKeyPath(publicKeyPath)
{
    // ensure that we are getting the openssl that we expect
    const char* version = SSLeay_version(SSLEAY_VERSION);
    std::string expectedVersion(OPENSSL_VERSION_TEXT);
    if (expectedVersion.compare(version)) {
        std::string msg = "Expected openssl version " + expectedVersion;
        msg = msg + ", got version " + version;
        BPLOG_ERROR_STRM("Fatal error: " << msg);
        BP_THROW_FATAL(msg);
    }
    
    try {
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        createStore();
        readSignerFile();
    } catch (SSLException& e) { 
        BPLOG_ERROR("Error initializing Signer");
        BPLOG_ERROR(e.what());
        throw;
    }
}


Signer::~Signer()
{
    if (m_store) {
        X509_STORE_free(m_store);
    }
    if (m_certStack) {
        sk_X509_free(m_certStack);
    }
}


bool
Signer::doSign(const Path& privateKeyPath, 
               const Path& publicKeyPath,
               const string& password,
               const Path& inFile,
               BIO* outBio,
               bool detached)
{
    bool rval = false;
    BIO* pvkBio = NULL;
    BIO* certBio = NULL;
    BIO* in = NULL;
    try {
        // read private key
        pvkBio = openReadBIOFromPath(privateKeyPath);
        if (pvkBio == NULL) {
            string s("unable to open ");
            s += privateKeyPath.externalUtf8();
            throw SSLException(s);
        }
        (void) BIO_set_close(pvkBio, BIO_CLOSE);
        
        EVP_PKEY* pkey = PEM_read_bio_PrivateKey(pvkBio, NULL, NULL, 
                                                 password.empty() ?
                                                 NULL : (void*)password.c_str());
        if (pkey == NULL) {
            string s = "Error reading signer private key " + privateKeyPath.externalUtf8();
            throw SSLException(s);
        }
        
        // now for signer certificate
        certBio = openReadBIOFromPath(publicKeyPath);
        if (certBio == NULL) {
            string s("unable to open ");
            s += publicKeyPath.externalUtf8();
            throw SSLException(s);
        }
        (void) BIO_set_close(certBio, BIO_CLOSE);
        
        X509* cert = PEM_read_bio_X509(certBio, NULL, NULL, NULL);
        if (cert == NULL) {
            string s = "Error reading signer certificate in " + publicKeyPath.externalUtf8();
            throw SSLException(s);
        }
        
        // input stream
        in = openReadBIOFromPath(inFile);
        if (in == NULL) {
            string s("unable to open ");
            s += inFile.externalUtf8();
            throw SSLException(s);
        }
        (void) BIO_set_close(in, BIO_CLOSE);
        
        int flags = PKCS7_BINARY;
        if (detached) flags |= (PKCS7_NOCERTS | PKCS7_DETACHED);
        PKCS7* pkcs7 = PKCS7_sign(cert, pkey, NULL, in, flags);
        if (pkcs7 == NULL) {
            throw SSLException("Error making the PKCS#7 object");
        }
        int outFlags = detached ? PKCS7_DETACHED : 0;
        if (SMIME_write_PKCS7(outBio, pkcs7, in, outFlags) != 1) {
            throw SSLException("Error writing output");
        }
        rval = true;
    } catch (SSLException& e) {
        BPLOG_ERROR("Error creating signature");
        BPLOG_ERROR(e.what());
        rval = false;
    }
    if (in) BIO_free(in);
    if (pvkBio) BIO_free(pvkBio);
    if (certBio) BIO_free(certBio);
    return rval;
}


bool 
Signer::doVerify(BIO* contentBio,
                 const string& signature,
                 BPTime& timestamp)
{
    bool rval = false;
    BIO* in = NULL;
    BIO* pkcs7_bio = NULL;
    try {
        in = BIO_new_mem_buf((void*) signature.c_str(), -1);
        (void) BIO_set_close(in, BIO_NOCLOSE);
        
        PKCS7* pkcs7 = SMIME_read_PKCS7(in, &pkcs7_bio);
        if (pkcs7 == NULL) {
            throw SSLException("Error reading the PKCS#7 object");
        }
        if (!getTimestamp(pkcs7, timestamp)) {
            throw SSLException("Error extracting signing time");
        } 
        int res = PKCS7_verify(pkcs7, m_certStack, m_store, contentBio, NULL, 0);
        if (res != 1) {
            throw SSLException("Error verifying the signature");
        }
        rval = true;
    } catch (SSLException& e) {
        BPLOG_ERROR("Error verifying signature");
        BPLOG_ERROR(e.what());
        rval = false;
    }
    if (in) BIO_free(in);
    if (pkcs7_bio) BIO_free(pkcs7_bio);
    return rval;
}


bool
Signer::getTimestamp(PKCS7* pkcs7,
                      BPTime& t)
{
    bool rval = false;
    ASN1_UTCTIME* time = NULL;
    STACK_OF(PKCS7_SIGNER_INFO)* sk = PKCS7_get_signer_info(pkcs7);
    for (int i = 0; i < sk_PKCS7_SIGNER_INFO_num(sk); ++i) {
        PKCS7_SIGNER_INFO* si = sk_PKCS7_SIGNER_INFO_value(sk, i);
        // we only trust a signed timestamp attribute
        ASN1_TYPE* so = PKCS7_get_signed_attribute(si, NID_pkcs9_signingTime);
        if (so && so->type == V_ASN1_UTCTIME) {
            time = so->value.utctime;
        } 
        if (time != NULL) {
            // timestamp is YYmmddhhssZ, convert to
            // ISO8601 string (yyyy-mm-dd hh:mm:ssZ)
            // and make a BPTime
            string ts;
            ts.append((const char*)time->data, time->length);
            string iso;
            int y, m, d, h, min, s;
            y = m = d = h = s = 0;
            sscanf((const char*)time->data, "%2d%2d%2d%2d%2d%2dZ", 
                   &y, &m, &d, &h, &min, &s);
            const int bufSize = 80;
            char buf[bufSize];
            snprintf(buf, bufSize, 
                    "20%02d-%02d-%02d %02d:%02d:%02dZ", 
                    y, m, d, h, min, s);
            string str(buf);
            t = BPTime(str);
            rval = true;
            break;
        }
    }
    return rval;
} 


void
Signer::createStore(void)
{
    try {
        // create the cert store and set the verify callback
        m_store = X509_STORE_new();
        if (m_store == NULL) {
            throw SSLException("Error creating X509_STORE_CTX object\n");
        }
        X509_STORE_set_verify_cb_func(m_store, Signer::verify_callback);
        
        // load our trusted certificates
        BIO* cbio = openReadBIOFromPath(m_publicKeyPath);
        if (!cbio) {
            throw SSLException("Error loading the browserplus certificate");
        }
        X509* x = NULL;
        while ((x = PEM_read_bio_X509(cbio, NULL, NULL, NULL)) != NULL) {
            X509_STORE_add_cert(m_store, x);
        }
        BIO_free(cbio);
        if (X509_STORE_set_default_paths(m_store) != 1) {
            throw SSLException("Error loading certificates");
        }
        X509_LOOKUP* lookup = X509_STORE_add_lookup(m_store, 
                                                    X509_LOOKUP_file());
        if (lookup == NULL) {
            throw SSLException("Error creating X509_LOOKUP object");
        }
    } catch (SSLException& e) {
        BPLOG_ERROR("Error creating X509 store");
        BPLOG_ERROR(e.what());
        if (m_store) {
            X509_STORE_free(m_store);
            m_store = NULL;
        }
        throw;
    }
}


int 
Signer::verify_callback(int ok, 
                         X509_STORE_CTX* store) 
{
    if (!ok) {
        BPLOG_ERROR_STRM( "BrowserPlus verify failure: " <<
                          X509_verify_cert_error_string(store->error));
    }
    return ok;
}

}}
