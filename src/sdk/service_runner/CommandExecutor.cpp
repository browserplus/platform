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

#include "CommandExecutor.h"
#include <iostream>
#include "BPUtils/bpfile.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/ProductPaths.h"

using namespace std;
using namespace std::tr1;

#ifdef WIN32
#pragma warning(disable:4100)
#endif

CommandExecutor::CommandExecutor(
    shared_ptr<ServiceRunner::Controller> controller)
    : m_controller(controller)
{
    m_controlMan.reset(new ControllerManager(this));
}

CommandExecutor::~CommandExecutor()
{
}

void
CommandExecutor::start(const bp::service::Description & desc)
{
    m_desc = desc;
    m_controller->setListener(m_controlMan.get());
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::allocate)
{
    // set up allocate parameters
    std::string uri("bpclient://");
    uri.append(BPCLIENT_UUID);
    bp::file::Path data_dir = 
        bp::paths::getCoreletDataDirectory(m_desc.name(),
                                           m_desc.majorVersion());
    bp::file::Path tmpdir = bp::file::getTempDirectory();
    tmpdir = bp::file::getTempPath(tmpdir, m_desc.name());
    std::string locale = "en";
    std::string userAgent(BPCLIENT_APPNAME);
    long pid = bp::process::currentPid();

    m_controller->allocate(uri, data_dir, tmpdir, locale, userAgent,
                           (unsigned int) pid);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::destroy)
{
    m_controller->destroy(atoi(tokens[0].c_str()));
    m_controlMan->destroy(atoi(tokens[0].c_str()));
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::select)
{
    if (m_controlMan->select(atoi(tokens[0].c_str()))) onSuccess();
    else onFailure();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::invoke)
{
    bp::Map argMap;
    if (tokens.size() == 2) {
        std::string err;
        bp::Object * args = bp::Object::fromPlainJsonString(tokens[1], &err);
        if (!args) {
            std::cout << "couldn't parse json:" << std::endl
                      << err.c_str() << std::endl;
            onFailure();        
            return;
        } else if (args->type() != BPTMap) {
            std::cout << "provided json must specify a map (aka 'object')"
                      << std::endl;
            onFailure();        
            return;
        }
        argMap = *((bp::Map *) args);
        delete args;
    }

    // now we must traverse the service description and automatically
    // provide callback arguments, and automatically convert file://
    // urls or native paths into BPPaths
    bp::service::Function f;
    if (!m_desc.getFunction(tokens[0].c_str(), f)) {
        std::cout << "no such function: " << tokens[0] << std::endl;
        onFailure();        
        return;
    }
    
    // fixup callbacks and file arguments
    std::list<bp::service::Argument> l = f.arguments(); 
    std::list<bp::service::Argument>::iterator it;
    int callbackId = 1;
    for (it = l.begin(); it != l.end(); it++) {
        if (it->type() == bp::service::Argument::CallBack) {
            argMap.add(it->name(), new bp::CallBack(callbackId++));
        } else if (it->type() == bp::service::Argument::Path) {
            // get the string out of the argMap
            std::string str;
            if (argMap.getString(it->name().c_str(), str)) {
                bp::file::Path path;
                if (bp::url::isFileUrl(str)) {
                    path = bp::file::pathFromURL(str);
                }
                argMap.add(it->name(), new bp::Path(path));                
            }
        } else if (it->type() == bp::service::Argument::List) {
            // deal with lists of files
            const bp::List* argList = NULL;
            if (argMap.getList(it->name().c_str(), argList)) {
                bp::List* newList = new bp::List;
                for (size_t i = 0; i < argList->size(); i++) {
                    // get the string out of the argList
                    const bp::Object* obj = argList->value(i);
                    const bp::String* str = dynamic_cast<const bp::String*>(obj);
                    if (str && bp::url::isFileUrl(str->value())) {
                        bp::file::Path path = bp::file::pathFromURL(str->value());
                        newList->append(new bp::Path(path));
                    } else {
                        newList->append(obj->clone());
                    }
                }
                argMap.add(it->name(), newList);
            }
        }
    }

    unsigned int instance = m_controlMan->currentInstance();    

    (void) m_controller->invoke(instance, tokens[0], &argMap);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::show)
{
    std::set<unsigned int> s = m_controlMan->instances();
    std::set<unsigned int>::iterator it;
    unsigned int c = m_controlMan->currentInstance();

    std::cout << s.size() << " instance(s) allocated: ";
    bool firstRun = true;
    for (it = s.begin(); it != s.end(); it++) {
        if (!firstRun) std::cout << ",";
        std::cout << " " << (c == *it ? "(" : " ")
                  << *it
                  << (c == *it ? ")" : "");
        firstRun = false;
    }
    std::cout << std::endl;
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::prompts)
{
    std::set<unsigned int> s = m_controlMan->prompts();
    std::set<unsigned int>::iterator it;

    std::cout << s.size() << " outstanding prompt(s): ";
    bool firstRun = true;
    for (it = s.begin(); it != s.end(); it++) {
        if (!firstRun) std::cout << ",";
        std::cout << " " << *it;
        firstRun = false;
    }
    std::cout << std::endl;
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::respond)
{
    bp::Object * args = NULL;
    if (tokens.size() == 2) {
        std::string err;
        args = bp::Object::fromPlainJsonString(tokens[1], &err);
        if (!args) {
            std::cout << "couldn't parse json:" << std::endl
                      << err.c_str() << std::endl;
            onFailure();        
            return;
        }
    }

    unsigned int promptId = (unsigned int) atoi(tokens[0].c_str());

    if (!m_controlMan->responded(promptId)) {
        std::cout << "Warning: responding to unknown prompt Id: "
                  << promptId << std::endl;
    }
    
    (void) m_controller->sendResponse(promptId, args);

    if (args) delete args;

    onSuccess();
}


BP_DEFINE_COMMAND_HANDLER(CommandExecutor::describe)
{
    m_controller->describe();
}
