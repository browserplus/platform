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

#include "ScriptableConfigObject.h"
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpconfig.h"
#include "BPUtils/bpconvert.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/HttpRequest.h"
#include "BPUtils/HttpSyncTransaction.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/ProcessLock.h"


#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif

#define CONFIGPANEL_UUID "E366D7CF-5C6E-4C1D-9BA4-2E4A5BD29605"
#define CONFIGPANEL_APPNAME "BrowserPlus Config Panel"
#define BUGS_URI_PATH "/api/v2/bugs"

using namespace std;
using namespace bp::file;

// a prompt handler which grants all permissions
void
ScriptableConfigObject::handleUserPrompt(void * cookie,
                                         const char * /*pathToHTMLDialog*/,
                                         const BPElement * /*arguments*/,
                                         unsigned int tid)
{
    ScriptableConfigObject * self = (ScriptableConfigObject *) cookie;
    bp::String s("Allow");
    BPDeliverUserResponse(self->m_hand, tid, s.elemPtr());
}


////////////////////////////////////////////////////////////////////////
// connect up to BrowserPlusCore
// called once we're connected to BrowserPlusCore
void
ScriptableConfigObject::connectCB(BPErrorCode ec, void * cookie,
                                  const char * err, const char * verbErr)
{
    if (ec != BP_EC_OK) {
        stringstream ss;
        ss << "Connection error: (" << BPErrorCodeToString(ec) << ")";
        if (err) ss << " " << err;
        if (verbErr) ss << ": " << verbErr;
        BPLOG_WARN(ss.str());
    } else {
        BPLOG_INFO("Successfully connected");
    }

    ScriptableConfigObject * sco = (ScriptableConfigObject *) cookie;
    sco->connectIfNeeded(ec);
}


void
ScriptableConfigObject::connectIfNeeded(BPErrorCode ec)
{
    if (m_dontReconnect) {
        BPLOG_DEBUG("Ignoring reconnect request");
        return;
    }

    // any time we get a non OK error code, we'll try to reconnect
    // The exception is BP_EC_INVALID_STATE.  This error code will occur
    // when we're not connected yet.  We don't want to retry a connection
    // attempt that's still in progress.
    if (ec != BP_EC_OK && ec != BP_EC_INVALID_STATE) {
        BPLOG_INFO("Attempting to connect to BrowserPlusCore");

        m_hand = BPAlloc();
        
        // set a handler for the user prompt callback
        BPErrorCode ec2 = BPSetUserPromptCallback(m_hand,
                                                  handleUserPrompt,
                                                  (void *) this);
        if (ec2 != BP_EC_OK) {
            BPLOG_ERROR_STRM("Failed to set user prompt handler: "
                             << BPErrorCodeToString(ec2));
        }

        std::string locale = bp::localization::getUsersLocale();
        ec2 = BPConnect(m_hand, m_uri.c_str(), locale.c_str(),
                        "BrowserPlus configuration panel",
                        connectCB, (void *) this);

        if (ec2 != BP_EC_OK) {
            BPLOG_ERROR_STRM("Synchronous connection error: "
                             << BPErrorCodeToString(ec2));
        }
    }
}

struct ProtcolRequestContext
{
    ProtcolRequestContext() : sco(NULL), tid(0), cb(0) {  }
    ScriptableConfigObject * sco;
    unsigned int tid;
    bp::CallBack cb;
};

void
ScriptableConfigObject::genericProtocolCallback(BPErrorCode ec,
                                                void * cookie,
                                                const BPElement * response)
{
    ProtcolRequestContext * prc = (ProtcolRequestContext *) cookie;

    prc->sco->connectIfNeeded(ec);

    if (ec == BP_EC_OK) {
        bp::Object * o = bp::Object::build(response);
        vector<const bp::Object *> args;
        args.push_back(o);
        prc->sco->invokeCallback(prc->tid, prc->cb, args);
        delete o;
    }

    // now release the callbacks
    prc->sco->release(prc->tid);

    delete prc;
}

ScriptableConfigObject::ScriptableConfigObject()
{
    BPInitialize();

    // let's expose some functions
    m_so.mountFunction(this, "get");
    m_so.mountFunction(this, "getSync");
    m_so.mountFunction(this, "set"); 
    m_so.mountFunction(this, "log");    
    m_so.mountFunction(this, "uninstall");    
    m_so.mountFunction(this, "restart");
    m_so.mountFunction(this, "sendReport");
    m_so.mountFunction(this, "deleteService");

    m_uri.append("bpclient://");
    m_uri.append(CONFIGPANEL_UUID);
    m_uri.append("/");
    m_uri.append(CONFIGPANEL_APPNAME);

    // start a connection attempt
    m_dontReconnect = false;
    connectIfNeeded(BP_EC_PEER_ENDED_CONNECTION);
}

ScriptableConfigObject::~ScriptableConfigObject()
{
    if (m_hand != NULL) {
        BPFree(m_hand);
        m_hand = NULL;
    }

    BPShutdown();
}

bp::html::ScriptableObject *
ScriptableConfigObject::getScriptableObject()
{
    return &m_so;
}

bp::Object *
ScriptableConfigObject::invoke(const string & functionName,
                               unsigned int id,
                               vector<const bp::Object *> args)
{
    bp::Object * rv = NULL;

    if (!functionName.compare("log"))
    {
        for (unsigned int i = 0; i < args.size(); i++) {
            BPLOG_INFO(args[i]->toPlainJsonString(true));
            //cerr << args[i]->toPlainJsonString(true) << endl;
        }
    }
    else if (!functionName.compare("get")) 
    {
        if (args.size() != 2 || args[0]->type() != BPTString
            || args[1]->type() != BPTCallBack) {
            return new bp::Bool(false);
        }
        
        bool bpEnabled = isEnabled();
        const bp::String * key = dynamic_cast<const bp::String *>(args[0]);
        assert(key != NULL);
        
        BPLOG_DEBUG_STRM("get(" << key->value() << ")");
        if (!bpEnabled) {
            BPLOG_DEBUG_STRM("\treturns false (bp disabled)");
            return new bp::Bool(false);
        }
        
        // allocate context
        ProtcolRequestContext * prc = new ProtcolRequestContext;
        prc->sco = this;
        prc->tid = id;
        prc->cb = *((bp::CallBack *) args[1]);
        
        BPErrorCode ec = BP_EC_OK;
        
        ec = BPGetState(m_hand, key->value(),
                        genericProtocolCallback,
                        (void *) prc);

        connectIfNeeded(ec);
        
        if (ec != BP_EC_OK) {
            delete prc;
        } else {
            // we'll invoke a callback when the protocol callback is hit
            retain(id);            
        }
        
        rv = new bp::Bool(ec == BP_EC_OK);
    } 
    else if (!functionName.compare("getSync")) 
    {
        if (args.size() < 1 || args[0]->type() != BPTString) {
            return NULL;
        }
        
        bool bpEnabled = isEnabled();
        const bp::String * key = dynamic_cast<const bp::String *>(args[0]);
        assert(key != NULL);
        
        if (!strcmp(key->value(), "version")) {
            string s;
            if (isInstalled()) {
                s = bp::paths::versionString();
            }
            BPLOG_DEBUG_STRM("get(version) returns " << s);
            rv = new bp::String(s);
        } else if (!strcmp(key->value(), "enabled")) {
            BPLOG_DEBUG_STRM("get(enabled) returns " << bpEnabled);
            rv = new bp::Bool(bpEnabled);
        } else if (!strcmp(key->value(), "logSize")) {
            uintmax_t totalSize = 0;
            Path dir = bp::paths::getObfuscatedWritableDirectory();
            if (isDirectory(dir)) {
                try {
                    tDirIter end;
                    tString logExt = nativeFromUtf8(".log");
                    for (tDirIter iter(dir); iter != end; ++iter) {
                        if (isRegularFile(iter->path())) {
                            if (boost::filesystem::extension(*iter).compare(logExt) == 0) {
                                totalSize += size(iter->path());
                            }
                        }
                    }
                } catch (tFileSystemError& e) {
                    BPLOG_WARN_STRM("unable to iterate thru " << dir
                                    << ": " << e.what());
                }
               
            }
            rv = new bp::Integer(totalSize);
        } else {
            BPLOG_ERROR_STRM("bad key " << key->value() << " to getSync");
            rv = new bp::Bool(false);
        }
    }
    else if (!functionName.compare("set"))    
    {
        if (args.size() != 2 || args[0]->type() != BPTString) return NULL;
        const bp::String * key = dynamic_cast<const bp::String *>(args[0]);
        assert(key != NULL);
        
        BPLOG_DEBUG_STRM("set(" << key->value() << ")");
        for (unsigned int i = 1; i < args.size(); i++) {
            string s = args[i]->toPlainJsonString();
            BPLOG_DEBUG_STRM("\targ[" << i << "] = " << s);
        }

        // setting enabled doesn't go to server
        if (!strcmp(key->value(), "enabled")) {
            Path path = bp::paths::getBPDisabledPath();
            if (args[1]->type() != BPTBoolean) {
                return new bp::Bool(false);
            }
            if ((bool) *(args[1])) {
                bp::file::remove(path);
            } else {
                BPTime now;
                string s = "BrowserPlus disabled at: " 
                    + now.asString() + "\n";
                (void) bp::strutil::storeToFile(path, s);
            }
            return new bp::Bool(true);
        }
        
        // can't set anything else if disabled
        if (!isEnabled()) {
            return new bp::Bool(false);
        }
    
        BPErrorCode ec = BPSetState(m_hand, key->value(), args[1]->elemPtr());
        connectIfNeeded(ec); 

        rv = new bp::Bool(ec == BP_EC_OK);
    } 
    else if (!functionName.compare("uninstall"))
    {
        string cmd;
        vector<string> uninstArgs;
        Path uninstaller = bp::paths::getUninstallerPath();
        
        // Forcefully kill daemon and don't let us reconnect
        // This helps prevent cruft after the uninstall, although
        // the side effect is that the config panel closes
        m_dontReconnect = true;
        string daemon = utf8FromNative(bp::paths::getDaemonPath().filename());
        bp::process::kill(daemon, true);

        bp::ProcessLock lock =  NULL;
#ifdef WIN32
        // wait for daemon to die
        string lockName = bp::paths::getIPCLockName();
        int count = 0;
        while (!lock && count < 10) {
            lock = bp::acquireProcessLock(false, lockName);
            if (!lock) {
                Sleep(500);
                count++;
            }
        }
#endif

        // now run uninstaller
        bp::process::spawnStatus status;
        bp::process::spawn(uninstaller, std::string(), Path(), 
                           uninstArgs, &status);

        if (lock) {
            bp::releaseProcessLock(lock);
        }

#ifdef WIN32
        // uninstaller will leave cruft since windows can't 
        // delete open files.  thus, we die
        exit(0);
#endif
        rv = new bp::Bool(true);
    }
    else if (!functionName.compare("restart")) 
    {
        if (!isEnabled()) {
            return new bp::Bool(false);
        }
        string daemon = utf8FromNative(bp::paths::getDaemonPath().filename());
        // forceful kill here.  Restart should work even when the daemon
        // is hung
        bool b = bp::process::kill(daemon, true);
        rv = new bp::Bool(b);
    }
    else if (!functionName.compare("sendReport"))
    {
        using namespace bp::http;
        
        Path tarball;
        Path postBodyPath;
        Path userReportPath;
        try {
            if (!isEnabled()) {
                throw("BrowserPlus is disabled");
            }
            
            if (args.size() < 2 || args[0]->type() != BPTString 
                || args[1]->type() != BPTBoolean) {
                throw("bad arguments to sendReport");
            }
            
            const bp::String * reportObj = dynamic_cast<const bp::String *>(args[0]);
            const bp::Bool * includeLogs = dynamic_cast<const bp::Bool *>(args[1]);
            
            // Get post url
            bp::config::ConfigReader configReader;
            if (!configReader.load(bp::paths::getConfigFilePath())) {
                BPLOG_ERROR_STRM("unable to read config file");
                return new bp::Bool(false);
            }
            string url;
            if (!configReader.getStringValue("DistServer", url)) {
                throw("unable to read DistServer from config file");
            }
            url.append(BUGS_URI_PATH);

            // Create tarball of contents
            tarball = getTempPath(getTempDirectory(), "BrowserPlusErrorTar_");
            bp::tar::Create tar;
            if (!tar.open(tarball)) {
                throw string("unable to open " + tarball.externalUtf8());
            }
            userReportPath = getTempPath(getTempDirectory(), "BrowserPlusErrorReport_");
            if (!bp::strutil::storeToFile(userReportPath, reportObj->value())) {
                throw string("unable to save report to " + userReportPath.externalUtf8());
            }
            if (!tar.addFile(userReportPath, Path("error_report.txt"))) {
                throw string("unable to add " + userReportPath.externalUtf8());
            }
            
            if (includeLogs->value()) {
                tString logExt = nativeFromUtf8(".log");
                Path dir = bp::paths::getObfuscatedWritableDirectory();
                if (isDirectory(dir)) {
                    try {
                        tDirIter end;
                        for (tDirIter iter(dir); iter != end; ++iter) {
                            if (boost::filesystem::extension(*iter).compare(logExt) == 0) {
                                Path log(iter->path());
                                Path relPath = log.relativeTo(dir);
                                if (!tar.addFile(log, relPath)) {
                                    throw string("unable to add " + log.externalUtf8());
                                }
                            }
                        }
                    } catch (const tFileSystemError& e) {
                        BPLOG_WARN_STRM("unable to iterate thru " << dir
                                        << ": " << e.what());
                    }
                }
            }
            if (!tar.close()) {
                throw("unable to close tarball " + tarball.externalUtf8());
            }

            // Now compress tarball
            postBodyPath = getTempPath(getTempDirectory(), "BrowserPlusError_");
            bp::lzma::Compress compress;
            ifstream ifs;
            if (!openReadableStream(ifs, tarball, ifstream::in | ifstream::binary)) {
                throw string("unable to open stream for " + tarball.externalUtf8());
            }
            ofstream ofs;
            if (!openWritableStream(ofs, postBodyPath, 
                                    ofstream::out | ofstream::binary | ofstream::trunc)) {
                throw string("unable to open stream for " + postBodyPath.externalUtf8());
            }
            compress.setInputStream(ifs);
            compress.setOutputStream(ofs);
            if (!compress.run()) {
                throw string("unable to compress " + tarball.externalUtf8()
                             + " to " + postBodyPath.externalUtf8());
            }
            ifs.close();
            ofs.close();
                
            // Now send via sync http post
            RequestPtr req(new Request(Method::HTTP_POST, url));
            req->headers.add(Headers::ksUserAgent,
                             "Yahoo! BrowserPlus Error Reporter");
            req->headers.add(Headers::ksHost,
                             req->url.host() + ":"
                             + bp::conv::toString(req->url.port()));
            string filevar("errorTar");
            req->headers.add("Content-Disposition: form-data; name=\""
                             + filevar + "\"; filename=\"" + 
                             utf8FromNative(postBodyPath.filename()) + "\"");
            req->headers.add(Headers::ksContentType, "application/x-lzip");
            uintmax_t contentLength = size(postBodyPath);
            req->headers.add(Headers::ksContentLength,
                             bp::conv::toString(contentLength));
            req->headers.add(Headers::ksConnection, "close");
            req->body.fromPath(postBodyPath);
            
            client::SyncTransaction tran(req);
            client::SyncTransaction::FinalStatus results;
            ResponsePtr ptrResp = tran.execute(results);
            if (results.code == client::SyncTransaction::FinalStatus::eOk) {
                rv = new bp::Bool(true);
            } else {
                BPLOG_ERROR_STRM("sync transaction failed, code = " << results.code);
                rv = new bp::Bool(false);
            }

            rv = new bp::Bool(true);
        } catch (const string& msg) {
            BPLOG_ERROR_STRM(msg);
            rv = new bp::Bool(false);
        }
        
        // clean up
        remove(tarball);
        remove(postBodyPath);
        remove(userReportPath);
    }
    else if (!functionName.compare("deleteService")) 
    {
        if (args.size() != 2 || args[0]->type() != BPTString
            || args[1]->type() != BPTString) {
            return new bp::Bool(false);
        }
        
        if (!isEnabled()) {
            BPLOG_DEBUG_STRM("\treturns false (bp disabled)");
            return new bp::Bool(false);
        }
        
        const bp::String * service = dynamic_cast<const bp::String *>(args[0]);
        const bp::String * version = dynamic_cast<const bp::String *>(args[1]);
        bp::Map m;
        m.add("service", service->clone());
        m.add("version", version->clone());
        BPErrorCode ec = BPSetState(m_hand, "deleteService", m.elemPtr());
        connectIfNeeded(ec); 
        
        rv = new bp::Bool(ec == BP_EC_OK);
    }
    
    return rv;
}


bool
ScriptableConfigObject::isInstalled()
{
    return exists(bp::paths::getBPInstalledPath());
}


bool
ScriptableConfigObject::isEnabled()
{
    return isInstalled() && !exists(bp::paths::getBPDisabledPath());
}
