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
 * An abstraction around the service side of the IPC protocol spoken
 * with spawned services.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/15
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "ServiceProtocol.h"
#include "BPUtils/BPLog.h"

using namespace ServiceRunner;

ServiceProtocol::ServiceProtocol(ServiceLibrary * lib,
                                 bp::runloop::RunLoop * rl,
                                 const std::string & ipcName)
    : m_lib(lib), m_rl(rl), m_ipcName(ipcName)
{
    m_chan.setListener(this);
    m_lib->setListener(this);
}

bool
ServiceProtocol::connect()
{
    if (!m_chan.connect(m_ipcName))
    {
        return false;
    }

    // now send an initial message containing the service library's
    // name and version 
    bp::ipc::Message m;
    m.setCommand("loaded");
    bp::Map * payload = new bp::Map;
    payload->add("service", new bp::String(m_lib->name()));
    payload->add("version", new bp::String(m_lib->version()));    
    payload->add("apiVersion", new bp::Integer(m_lib->apiVersion()));

    m.setPayload(payload);
    
    if (!m_chan.sendMessage(m)) return false;

    return true;
}

ServiceProtocol::~ServiceProtocol()
{
}

void
ServiceProtocol::channelEnded(
    bp::ipc::Channel *,
    bp::ipc::IConnectionListener::TerminationReason,
    const char *)
{
    BPLOG_ERROR("connection ended!");
    m_rl->stop();
}

void
ServiceProtocol::onMessage(bp::ipc::Channel *, const bp::ipc::Message & m)
{
    if (!m.command().compare("destroy"))
    {
        if (NULL == m.payload() || m.payload()->type() != BPTInteger)
        {
            BPLOG_ERROR_STRM("Malformed IPC message to " << m.command()
                             << " message");
            // TODO: should something beyond logging be
            //       performed for protocol errors?
        } else {
            unsigned int id = (unsigned int) (long long) *(m.payload());
            m_lib->destroy(id);
        }
    }
    else if (!m.command().compare("promptResponse"))
    {
        if (NULL == m.payload() ||
            !m.payload()->has("promptId", BPTInteger))
        {
            BPLOG_ERROR_STRM("Malformed IPC message to " << m.command()
                             << " message");
            // TODO: should something beyond logging be
            //       performed for protocol errors?
        } else {
            unsigned int promptId  =
                (unsigned int) (long long) *(m.payload()->get("promptId"));
            const bp::Object * arguments = m.payload()->get("arguments");

            m_lib->promptResponse(promptId, arguments);
        }
    }
}

bool
ServiceProtocol::onQuery(bp::ipc::Channel *, const bp::ipc::Query & query,
                         bp::ipc::Response & response)
{
    if (!query.command().compare("invoke"))
    {
        // validate
        if (NULL == query.payload() ||
            query.payload()->type() != BPTMap ||
            !query.payload()->has("function", BPTString) ||
            !query.payload()->has("instance", BPTInteger))
        {            
            BPLOG_ERROR_STRM("Malformed IPC payload to " << query.command()
                             << " query");
            return true;
        }
        
        std::string err;
        if (!m_lib->invoke(
                (unsigned int) (long long) *(query.payload()->get("instance")),
                query.id(),
                (std::string) *(query.payload()->get("function")),
                query.payload()->get("arguments"),
                err))
        {
            BPLOG_ERROR_STRM("Service method invocation fails: "
                             << err);
        }
    }
    else if (!query.command().compare("getDescription"))
    {
        const bp::service::Description & desc = m_lib->description();
        bp::Object * obj = desc.toBPObject();
        response.setPayload(obj);
        return true;
    }
    else if (!query.command().compare("allocate"))
    {
        // extract context map
        bp::Map context;
        if (query.payload() && query.payload()->type() == BPTMap) {
            context = *((bp::Map *) query.payload());
        }
        unsigned int id = m_lib->allocate(context);
        response.setPayload(new bp::Integer(id));
        return true;
    }
    else
    {
        BPLOG_ERROR_STRM("unsupported query received: " << query.command());
    }
        
    return false;
}

void
ServiceProtocol::onResponse(bp::ipc::Channel *,
                            const bp::ipc::Response & r)
{
	BPLOG_ERROR_STRM("unexpected response received: "
		             << r.command());
}

void
ServiceProtocol::onResults(unsigned int instance, unsigned int tid,
                           const bp::Object * o)
{
    bp::ipc::Response r(tid);
    r.setCommand("invoke");
    bp::Map * m = new bp::Map;
    m->add("success", new bp::Bool(true));
    m->add("instance", new bp::Integer(instance));
    if (o) m->add("results", o->clone());
    r.setPayload(m);
    m_chan.sendResponse(r);
}

void
ServiceProtocol::onError(unsigned int instance,
                         unsigned int tid,
                         const std::string & error,
                         const std::string & verboseError)
{
    bp::ipc::Response r(tid);
    r.setCommand("invoke");
    bp::Map * m = new bp::Map;
    m->add("success", new bp::Bool(false));
    m->add("instance", new bp::Integer(instance));
    if (!error.empty()) {
        m->add("error", new bp::String(error));
    } else {
        m->add("error", new bp::String("bp.unknownError"));
    }
    if (!verboseError.empty()) {
        m->add("verboseError", new bp::String(verboseError));
    }
    r.setPayload(m);
    m_chan.sendResponse(r);
}

void
ServiceProtocol::onPrompt(unsigned int instance,
                          unsigned int promptId,
                          const bp::file::Path & pathToDialog,
                          const bp::Object * arguments)
{
    bp::ipc::Message m;
    m.setCommand("promptUser");
    bp::Map p;
    p.add("instance", new bp::Integer(instance));
    p.add("id", new bp::Integer(promptId));
    p.add("path", new bp::String(pathToDialog.utf8()));    
    if (arguments) p.add("arguments", arguments->clone());
    m.setPayload(p.clone());
    m_chan.sendMessage(m);    
}

void
ServiceProtocol::onCallback(unsigned int instance,
                            unsigned int tid,
                            long long int callbackId,
                            const bp::Object * o)
{
    bp::ipc::Message m;
    m.setCommand("callback");
    bp::Map * p = new bp::Map;
    p->add("instance", new bp::Integer(instance));
    p->add("tid", new bp::Integer(tid));
    p->add("id", new bp::Integer(callbackId));
    if (o) {
        p->add("value", o->clone());
    }
    m.setPayload(p);
    m_chan.sendMessage(m);
}

