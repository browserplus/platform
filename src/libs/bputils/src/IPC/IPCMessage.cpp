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

#include "api/IPCMessage.h"
#include "bperrorutil.h"
#include "bpsync.h"

using namespace bp::ipc;

// messages always have a command.  in the constructor we set up an empty
// command
Message::Message()
{
    add("command", new bp::String(""));
}


std::string
Message::command() const
{
    // may throw if the world is not sane
    return (*this)["command"];
}

void
Message::setCommand(const std::string & s)
{
    add("command", new bp::String(s));    
}

const bp::Object *
Message::payload() const
{
    return get("payload");
}

void
Message::setPayload(const bp::Object & payload)
{
    add("payload", payload.clone());
}

void
Message::setPayload(bp::Object * payload)
{
    if (payload == NULL) BP_THROW("invalid null argument");
    add("payload", payload);    
}

Query::Query() 
{
    unsigned int id = 0;
    // attain a process wide unique id.
    {
        static unsigned int currentId = 1000;
        static bp::sync::Mutex lock;
        lock.lock();
        id = currentId++;
        lock.unlock();
    }
    add("id", new bp::Integer(id));
}

unsigned int
Query::id() const
{
    return (unsigned int) (long long) (*this)["id"];
}


Response::Response(unsigned int responseTo)
{
    add("response-to", new bp::Integer(responseTo));
}

unsigned int 
Response::responseTo() const
{
    return (unsigned int) (long long) *get("response-to");
}

bool
bp::ipc::readFromString(const std::string & s,
                        Message ** oMessage,
                        Query ** oQuery,
                        Response ** oResponse)
{
    *oMessage = NULL;
    *oQuery = NULL;
    *oResponse = NULL;

    bp::Object * o = bp::Object::fromJsonString(s);
    if (o == NULL) return false;
    if (o->type() != BPTMap) {
        delete o;
        return false;
    }

    // now let's determine what type of message this is, first we'll validate
    // that all common fields are present
    if (!o->has("command", BPTString)) {
        delete o;
        return false;
    }

    // if a response-to field is present, this is a response
    if (o->has("response-to", BPTInteger)) {
        *oResponse = (Response *) o;
    } else if (o->has("id", BPTInteger)) {
        // queries have an id
        *oQuery = (Query *) o;
    } else {
        // everything else is a message
        *oMessage = (Message *) o;        
    }
    
    return true;
}

bool
bp::ipc::readFromString(const unsigned char * msg,
                        unsigned int msg_len,
                        Message ** oMessage,
                        Query ** oQuery,
                        Response ** oResponse)
{
    std::string s;
    s.append((const char *) msg, msg_len);
    return readFromString(s, oMessage, oQuery, oResponse);
}


