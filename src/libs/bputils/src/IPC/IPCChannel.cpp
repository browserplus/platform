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
 *  IPCChannel.h - higher level abstraction to move messages between
 *                 processes running on the same machine.  This includes
 *                 an async single threaded model supporting both 
 *                 query/repsonse and fire & forget semantics.
 */

#include "api/IPCChannel.h"
#include "BPUtils/bperrorutil.h"

#include <stdlib.h>

using namespace bp::ipc;

struct ChannelEvent 
{
    Channel * c;
    Message * m;    
    Query * q;    
    Response * r;
};

void 
Channel::deliverMessageEvent(void * ctx)
{
    ChannelEvent * ce = (ChannelEvent *) ctx;
    BPASSERT(ce != NULL);
    IChannelListener * l = ce->c->m_cListener;

    if (ce->m) {
        l->onMessage(ce->c, *(ce->m));
        delete ce->m;
    } else if (ce->q) {
        Response resp(ce->q->id());
        resp.setCommand(ce->q->command());
        if (l->onQuery(ce->c, *(ce->q), resp)) {
            ce->c->sendResponse(resp);
        }
        delete ce->q;
    } else if (ce->r) {
        l->onResponse(ce->c, *(ce->r));
        delete ce->r;
    }

    free(ctx);
}

void
Channel::gotMessage(const class Connection *,
                    const unsigned char * msg,
                    unsigned int msg_len)
{
    // parse, determine what this thing is, and call the correct function
    ChannelEvent * ce = (ChannelEvent *) calloc(1, sizeof(ChannelEvent));
    ce->c = this;
    
    if (!readFromString(msg, msg_len, &(ce->m), &(ce->q), &(ce->r))) {
        // TODO: How might we handle failure?
    }

    // now marshal ce over to the correct thread for delivery
    m_hopper.invokeOnThread(deliverMessageEvent, ce);
    // TODO: How might we handle failure?
}

struct ChannelEndedEvent 
{
    Channel * c;
    bp::ipc::IConnectionListener::TerminationReason why;
    std::string errorString;
};

void 
Channel::deliverEndedEvent(void * ctx)
{
    ChannelEndedEvent * cee = (ChannelEndedEvent *) ctx;

    if (cee->c->m_cListener) {
        cee->c->m_cListener->channelEnded(cee->c, cee->why,
                                          cee->errorString.c_str());
    }

    delete cee;
}

void
Channel::connectionEnded(class Connection *,
                         TerminationReason why,
                         const char * errorString)
{
    ChannelEndedEvent * cee = new ChannelEndedEvent;
    cee->c = this;
    cee->why = why;
    if (errorString) cee->errorString.append(errorString);
    m_hopper.invokeOnThread(deliverEndedEvent, cee);
}


Channel::Channel()
    : m_conn(new Connection), m_cListener(NULL)
{
    if (!m_hopper.initializeOnCurrentThread()) {
        BP_THROW_FATAL("Couldn't initialize Channel threadhopper");
    }
}

Channel::Channel(Connection * c)
    : m_conn(c), m_cListener(NULL)
{
    if (!m_hopper.initializeOnCurrentThread()) {
        BP_THROW_FATAL("Couldn't initialize Channel threadhopper");
    }
}

Channel::~Channel()
{
    delete m_conn;
}

bool
Channel::connect(const std::string & location, std::string * error)
{
    return m_conn->connect(location, error);
}

bool
Channel::setListener(IChannelListener * listener)
{
    m_cListener = listener;
    if (listener == NULL) return m_conn->setListener(NULL);    
    return m_conn->setListener(this);    
}

void
Channel::disconnect()
{
    m_conn->disconnect();
}

bool
Channel::sendMessage(const Message & m)
{
    return m_conn->sendMessage(m.serialize());
}

bool
Channel::sendQuery(const Query & q)
{
    return m_conn->sendMessage(q.serialize());
}

bool
Channel::sendResponse(const Response & r)
{
    return m_conn->sendMessage(r.serialize());
}
