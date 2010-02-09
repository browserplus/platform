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
 * A simple proxy abstraction which runs BrowserPlusUpdater
 * as a separate process, getting progress/status/errors via IPC
 */

#include "InstallProcessRunner.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"

InstallProcessRunner::InstallProcessRunner()
    : m_listener(NULL), m_ipcName(), m_server(), m_procStatus()
{
}
    

InstallProcessRunner::~InstallProcessRunner()
{
    m_server.reset();
}


void
InstallProcessRunner::setListener(bp::install::IInstallerListener* listener)
{
    m_listener = listener;
}


// allocate the underlying installer and get it running
void
InstallProcessRunner::start(const bp::file::Path& dir,
                            bool deleteWhenDone)
{
    m_server.reset(new bp::ipc::ChannelServer);
    m_server->setListener(this);
    m_ipcName = bp::paths::getEphemeralIPCName();
    std::string err;
    BPLOG_DEBUG_STRM("start IPC server on " << m_ipcName);
    if (!m_server->start(m_ipcName, &err)) {
        BPLOG_WARN_STRM("unable to start IPC server on " << m_ipcName
                        << ", err = " << err
                        << ", progress will be inaccurate and status/errors"
                        << " will be lost");
    }

    bp::file::Path updaterExe = bp::file::canonicalProgramPath(dir/"BrowserPlusUpdater");
    std::vector<std::string> args;
    args.push_back("-ipcName=" + m_ipcName);
    args.push_back(dir.externalUtf8());
    if (bp::process::spawn(updaterExe, "BrowserPlusUpdater",
                           dir, args, &m_procStatus)) {
        // XXX timer to ensure connection
    } else {
        BPLOG_ERROR_STRM("failed to spawn(" << updaterExe 
                         << "), errCode = " << m_procStatus.errCode);
        // XXX localize!
        if (m_listener) {
            m_listener->onError("unable to launch BrowserPlusUpdatere");
        }
    }
}


void
InstallProcessRunner::gotChannel(bp::ipc::Channel* c) 
{
    BPLOG_DEBUG("Installer IPC received connection from BrowserPlusUpdater");
    c->setListener(this);
    // stop the server immediately, we're a one trick pony
    m_server->stop();
}


void 
InstallProcessRunner::serverEnded(bp::ipc::IServerListener::TerminationReason,
                                  const char* err) const
{
    // XXX localize!
    if (m_listener) {
        m_listener->onError("unable to launch BrowserPlusUpdatere");
    }

	BPLOG_ERROR_STRM("Installer IPC listening channel fell down: " 
		             << (err ? err : ""));
    m_server.reset();
}


void
InstallProcessRunner::channelEnded(bp::ipc::Channel* c,
                                   bp::ipc::IConnectionListener::TerminationReason why,
                                   const char* errorString)
{
    // XXX?  hmm, never get this callback
#ifdef NOTDEF
    int exitCode = 0;
    (void) bp::process::wait(m_procStatus, true, exitCode);
    BPLOG_DEBUG_STRM("updater exits with status " << exitCode);
#endif
	BPLOG_ERROR_STRM("Installer IPC channel ended, err = "
                     << errorString);
    m_server.reset();
}


void 
InstallProcessRunner::onMessage(bp::ipc::Channel* c,
                                const bp::ipc::Message& m)
{
    BPLOG_DEBUG_STRM("got message " << m.serialize());
    if (!m_listener) {
        BPLOG_WARN_STRM("no listener, message ignored");
        return;
    }

    // messages are status, progress, and error
    const bp::Object* payload = m.payload();
    if (!payload) {
        BPLOG_WARN_STRM("malformed message, no payload");
        return;
    }
    if (!m.command().compare("status")) {
        if (!payload->has("message")) {
            BPLOG_WARN_STRM("malformed 'status' message, no 'message'");
        } else {
            std::string msg = std::string(*(payload->get("message")));
            m_listener->onStatus(msg);
        }
    } else if (!m.command().compare("progress")) {
        if (!payload->has("percent")) {
            BPLOG_WARN_STRM("malformed 'progress' message, no 'percent'");
        } else {
            unsigned int percent = (unsigned int)(long long) *(payload->get("percent"));
            m_listener->onProgress(percent);
        }
    } else if (!m.command().compare("error")) {
        if (!payload->has("message")) {
            BPLOG_WARN_STRM("malformed 'error' message, no 'message'");
        } else {
            std::string msg = std::string(*(payload->get("message")));
            m_listener->onError(msg);
        }
    } else {
        BPLOG_WARN_STRM("unrecognized message " << m.command()
                        << " ignored");
    }
}


bool 
InstallProcessRunner::onQuery(bp::ipc::Channel* c,
                              const bp::ipc::Query& query,
                              bp::ipc::Response& response)
{
	BPLOG_ERROR_STRM("Got unexpected query from service: "
		             << query.command());
    return false;
}


void
InstallProcessRunner::onResponse(bp::ipc::Channel* c,
                                 const bp::ipc::Response& response)
{
	BPLOG_ERROR_STRM("Got unexpected response from service: "
		             << response.command());
}

