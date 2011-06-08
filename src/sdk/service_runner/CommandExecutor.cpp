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
#include "platform_utils/ProductPaths.h"
#include "Output.h"


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

    // handle an optional argument which is the URI we should
    // pass, a change that would allow for testing of cross domain
    // checks among other things.
    if (tokens.size() == 1) {
        uri = tokens[0];
    }

    boost::filesystem::path data_dir = 
        bp::paths::getServiceDataDirectory(m_desc.name(),
                                           m_desc.majorVersion());
    boost::filesystem::path tmpdir = bp::file::getTempDirectory();
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

// utility functions which traverses a bpobject and
// maps strings with the prefix of path: into Paths,
// and those with the prefix of writable_path: into
// WritablePaths
static bp::Object * cloneOrConvert(const bp::Object * o) 
{
    static std::string uriPrefix("file://");
    static std::string pathPrefix("path:");
    static std::string writablePathPrefix("writable_path:");
    
    if (!o) return NULL;
    bp::Object * rv = NULL;
    const bp::String* str = dynamic_cast<const bp::String*>(o);
    if (str) {
        // get the string out of the argList
		std::string val;
        if (str->value()) val.append(str->value());
        if (!val.compare(0, pathPrefix.size(), pathPrefix)) {
            boost::filesystem::path path(val.substr(pathPrefix.size()));
            rv = new bp::Path(path);
        } else if (!val.compare(0, writablePathPrefix.size(), writablePathPrefix)) {
            boost::filesystem::path path(val.substr(writablePathPrefix.size()));
            rv = new bp::WritablePath(path);
        } else if (!val.compare(0, uriPrefix.size(), uriPrefix)) {
            boost::filesystem::path path(val.substr(uriPrefix.size()));
            rv = new bp::Path(path);
        }
    }

    return (rv ? rv : o->clone());
}

static bp::Object * replacePaths(const bp::Object * o)
{
    if (NULL == o) return NULL;

    if (o->type() == BPTMap) {
        const bp::Map* argMap = dynamic_cast<const bp::Map *>(o);
        if (argMap) {        
            bp::Map* newMap = new bp::Map;
            bp::Map::Iterator i(*argMap);
            const char * k;
            while (NULL != (k = i.nextKey())) {
                newMap->add(k, replacePaths(argMap->value(k)));
            }
            return newMap;
        }

    } else if (o->type() == BPTList) {
        const bp::List* argList = dynamic_cast<const bp::List *>(o);
        if (argList) {        
            bp::List* newList = new bp::List;
            for (size_t i = 0; i < argList->size(); i++) {
                newList->append(replacePaths(argList->value(i)));
            }
            return newList;
        }
    }

    return cloneOrConvert(o);
}


BP_DEFINE_COMMAND_HANDLER(CommandExecutor::invoke)
{
    bp::Map rawArgMap;
    if (tokens.size() == 2) {
        std::string err;
        bp::Object * args = bp::Object::fromPlainJsonString(tokens[1], &err);
        if (!args) {
            std::stringstream ss;
            ss << "couldn't parse json:" << err.c_str();
            output::puts(output::T_ERROR, ss.str());
            onFailure();        
            return;
        } else if (args->type() != BPTMap) {
            output::puts(output::T_ERROR, "provided json must specify a map (aka 'object')");
            onFailure();        
            return;
        }
        rawArgMap = *((bp::Map *) args);
        delete args;
    }

    // now we must traverse the service description and automatically
    // provide callback arguments, and automatically convert file://
    // urls or native paths into BPPaths
    bp::service::Function f;
    if (!m_desc.getFunction(tokens[0].c_str(), f)) {
        std::stringstream ss;
        ss << "no such function: " << tokens[0];
        output::puts(output::T_ERROR, ss.str());
        onFailure();        
        return;
    }

    // first do a deep replacement of strings with file: and writable_file:
    // prefixes
    bp::Map * argMap = dynamic_cast<bp::Map *>(replacePaths(&rawArgMap));
    BPASSERT(argMap);
    
    // fixup callbacks and file arguments
    std::list<bp::service::Argument> l = f.arguments(); 
    std::list<bp::service::Argument>::iterator it;
    int callbackId = 1;
    for (it = l.begin(); it != l.end(); it++) {
        if (it->type() == bp::service::Argument::CallBack) {
            argMap->add(it->name(), new bp::CallBack(callbackId++));
        }
        // we allow the client to be lazy at the top level and
        // *not* specify a prefix (either path: or writable_path:),
        // because we know the types of the first level args
        else if (it->type() == bp::service::Argument::Path) {
            // get the string out of the argMap
            std::string str;
            if (argMap->getString(it->name().c_str(), str)) {
                boost::filesystem::path path(str);
                argMap->add(it->name(), new bp::Path(path));                
            }
        }
        else if (it->type() == bp::service::Argument::WritablePath) {
            // get the string out of the argMap
            std::string str;
            if (argMap->getString(it->name().c_str(), str)) {
                boost::filesystem::path path(str);
                argMap->add(it->name(), new bp::WritablePath(path));                
            }
        }
    }
    
    unsigned int instance = m_controlMan->currentInstance();    

    (void) m_controller->invoke(instance, tokens[0], argMap);

    if (argMap) delete argMap;
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::show)
{
    std::set<unsigned int> s = m_controlMan->instances();
    std::set<unsigned int>::iterator it;
    unsigned int c = m_controlMan->currentInstance();
    std::stringstream ss;
    
    ss << s.size() << " instance(s) allocated: ";
    bool firstRun = true;
    for (it = s.begin(); it != s.end(); it++) {
        if (!firstRun) ss << ",";
        ss << " " << (c == *it ? "(" : " ")
           << *it
           << (c == *it ? ")" : "");
        firstRun = false;
    }
    output::puts(output::T_RESULTS, ss.str());
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::prompts)
{
    std::set<unsigned int> s = m_controlMan->prompts();
    std::set<unsigned int>::iterator it;
    std::stringstream ss;

    ss << s.size() << " outstanding prompt(s): ";
    bool firstRun = true;
    for (it = s.begin(); it != s.end(); it++) {
        if (!firstRun) ss << ",";
        ss << " " << *it;
        firstRun = false;
    }
    output::puts(output::T_RESULTS, ss.str());
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::respond)
{
    bp::Object * args = NULL;
    if (tokens.size() == 2) {
        std::string err;
        args = bp::Object::fromPlainJsonString(tokens[1], &err);
        if (!args) {
            std::stringstream ss;
            ss << "couldn't parse json: " << err.c_str();
            output::puts(output::T_ERROR, ss.str());
            onFailure();        
            return;
        }
    }

    unsigned int promptId = (unsigned int) atoi(tokens[0].c_str());

    if (!m_controlMan->responded(promptId)) {
        std::stringstream ss;
        ss << "Warning: responding to unknown prompt Id: " << promptId;
        output::puts(output::T_WARNING, ss.str());
    }
    
    (void) m_controller->sendResponse(promptId, args);

    if (args) delete args;

    onSuccess();
}


BP_DEFINE_COMMAND_HANDLER(CommandExecutor::describe)
{
    m_controller->describe();
}
