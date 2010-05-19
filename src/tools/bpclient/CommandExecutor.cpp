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

// for json serialization

#include "CommandExecutor.h"
#include <iostream>
#include "BPUtils/bptypeutil.h"
#include "BPUtils/ServiceDescription.h"

#ifdef WIN32
#pragma warning(disable:4100)
#endif

static void printProtoError(BPErrorCode ec)
{
    std::cout << BPErrorCodeToString(ec) << " (" << ec << ")" << std::endl;
}

CommandExecutor::CommandExecutor()
{
    m_hand = BPAlloc();
    BPSetUserPromptCallback(m_hand, promptCB, m_hand);
}

CommandExecutor::~CommandExecutor()
{
    BPFree(m_hand);
}

void
CommandExecutor::connectCB(BPErrorCode ec, void * cookie,
                           const char * error, const char * verboseError)
{
    CommandExecutor * ce = (CommandExecutor *) cookie;
    std::cout << "connect returns async: ";
    printProtoError(ec);
    ce->onSuccess();
}

void
CommandExecutor::promptCB(void * handptr, const char * path, const BPElement *,
                          unsigned int tid)
{
    BPProtoHand hand = (BPProtoHand) handptr;
    std::cout << "Received request to prompt user: " << path << std::endl;
    bp::String s("AlwaysAllow");
    BPDeliverUserResponse(hand, tid, s.elemPtr());
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::connect)
{
    std::string uri = "bpclient://";
    uri.append(BPCLIENT_UUID);
    uri.append("/");
    uri.append(BPCLIENT_APPNAME);
    
    BPErrorCode ec = BPConnect(m_hand, uri.c_str(), "en",
                               "BrowserPlus command line client",
                               connectCB,
                               (void *) this);
    std::cout << "connect returns sync: ";
    printProtoError(ec);
}

static void
dumpResultsToTerminal(bp::Object * obj)
{
    if (obj != NULL) {
        std::cerr << obj->toPlainJsonString(true) << std::endl;
    }
}

static void
dumpResultsToTerminal(const BPElement * results)
{
    if (results != NULL) {
        bp::Object * obj = bp::Object::build(results);
        dumpResultsToTerminal(obj);
        delete obj;
    }
}

void
CommandExecutor::resultsCB(void * ptr, unsigned int tid,
                           BPErrorCode ec, const BPElement * results)
{
    std::cerr << "got response to query " << tid << ", status: "
              << BPErrorCodeToString(ec) << " (" << ec << ")" << std::endl;
    dumpResultsToTerminal(results);

    if (ptr) {
        CommandExecutor * ce = (CommandExecutor *) ptr;
        ce->onSuccess();
    }
}

void
CommandExecutor::invokeCB(void *, unsigned int /*tid*/,
                          long long int cb,
                          const BPElement * results)
{
    std::cerr << "Callback " << cb << " invoked: " << std::endl;
    dumpResultsToTerminal(results);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::execute)
{
    unsigned int tid = 0;
    
    bp::Object * obj = NULL;
    const BPElement * bpElem = NULL;

    // convert typed in JSON to BPElement
    if (tokens.size() == 4) {
        std::string jsonText(tokens[3]);
        obj = bp::Object::fromJsonString(jsonText);
        if (obj != NULL) bpElem = obj->elemPtr();
    }

    BPErrorCode ec = BPExecute(m_hand,
                               tokens[0].c_str(),
                               tokens[1].c_str(),
                               tokens[2].c_str(),
                               bpElem,
                               resultsCB,
                               this,
                               invokeCB,
                               NULL,
                               &tid);

    if (obj != NULL) delete obj;

    std::cerr << "executing query, tid: " << tid << " status: "
              << BPErrorCodeToString(ec) << " (" << ec << ")" << std::endl;

    if (ec != BP_EC_OK) onFailure();
}

void
CommandExecutor::descriptionCB(BPErrorCode ec,
                               void * cookie,
                               const BPCoreletDefinition * def,
                               const char * error,
                               const char * verboseError)
{
    CommandExecutor * cmdexec = (CommandExecutor *) cookie;

    // let's output the definition
    if (ec != BP_EC_OK)
    {
        std::cout << "error: ";
        printProtoError(ec);
    }
    else if (def == NULL)
    {
        std::cout << "null corelet definition" << std::endl;
    }
    else
    {
        bp::service::Description d;
        d.fromBPCoreletDefinition(def);
        std::cout << d.toHumanReadableString();
    }

    cmdexec->onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::describe)
{
    BPErrorCode ec = BPDescribe(m_hand,
                                tokens[0].c_str(),
                                tokens[1].c_str(),
                                NULL,
                                descriptionCB,
                                (void *) this);

    if (ec != BP_EC_OK) onFailure();
}

void
CommandExecutor::requireCB(BPErrorCode ec,
                           void * cookie,
                           const BPCoreletDefinition ** defs,
                           unsigned int numDefs,
                           const char * error,
                           const char * verboseError)
{
    CommandExecutor * cmdexec = (CommandExecutor *) cookie;

    // let's output the definition
    if (ec != BP_EC_OK)
    {
        std::cout << "error: ";
        if (ec == BP_EC_EXTENDED_ERROR) {
            std::cout << error;
            if (verboseError) {
                std::cout << ": " << verboseError;
            }
            std::cout << std::endl;
        } else {
            printProtoError(ec);
        }
    }
    else if (defs == NULL)
    {
        std::cout << "null corelet definitions" << std::endl;
    }
    else
    {
        for (unsigned int di = 0; di < numDefs; di++) 
        {
            bp::service::Description d;
            d.fromBPCoreletDefinition(defs[di]);
            std::cout << d.toHumanReadableString();
        }
    }

    cmdexec->onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::require)
{
    std::string errBuf;
    bp::Object * obj = bp::Object::fromPlainJsonString(tokens[0], &errBuf);

    if (obj == NULL) {
        std::cout << "parse error: " << std::endl
                  << errBuf.c_str() << std::endl;
        onFailure();
        return;
    }
    
    bp::Map m;
    m.add("services", obj);
    
    BPErrorCode ec = BPRequire(
        m_hand,
        m.elemPtr(), 
        requireCB,
        (void *) this,
        NULL, NULL, NULL);

    if (ec != BP_EC_OK) {
        printProtoError(ec);
        onFailure();
    }
}


static char *
getStringValue(const bp::Object * obj)
{
    const char * str = NULL;
    if (obj != NULL) {
        const bp::String * s = dynamic_cast<const bp::String *>(obj);
        if (s != NULL) {
            str = s->value();
        }
    }
    return (BPString) str;
}

void
CommandExecutor::installedCB(BPErrorCode ec,
                             void * cookie,
                             const BPElement * corelets)
{
    CommandExecutor * cmdexec = (CommandExecutor *) cookie;

    // print out the list of corelets.
    if (ec != BP_EC_OK) {
        printProtoError(ec);
    } else if (corelets != NULL && corelets->type == BPTList) {
        std::cout << corelets->value.listVal.size
                  << " corelets installed:"  << std::endl;

        bp::List * clets =
            dynamic_cast<bp::List *>( bp::Object::build(corelets) );

        for (unsigned int i = 0; i < clets->size(); i++)
        {
            const bp::Object * m = clets->value(i);
            const char * name = getStringValue(m->get("name"));
            const char * version = getStringValue(m->get("version"));        
        
            std::cout << i + 1 << ": " << (name ? name : "(null)") << " - ver. "
                      << (version ? version : "(null)") << std::endl;
        }
    } else {
        std::cout << "error, unexpected response from protocol library"
                  << std::endl;
    }

    cmdexec->onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::enumerate)
{
    BPErrorCode ec = BPEnumerate(m_hand, installedCB, (void *) this);
    if (ec != BP_EC_OK) onFailure();        
}

void
CommandExecutor::stateCB(BPErrorCode ec, void * cookie,
                            const BPElement * response)
{
    CommandExecutor * cmdexec = (CommandExecutor *) cookie;

    // print out the list of corelets.
    if (ec != BP_EC_OK) {
        printProtoError(ec);
    } else if (response != NULL) {
        bp::Object * obj = bp::Object::build(response);
        dumpResultsToTerminal(obj);
        if (obj != NULL) delete obj;
    } else {
        std::cout << "error, unexpected response from protocol library"
                  << std::endl;
    }

    cmdexec->onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::getState)
{
    BPErrorCode ec = BPGetState(m_hand, tokens[0].c_str(),
                                stateCB, (void *) this);
    if (ec != BP_EC_OK) onFailure();        
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::setState)
{
    // second token is plain json value
    bp::Object * obj = NULL;
    const BPElement * bpElem = NULL;
    
    // convert typed in JSON to BPElement
    std::string jsonText(tokens[1]);
    obj = bp::Object::fromJsonString(jsonText);
    if (obj != NULL) bpElem = obj->elemPtr();

    BPErrorCode ec = BPSetState(m_hand, tokens[0].c_str(), bpElem);

    if (obj != NULL) delete obj;
    if (ec != BP_EC_OK) {
        onFailure();
    } else {
        onSuccess();
    }
}
