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
 * SessionCreator.cpp -- An abstraction resposible for establishing a
 *                       connection with the core daemon.  This involves
 *                       spawning the daemon if not running, connecting,
 *                       and executing a CreateSession protocol message
 *                       which relays the URI to the daemon and gives
 *                       a chance for the daemon to pass errors.
 */

#include "SessionCreator.h"
#include <fstream>
#include "BPProtoUtil.h"
#include "BPUtils/bpconvert.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "platform_utils/bpexitcodes.h"
#include "platform_utils/bpplatformutil.h"
#include "platform_utils/ProductPaths.h"


const double SessionCreator::c_initialPollPeriodS = 0.020;
const double SessionCreator::c_maxPollPeriodS = 0.5;
const double SessionCreator::c_pollPeriodGrowthRate = 1.8;
const double SessionCreator::c_maxWaitS = 20.0;    

const char * SessionCreator::c_createSessionCommand = "CreateSession";

SessionCreator::SessionCreator(bp::ipc::Channel * channel)
    : m_channel(channel), m_spawnedProcess(false),
      m_sentCreateSession(false),
      m_elapsedTime(), m_timer(),
      m_curPollS(c_initialPollPeriodS), m_listener(NULL)
{
    m_timer.setListener(this);
}

SessionCreator::~SessionCreator()
{
    m_timer.cancel();
}

void
SessionCreator::reportError(BPErrorCode e,
                            const std::string & verboseError,
                            const char * errorString)
{
    BPLOG_INFO_STRM(this << ": connection to daemon fails ("
                    << BPErrorCodeToString(e) << "): "
                    << verboseError);

    // shut everything down
    m_timer.cancel();
    m_elapsedTime.reset();
    m_channel->disconnect();
    m_channel->setListener(NULL);

    if (m_listener) {
        std::string stdErrorString;
        if (errorString) stdErrorString.append(errorString);
        m_listener->sessionCreationFailed(e, stdErrorString, verboseError);
        m_listener = NULL;
    }
}

void
SessionCreator::setListener(ISessionCreatorListener * listener)
{
    m_listener = listener;
}

/**
 * Connect must attempt to connect to the local browserplus daemon,
 * and spawn it if not running.  The algoritm employed is thus:
 * 1. attempt to connect to browserplus
 * 2. if fails, spawn browser plus
 * 3. attempt to connect to browserplus
 * 4. if allowed time elapses, fail
 */
void
SessionCreator::createSession(const char * uri, const char * locale,
                              const char * userAgent)
{
    BPLOG_INFO_STRM(this << ": createSession");
    BPLOG_DEBUG_STRM("to " << uri);

    // Do NOT call report error from within this function.  We must
    // return normally and then return errors through our listener,
    // otherwise the semantics become difficult and unexpected for the
    // client (YIB-2172568)

    m_uri.clear();
    if (uri) m_uri.append(uri);
    m_locale.clear();
    if (locale) m_locale.append(locale);
    m_userAgent.clear();
    if (userAgent) m_userAgent.append(userAgent);
    
    // start the clock
    m_elapsedTime.reset();
    m_elapsedTime.start();

    // We always start by trying to connect to the daemon
    // We'll introduce a 1 msec delay to allow the call to complete,
    // and then attempt connection.  this gaurantees we don't call
    // our listener before the createSession call completes.
    m_timer.setMsec(1);
}

static std::string
getNewestInstalledPlatform()
{
    bp::SemanticVersion newest;
    bp::file::Path dir = bp::paths::getProductTopDirectory();
    if (bp::file::isDirectory(dir)) {
        try {
            bp::file::tDirIter end;
            for (bp::file::tDirIter it(dir); it != end; ++it)
            {
                bp::SemanticVersion version;
                std::string s = bp::file::utf8FromNative(it->path().filename());
                if (version.parse(s) && version.compare(newest) >= 0 &&
                    bp::file::exists(bp::paths::getBPInstalledPath(
                                         version.majorVer(),
                                         version.minorVer(),
                                         version.microVer())))
                {
                    newest = version;
                }
            }
        } catch (const bp::file::tFileSystemError& e) {
            BPLOG_WARN_STRM("unable to iterate thru " << dir
                            << ": " << e.what());
        }
    }

    if (newest.majorVer() < 0) return std::string();
    
    return newest.asString();
}

void
SessionCreator::tryConnect() 
{
    // Make sure that bp is fully installed.  The installer's last
    // act is to create the file at BPInstalledPath.  This test
    // prevents us from trying to launch before our install is complete.
    if (!bp::file::exists(bp::paths::getBPInstalledPath())) {
        // not installed!  now either there's a newer version that
        // just got installed, or we were just uninstalled but still
        // loaded into browser memory.
        std::string newVer = getNewestInstalledPlatform();
        
        if (newVer.empty()) {
            BPLOG_ERROR("BrowserPlus not installed");
            reportError(BP_EC_NOT_INSTALLED, "BrowserPlus not installed");
            return;
        } else {
            BPLOG_WARN("BrowserPlus has been updated");
            reportError(BP_EC_SWITCH_VERSION, newVer);
            return;
        }
    }
    
    if (bp::file::exists(bp::paths::getBPDisabledPath())) {
        BPLOG_ERROR("BrowserPlus disabled");
        reportError(BP_EC_PLATFORM_DISABLED,
                    "The BrowserPlus platform has been disabled");
        return;
    } 

    BPLOG_DEBUG_STRM(this << ", TryConnect");
    
    std::string ipcName = bp::paths::getIPCName();

    BPLOG_DEBUG_STRM(this << ", try IPC connect to " << ipcName);
    BPASSERT( m_channel != NULL );
    std::string errBuf;
    
    if (!m_channel->connect(ipcName, &errBuf)) {
        BPLOG_WARN_STRM(this << ", IPC connect failed: " << errBuf);

        if (m_elapsedTime.elapsedSec() > c_maxWaitS) {
            BPLOG_WARN_STRM(this << ", Couldn't connect to daemon after "
                            << m_elapsedTime.elapsedSec() << "s");
            reportError(BP_EC_CONNECTION_FAILURE,
                        "max time exceeded");
        } else if (!m_spawnedProcess) {
            BPLOG_INFO_STRM("Cannot connect to BrowserPlus, spawning");

            if (!startupDaemon(m_spawnStatus)) {
                BPLOG_ERROR_STRM(this << ", Couldn't start daemon: "
                                 << m_spawnStatus.errCode);
                reportError(BP_EC_SPAWN_FAILED,
                            "failed to start BrowserPlus daemon");
                return;
            }

            // reset the timer, accounting for time spent in the UAC
            m_elapsedTime.reset();
            m_elapsedTime.start();
            m_spawnedProcess = true;

            double pp = getPollPeriodSec();
            BPLOG_INFO_STRM(this << ", Spawned browserplus, waiting " 
                            << pp << "s for startup...");
            m_timer.setMsec(bp::conv::SecToMsec(pp));
        } else {
            // Check for daemon error exit.  If no exit, wait and try again.
            int errCode = 0;
            if (bp::process::wait(m_spawnStatus, false, errCode)) {
                // daemon has exited, exit status of kKillswitch well-known
                if (errCode == bp::exit::kKillswitch) {
                    BPLOG_ERROR_STRM("daemon not running, blacklisted");
                    BPLOG_ERROR_STRM("removing blacklisted platform");
                    bp::SemanticVersion version;
                    (void) version.parse(bp::paths::versionString());
                    bp::platformutil::removePlatform(version, true);
                    reportError(BP_EC_PLATFORM_BLACKLISTED,
                                "BrowserPlus platform version blacklisted");
                    return;
                }
            }
            double pp = getPollPeriodSec();
            BPLOG_INFO_STRM(this << ", Connection failed, waiting " 
                            << pp << "s for startup...");
            m_timer.setMsec(bp::conv::SecToMsec(pp));
        }
    } else {

        if (!m_channel->setListener(this)) {
            reportError(BP_EC_CONNECTION_FAILURE,
                        "system resources exhausted");
            return;
        }


        BPLOG_INFO_STRM(this <<
                        ", IPC connect ok, sending CreateSession message");

        BPLOG_INFO_STRM(this << ", session started in " <<
                        m_elapsedTime.elapsedSec() << "s");



        // now we need to send our "CreateSession" message
        bp::ipc::Query query;

        // set the command
        query.setCommand(c_createSessionCommand);

        // add payload
        bp::Map payload;
        payload.add("uri", new bp::String(m_uri));
        payload.add("locale", new bp::String(m_locale));
        payload.add("userAgent", new bp::String(m_userAgent));
        payload.add("clientPid", new bp::Integer(bp::process::currentPid()));
        query.setPayload(payload);

        if (!m_channel->sendQuery(query)) {
            BPLOG_ERROR_STRM("Couldn't send CreateSession message to daemon");
            reportError(BP_EC_CONNECTION_FAILURE,
                        "Couldn't initiate session");
        }
        // now how long shall we allow for the daemon to respond
        double pp = c_maxWaitS - m_elapsedTime.elapsedSec();
        // it shouldn't be the case that we've gone past our allowed time,
        // if somehow magically we're right on the brink, we'll allow
        // another max poll period for the daemon to respond (he's on
        // probation already)
        if (pp < 0) pp = c_maxPollPeriodS;
        m_timer.setMsec(bp::conv::SecToMsec(pp));
        BPLOG_INFO_STRM("Sent " << c_createSessionCommand << " message"
                        << ", waiting " << pp << "s for response");
        m_sentCreateSession = true;
    }
}

void
SessionCreator::timesUp(bp::time::Timer *)
{
    if (m_sentCreateSession) {
        // the daemon has taken too long to respond.
        reportError(BP_EC_CONNECTION_FAILURE,
                    "Daemon non-responsive.");
    } else {
        tryConnect();
    }
}

void
SessionCreator::onMessage(bp::ipc::Channel *,
                          const bp::ipc::Message &)
{
    // no messages are expected, protocol error
    reportError(BP_EC_PROTOCOL_ERROR, "unexpected message received");
}

void
SessionCreator::channelEnded(
    bp::ipc::Channel *,
    bp::ipc::IConnectionListener::TerminationReason,
    const char *)
{
    // TODO:  do something!
}

bool
SessionCreator::onQuery(bp::ipc::Channel *,
                        const bp::ipc::Query &q,
                        bp::ipc::Response &)
{
    // no queries are expected, protocol error
    std::stringstream ss;
    ss << "unexpected query received: " << q.command();
    reportError(BP_EC_PROTOCOL_ERROR, ss.str().c_str());
    return false;
}

void
SessionCreator::onResponse(bp::ipc::Channel *,
                           const bp::ipc::Response & response)
{
    if (!m_sentCreateSession) {
        reportError(BP_EC_PROTOCOL_ERROR,
                    "CreateSession response received BEFORE request");
        return;
    }
    
    if (0 != response.command().compare(c_createSessionCommand)) {
        std::string s("Unexpected protocol response: ");
        s.append(response.command());
        reportError(BP_EC_PROTOCOL_ERROR, s);
        return;
    }
    
    // we expect as a response the standard
    // { success: , error: , verboseError: }
    // message
    if (response.payload() == NULL ||
        !response.payload()->has("success", BPTBoolean))
    {
        std::string s("Malformed response to ");
        s.append(c_createSessionCommand);
        s.append(" command");
        reportError(BP_EC_PROTOCOL_ERROR, s);
        return;
    }

    if ((bool) *(response.payload()->get("success"))) {
        // success! w00t!
        BPLOG_INFO_STRM(this << " successful connection in "
                        << m_elapsedTime.elapsedSec() << "s");
        m_timer.cancel();
        if (m_listener) {
            m_listener->sessionCreated();
        }
    } else {
        BPLOG_INFO_STRM(this << " connection failed after "
                        << m_elapsedTime.elapsedSec() << "s");

        // error!  lets see how specific we can get
        BPErrorCode ec;
        std::string errorString;
        std::string verboseError;
        mapResponseToErrorCode(response.payload(), ec,
                               errorString, verboseError);
        reportError(ec, verboseError, errorString.c_str());
    }
}

#define BP_MIN(x,y) ((x < y) ? x : y)

double
SessionCreator::getPollPeriodSec()
{
    double rv = m_curPollS;

    m_curPollS *= c_pollPeriodGrowthRate;
    m_curPollS = BP_MIN(m_curPollS, c_maxPollPeriodS);
    
    return rv;
}
