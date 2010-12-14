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
 * A simple proxy abstraction which runs BrowserPlusUpdater
 * as a separate process, getting progress/status/errors via IPC
 */

#include "InstallProcessRunner.h"
#include "BPUtils/BPLog.h"
#include "platform_utils/ProductPaths.h"

#define TIMER_MSECS 1000
#define MAX_TIMER_FIRES 10

InstallProcessRunner::InstallProcessRunner(const boost::filesystem::path& logPath,
                                           const std::string& logLevel)
    : m_logPath(logPath), m_logLevel(logLevel), m_listener(), m_ipcName(), m_server(),
      m_procStatus(), m_timer(), m_timerFires(0),
      m_connectedToUpdater(false)
{
    m_timer.setListener(this);
}
    

InstallProcessRunner::~InstallProcessRunner()
{
    m_timer.cancel();
    m_server.reset();
}


void
InstallProcessRunner::setListener(std::tr1::weak_ptr<bp::install::IInstallerListener> l)
{
    m_listener = l;
}


// allocate the underlying installer and get it running
void
InstallProcessRunner::start(const boost::filesystem::path& dir,
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

    boost::filesystem::path updaterExe = bp::file::canonicalProgramPath(dir/"BrowserPlusUpdater");
    std::vector<std::string> args;
    args.push_back("-ipcName=" + m_ipcName);
    args.push_back("-logPath=" + m_logPath.string());
    args.push_back("-logLevel=" + m_logLevel);
    args.push_back(bp::file::nativeUtf8String(dir));
    if (bp::process::spawn(updaterExe, args, &m_procStatus,
                           dir, "BrowserPlusUpdater")) {
        // set a timer to catch non-responsive updater
        m_timer.setMsec(TIMER_MSECS);
    } else {
        BPLOG_ERROR_STRM("failed to spawn(" << updaterExe 
                         << "), errCode = " << m_procStatus.errCode);
        std::tr1::shared_ptr<bp::install::IInstallerListener> l = m_listener.lock();
        if (l) l->onError("unable to launch BrowserPlusUpdater");
    }
}


void
InstallProcessRunner::timesUp(bp::time::Timer* t) 
{
    if (!m_connectedToUpdater) {
        if (m_timerFires++ > MAX_TIMER_FIRES) {
            BPLOG_ERROR_STRM("Unable to connect to BrowserPlusUpdater");
            std::tr1::shared_ptr<bp::install::IInstallerListener> l = m_listener.lock();
            if (l) {
                l->onError("unable to launch BrowserPlusUpdater");
                l->onDone();
            }
        }
        m_timer.setMsec(TIMER_MSECS);
    } else {
        m_timer.cancel();
    }
}


void
InstallProcessRunner::gotChannel(bp::ipc::Channel* c) 
{
    BPLOG_DEBUG("Installer IPC received connection from BrowserPlusUpdater");
    m_connectedToUpdater = true;
    c->setListener(this);
    // stop the server immediately, we're a one trick pony
    m_server->stop();
}


void 
InstallProcessRunner::serverEnded(bp::ipc::IServerListener::TerminationReason,
                                  const char* err) const
{
	BPLOG_ERROR_STRM("Installer IPC listening channel fell down: " 
		             << (err ? err : ""));
    std::tr1::shared_ptr<bp::install::IInstallerListener> l = m_listener.lock();
    if (l) {
        l->onError("unable to launch BrowserPlusUpdater");
        l->onDone();
    }
    m_server.reset();
}


void
InstallProcessRunner::channelEnded(bp::ipc::Channel* c,
                                   bp::ipc::IConnectionListener::TerminationReason why,
                                   const char* errorString)
{
    int exitCode = 0;
    (void) bp::process::wait(m_procStatus, true, exitCode);
    BPLOG_DEBUG_STRM("updater exits with status " << exitCode);

    std::tr1::shared_ptr<bp::install::IInstallerListener> l = m_listener.lock();
    if (l) l->onDone();
    m_server.reset();
}


void 
InstallProcessRunner::onMessage(bp::ipc::Channel* c,
                                const bp::ipc::Message& m)
{
    // messages are status, progress, and error
    std::tr1::shared_ptr<bp::install::IInstallerListener> l = m_listener.lock();
    const bp::Object* payload = m.payload();
    if (!payload) {
        BPLOG_WARN_STRM("malformed message, no payload: " << m.serialize());
        return;
    }
    if (!m.command().compare("status")) {
        if (!payload->has("message")) {
            BPLOG_WARN_STRM("malformed 'status' message, no 'message': " << m.serialize());
        } else {
            std::string msg = std::string(*(payload->get("message")));
            BPLOG_DEBUG_STRM("status: " << msg);
            if (l) l->onStatus(msg);
        }
    } else if (!m.command().compare("progress")) {
        if (!payload->has("percent")) {
            BPLOG_WARN_STRM("malformed 'progress' message, no 'percent': "<< m.serialize());
        } else {
            unsigned int percent = (unsigned int)(long long) *(payload->get("percent"));
            BPLOG_DEBUG_STRM("progress: " << percent);
            if (l) l->onProgress(percent);
        }
    } else if (!m.command().compare("error")) {
        if (!payload->has("message")) {
            BPLOG_WARN_STRM("malformed 'error' message, no 'message': " << m.serialize());
        } else {
            std::string msg = std::string(*(payload->get("message")));
            BPLOG_DEBUG_STRM("error: " << msg);
            if (l) l->onError(msg);
        }
    } else if (!m.command().compare("done")) {
        BPLOG_DEBUG_STRM("done");
        if (l) l->onDone();
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

