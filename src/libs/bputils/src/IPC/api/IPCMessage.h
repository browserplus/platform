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

#ifndef __IPCMESSAGE_H__
#define __IPCMESSAGE_H__

#include "BPUtils/bptypeutil.h"

#include <string>

namespace bp { namespace ipc {

/**
 * an ipc message is a bp::Map with some semnatics built on top.
 * All Messages have a "command", and a "payload".
 */ 
class Message : protected bp::Map {
  public:
    Message();
    virtual ~Message() { }
    
    // COPY/ASSIGNMENT supported via compiler generated functions.

    // access the command
    std::string command() const;
    void setCommand(const std::string & s);    

    // access the payload
    const bp::Object * payload() const;
    // the client retains ownership of the payload memory
    void setPayload(const bp::Object & payload);    
    // the Message instance attains ownership of the payload memory
    void setPayload(bp::Object * payload); // may throw

    // serialize a message into a string that may be transmitted
    std::string serialize() const { return bp::Map::toJsonString(false); }

    // serialize to a plain json string for display in log messages
    std::string toHuman() const { return bp::Map::toPlainJsonString(false); }
};

/**
 * A query is a Message with a unique, sender-generated id.
 */
class Query : public Message {
  public:
    Query();
    virtual ~Query() { }
    // COPY/ASSIGNMENT supported via compiler generated functions.

    // access the message id.  The id is unique process wide and is
    // set at Message allocation time
    unsigned int id() const;
};

/**
 * A response is a Message with a "response-to" header, containing the
 * id of the Query which elicited this response.
 */
class Response : public Message {
  private:
    Response();
  public:
    Response(unsigned int responseTo);
    virtual ~Response() { }
    // COPY/ASSIGNMENT supported via compiler generated functions.

    // access the message id.  The id is unique process wide and is
    // set at Message allocation time
    unsigned int responseTo() const;
};

// read a message from a string.  depending on the type of message,
// (query/response/message) the appropriate return type will be non-null
// message memory is dynamically allocated and the client is responsible
// for freeing.  false is returned on error
bool readFromString(const std::string & s,
                    Message ** oMessage,
                    Query ** oQuery,
                    Response ** oResponse);

bool readFromString(const unsigned char * msg,
                    unsigned int msg_len,
                    Message ** oMessage,
                    Query ** oQuery,
                    Response ** oResponse);


} };

#endif
