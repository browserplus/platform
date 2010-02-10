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
 * An abstraction which allows a controlling process (usually
 * BrowserPlusCore) fork, exec, examine, and interact with a
 * spawned process (a service instance).
 *
 * First introduced by Lloyd Hilaiel on 2009/01/14
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "Controller.h"
#include <sstream>
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"
#include "Process.h"
#include "ServiceServer.h"


using namespace ServiceRunner;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;

Controller::Controller(const std::string & service,
                       const std::string & version)
    : m_pid(0), m_listener(NULL), m_id(0)
{
    m_path = bp::paths::getCoreletDirectory() / service / version;
}

Controller::Controller(const bpf::Path & path)
    : m_path(path), m_pid(0), m_listener(NULL), m_id(0)
{
}

Controller::~Controller()
{
    if (m_pid && m_chan) {
//        std::cout << "waiting on " << m_pid << std::endl;
        m_chan.reset();
//        int exitCode;
//        std::cout << "waiting!" << std::endl;
//        (void) bp::process::wait(m_spawnStatus, true, exitCode);
//        std::cout << "waited!" << std::endl;
    }
    
    m_spawnCheckTimer.setListener(NULL);
    m_spawnCheckTimer.cancel();

    m_serviceConnector.reset();
}

void
Controller::setListener(IControllerListener * listener)
{
    m_listener = listener;
}

bool
Controller::run(const bpf::Path & pathToHarness,
                const bpf::Path & providerPath,
                const std::string & serviceTitle,
                const std::string & logLevel,
                const bpf::Path & logFile,
                std::string & err)
{
    if (m_pid != 0 || m_id != 0) {
        err.append("SpawnedServiceController::run apparently called twice");
        return false;
    }

    // ensure that the target directory exists
    if (!bfs::is_directory(m_path)) {
        err.append("no such directory: ");
        err.append(m_path.externalUtf8());
        return false;
    }

    // what binary will we be using as a ipc harness program?
    bpf::Path executable = pathToHarness;
    if (executable.empty()) executable = bp::paths::getRunnerPath();

    if (!bpf::exists(executable)) {
        err.append("no such file: ");
        err.append(executable.externalUtf8());
        return false;
    }

    // now allocate a service connector to listen for incoming connection
    assert(m_serviceConnector == NULL);
    m_serviceConnector.reset(new ServiceRunner::Connector);
    assert(m_serviceConnector != NULL);
    m_serviceConnector->setListener(shared_from_this());

    // set up arguments for the child process, which will be an
    // instance of BrowserPlusCore with the -runService flag set
    std::vector<std::string> args;
    args.push_back("-runService");

    args.push_back("-ipcName");    
    args.push_back(m_serviceConnector->ipcName());

    if (!providerPath.empty())
    {
        args.push_back("-providerPath");    
        args.push_back(bpf::canonicalPath(providerPath).utf8());
    }

    if (!logLevel.empty())
    {
        args.push_back("-log");    
        args.push_back(logLevel);
    }

    if (!logFile.empty())
    {
        args.push_back("-logfile");    
        args.push_back(bpf::canonicalPath(logFile).utf8());
    }
    
    m_sw.reset();
    m_sw.start();    

	// spawn the little dude
    if (bp::process::spawn(executable, serviceTitle, m_path, args,
                           &m_spawnStatus))
    {
        BPLOG_INFO_STRM("successfully spawned service process for "
                        << m_path << ", pid: " << m_spawnStatus.pid <<
                        " - waiting for connection on "
                        << m_serviceConnector->ipcName());
        m_pid = m_spawnStatus.pid;

        // now let's set up our poller to poll the status of the spawned
        // process until we get an ipc connection established
        m_spawnCheckTimer.setListener(this);
        m_spawnCheckTimer.setMsec(200);

        
    }
    else
    {
        BPLOG_ERROR("Failed to spawn service process!");
        err.append("spawn failed: ");
        err.append(bp::error::lastErrorString());
        return false;
    }

    return true;
}

void
Controller::timesUp(bp::time::Timer *)
{
    // it's been 200ms since we spawned the process and we haven't
    // got a connection yet, has the process exited?
    int exitCode = 0;
    if (bp::process::wait(m_spawnStatus, false, exitCode)) {
        BPLOG_ERROR_STRM("Spawned service process exited with code: "
                         << exitCode);
        m_spawnCheckTimer.cancel();
        // this callback may delete us
        if (m_listener) m_listener->onEnded(this);
    } else {
        // perform another check in 200ms.
        m_spawnCheckTimer.setMsec(200);        
    }
}

void
Controller::describe()
{
    if (m_chan != NULL) {
        bp::ipc::Query q;
        q.setCommand("getDescription");
        (void) m_chan->sendQuery(q);
    } else {
        // TODO: return a failure code?
    }
}

unsigned int
Controller::allocate(const std::string & uri,
                     const bpf::Path & data_dir,
                     const bpf::Path & temp_dir,
                     const std::string & locale,
                     const std::string & userAgent,
                     unsigned int clientPid)
{
    bp::ipc::Query q;

    if (m_chan != NULL)
    {
        bp::Map context;
        context.add("uri", new bp::String(
                        uri.empty() ? std::string("bpclient://unknown") : uri));
        context.add("data_dir", new bp::String(data_dir.externalUtf8()));
        context.add("temp_dir", new bp::String(temp_dir.externalUtf8()));
        context.add("locale", new bp::String(locale));
        context.add("userAgent", new bp::String(userAgent));
        context.add("clientPid", new bp::Integer(clientPid));
        q.setCommand("allocate");
        q.setPayload(context.clone());
        if (m_chan->sendQuery(q)) return q.id();
        // if send fails, return -1
    }

    return (unsigned int) -1;
}

void
Controller::destroy(unsigned int id)
{
    if (m_chan != NULL) {
        bp::ipc::Message m;
        m.setCommand("destroy");
        m.setPayload(new bp::Integer(id));
        (void) m_chan->sendMessage(m);
    } else {
        // TODO: we do not convey this condition to the client
    }
}

unsigned int
Controller::invoke(unsigned int instanceId,
                   const std::string & function,
                   const bp::Object * arguments)
{
    if (m_chan != NULL) {
        bp::ipc::Query q;
        q.setCommand("invoke");
        bp::Map * payload = new bp::Map;
        payload->add("function", new bp::String(function));
        if (arguments != NULL) {
            payload->add("arguments", arguments->clone());
        }
        payload->add("instance", new bp::Integer(instanceId));
        q.setPayload(payload);
        if (m_chan->sendQuery(q)) return q.id();
    }
    return (unsigned int) -1;
}

void
Controller::sendResponse(unsigned int promptId,
                         const bp::Object * arguments)
{
    if (m_chan != NULL) {
        bp::ipc::Message m;
        m.setCommand("promptResponse");
        bp::Map * payload = new bp::Map;
        payload->add("promptId", new bp::Integer(promptId));
        if (arguments != NULL) {
            payload->add("arguments", arguments->clone());
        }
        m.setPayload(payload);
        (void) m_chan->sendMessage(m);
    }
}

void
Controller::onConnected(bp::ipc::Channel * c, const std::string & name,
                        const std::string & version,
                        unsigned int apiVersion)
{
    // no need to check spawn status anymore
    m_spawnCheckTimer.cancel();

    m_service = name;
    m_version = version;
    
    BPLOG_INFO_STRM("Received connected IPC channel for "
                    << name << " v" << version << " in "
                    << m_sw.elapsedSec() << "s");    

    // retain ownership!
    m_chan.reset(c);

    // we now will listen to this channel
    m_chan->setListener(this);

    // now let's call our listener
    if (m_listener) {
        m_listener->initialized(this, m_service, m_version, apiVersion);
    }
}

void
Controller::channelEnded(bp::ipc::Channel * c,
                         bp::ipc::IConnectionListener::TerminationReason why,
                         const char * errorString)
{
    // no need to poll process spawning status, it exited
    m_spawnCheckTimer.cancel();

	// the channel fell down!  we'll invoke our listener
    
    // TODO: we should wait for the child process and return useful
    // information, like processor used, etc.

    if (m_listener) {
        m_listener->onEnded(this);
    }
}

void
Controller::onMessage(bp::ipc::Channel *, const bp::ipc::Message & m)
{
    if (!m.command().compare("callback"))
    {
        if (!m.payload() ||
            !m.payload()->has("tid", BPTInteger) ||
            !m.payload()->has("id", BPTInteger) ||
            !m.payload()->has("instance", BPTInteger))
        {
            BPLOG_ERROR_STRM("Malformed IPC response to "
                             << m.command() << " message");
            // TODO: should something beyond logging be
            //       performed for protocol errors?
        } else if (m_listener == NULL) {

        } else {
            long long tid, callback, instance;
            instance = *(m.payload()->get("instance"));
            tid = *(m.payload()->get("tid"));
            callback = *(m.payload()->get("id"));
            const bp::Object * value = m.payload()->get("value");
            m_listener->onCallback(this, (unsigned int) instance,
                                   (unsigned int) tid,
                                   (unsigned int) callback, value);
        }
    }
    else if (!m.command().compare("promptUser"))
    {
        if (!m.payload() ||
            !m.payload()->has("id", BPTInteger) ||
            !m.payload()->has("path", BPTString) ||
            !m.payload()->has("instance", BPTInteger))
        {
            BPLOG_ERROR_STRM("Malformed IPC payload for "
                             << m.command() << " query");
            // TODO: should something beyond logging be
            //       performed for protocol errors?
        }
        else if (m_listener == NULL)
        {
            BPLOG_WARN_STRM("IPC query received ("
                             << m.command() << ") with null listener, "
                            "dropping...");
        }
        else
        {
            long long int promptId = *(m.payload()->get("id"));
            long long int instance = *(m.payload()->get("instance"));
            bpf::Path path(std::string(*(m.payload()->get("path"))));
            const bp::Object * arguments = m.payload()->get("arguments");
            m_listener->onPrompt(this, (unsigned int) instance,
                                 (unsigned int) promptId, path, arguments);
        }
    }
    
}

bool
Controller::onQuery(bp::ipc::Channel *, const bp::ipc::Query & q,
                    bp::ipc::Response &)
{
    BPLOG_INFO_STRM("Received IPC query: " << q.command());    
    return false;
}

void
Controller::onResponse(bp::ipc::Channel *,
                       const bp::ipc::Response & response)
{
    BPLOG_DEBUG_STRM("Received IPC response: " << response.command());    

    if (!response.command().compare("getDescription")) {
        bp::service::Description d;
        if (!response.payload() || !d.fromBPObject(response.payload())) {
            BPLOG_ERROR_STRM("Malformed IPC response to "
                             << response.command() << " query");
            // TODO: should something beyond logging be
            //       performed for protocol errors?
        } else if (m_listener != NULL) {
            m_listener->onDescribe(this, d);
        }
    } else if (!response.command().compare("allocate")) {
        if (!response.payload() || response.payload()->type() != BPTInteger)
        {
            BPLOG_ERROR_STRM("Malformed IPC response to "
                             << response.command() << " query");
            // TODO: should something beyond logging be
            //       performed for protocol errors?
        } else if (m_listener != NULL) {
            m_listener->onAllocated(
                this, response.responseTo(),
                (unsigned int) (long long int) *(response.payload()));
        }
    }
    else if (!response.command().compare("invoke"))
    {
        // first let's pars e the payload.  we expect a 'success' member
        // which is a boolean.
        if (!response.payload() ||
            !response.payload()->has("success", BPTBoolean) ||
            !response.payload()->has("instance", BPTInteger)) 
        {
            BPLOG_ERROR_STRM("Malformed IPC response to "
                             << response.command() << " query");
        } else if (!m_listener) {
            BPLOG_WARN_STRM("IPC response received ("
                             << response.command() << ") with null listener, "
                            "dropping...");
        } else {
            bool success = *(response.payload()->get("success"));
            unsigned int instance = (unsigned int)
                (long long) *(response.payload()->get("instance"));
            if (success) {
                m_listener->onInvokeResults(this, instance,
                                            response.responseTo(),
                                            response.payload()->get("results"));
            } else {
                std::string error("bp.unknownError");
                std::string verboseError;                
                const bp::Object * o = response.payload();
                if (o->has("error")) {
                    error = (std::string) *(o->get("error"));
                }
                if (o->has("verboseError")) {
                    verboseError = (std::string) *(o->get("verboseError"));
                }
                m_listener->onInvokeError(this, instance,
                                          response.responseTo(),
                                          error, verboseError);
            }
        }
    } else {
        BPLOG_ERROR_STRM("Received unhandled IPC response: "
                         << response.command());
        // TODO: should something beyond logging be
        //       performed for protocol errors?
    }
}
