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

#include <iostream>
#include "CommandExecutor.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bpfile.h"
#include "CoreletManager/CoreletManager.h"

#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace std::tr1;


#ifdef WIN32
#pragma warning(disable:4100)
#endif
    
CommandExecutor::CommandExecutor(const std::string & ll,
                                 const bp::file::Path & lf)
    : CommandHandler(), m_servMan(new DynamicServiceManager(ll, lf)),
      m_currentTid(1)
{
    m_instanceMan.reset(new InstanceManager(this));

    m_servMan->setPluginDirectory(bp::paths::getCoreletDirectory());
}

CommandExecutor::~CommandExecutor()
{
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::available)
{
    unsigned int i;
    std::vector<bp::service::Description> v;

    v = m_servMan->availableServices();

	std::cout << v.size() << " services installed: " << std::endl;
    for (i = 0; i < v.size(); i++)
    {	
        std::cout << (i+1) << ": " << v[i].name() << " - "
                  << v[i].versionString() << std::endl;
    }

    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::have)
{
    std::string name = tokens[0];
    std::string version = (tokens.size() > 1 ? tokens[1] : std::string());
    std::string minversion = (tokens.size() > 2 ? tokens[2] : std::string());

    if (m_servMan->haveService(name, version, minversion)) {
        std::cout << "There IS an installed service satisfying: " << std::endl;
    } else {
        std::cout << "There is NO installed service satisfying: " << std::endl;
    }
    std::cout << "name:       " << name << std::endl;
    if (!version.empty()) {
        std::cout << "version:    " << version << std::endl;
    }
    if (!minversion.empty()) {
        std::cout << "minversion: " << minversion << std::endl;
    }

    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::summarize)
{
    std::string name = tokens[0];
    std::string version = (tokens.size() > 1 ? tokens[1] : std::string());
    std::string minversion = (tokens.size() > 2 ? tokens[2] : std::string());
    bp::service::Summary summary;
    
    if (m_servMan->summary(name, version, minversion, summary))
    {
        std::cout << summary.toHumanReadableString();
    }
    else 
    {
        std::cout << "no installed service found which satisfies the "
                  << "requirments" << std::endl;
    }
        
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::describe)
{
    std::string name = tokens[0];
    std::string version = (tokens.size() > 1 ? tokens[1] : std::string());
    std::string minversion = (tokens.size() > 2 ? tokens[2] : std::string());
    bp::service::Description description;
    
    if (m_servMan->describe(name, version, minversion, description))
    {
        std::cout << description.toHumanReadableString();
    }
    else 
    {
        std::cout << "no installed service found which satisfies the "
                  << "requirements" << std::endl;
    }
        
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::instantiate)
{
    std::string name = tokens[0];
    std::string version = (tokens.size() > 1 ? tokens[1] : std::string());

    // we let the client supply a sloppy specification (i.e. omit version)
    // and automatically discover the latest available name and version
    bp::service::Description description;
    if (m_servMan->describe(name, version, std::string(), description))
    {
        name = description.name();
        version = description.versionString();        
    }
    else 
    {
        std::cout << "no installed service found which satisfies the "
                  << "requirements" << std::endl;
        onFailure();
        return;
    }

    // now let's begin the async allocation
    (void) m_servMan->instantiate(
        name, version,
        weak_ptr<CoreletExecutionContext>(m_instanceMan),
        weak_ptr<ICoreletRegistryListener>(m_instanceMan));
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::destroy)
{
    m_instanceMan->destroy(atoi(tokens[0].c_str()));
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::execute)
{
    unsigned int instance = atoi(tokens[0].c_str());
    std::string function = tokens[1];
    bp::Map argMap;

    if (tokens.size() == 3) {
        std::string err;
        bp::Object * args = bp::Object::fromPlainJsonString(tokens[2], &err);
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

    // now find the instance
    shared_ptr<CoreletInstance> iPtr =
        m_instanceMan->findInstance(instance);

    if (iPtr == NULL) {
        std::cout << "no such allocated instance: " << instance << std::endl;
        onFailure();        
    } else {
        iPtr->execute(m_currentTid++, function, argMap);
    }
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::rescan)
{
    m_servMan->forceRescan();
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::purge)
{
    m_servMan->purgeService(tokens[0], tokens[1]);
    onSuccess();
}
