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

#include "ControllerManager.h"
#include "CommandExecutor.h"

#define NO_INSTANCE ((unsigned int) -1)

ControllerManager::ControllerManager(CommandExecutor * callback)
    : m_callback(callback), m_currentInstance(NO_INSTANCE)
{
}

ControllerManager::~ControllerManager()
{
}

void
ControllerManager::onDescribe(ServiceRunner::Controller *,
                              const bp::service::Description & desc)
{
    std::cout << desc.toHumanReadableString();
    m_callback->onSuccess();
}

void
ControllerManager::onAllocated(ServiceRunner::Controller *,
                               unsigned int aid, unsigned int id)
{
    m_instances.insert(id);
    std::cout << "allocated: " << id << std::endl;
    if (m_currentInstance == NO_INSTANCE) m_currentInstance = id;

    m_callback->onSuccess();    
}

void
ControllerManager::onInvokeResults(ServiceRunner::Controller *,
                                   unsigned int,
                                   unsigned int,
                                   const bp::Object * results)
{
    if (results) {
        std::cout << results->toPlainJsonString(true);
        std::cout.flush();
    }
    m_callback->onSuccess();        
}

void
ControllerManager::onInvokeError(ServiceRunner::Controller *,
                                 unsigned int,
                                 unsigned int,
                                 const std::string & error,
                                 const std::string & verboseError)
{
    std::cout << "error: (" << error << ") " << verboseError << std::endl;
    m_callback->onFailure();
}

void
ControllerManager::onCallback(ServiceRunner::Controller *,
                              unsigned int,
                              unsigned int, long long int cid,
                              const bp::Object * v)
{
    // TODO: We should trun the cid into a human readable name
    std::cout << "callback argument "<< cid <<" invoked: "<< std::endl;
    if (v) std::cout << v->toPlainJsonString(true);
}

void
ControllerManager::onPrompt(ServiceRunner::Controller *,
                            unsigned int,
                            unsigned int promptId,
                            const bp::file::Path & pathToDialog,
                            const bp::Object * arguments)
{
    std::cout << "Prompt received (" << promptId
              << "), html at: " << pathToDialog << std::endl;    
    if (arguments) {
        std::cout << "arguments: " << arguments->toPlainJsonString(true);
    }

    std::cout << "(use 'respond' command to send a response to this prompt)"
              << std::endl;

    m_prompts.insert(promptId);

    m_callback->onSuccess();
}

bool
ControllerManager::responded(unsigned int promptId)
{
    std::set<unsigned int>::iterator it;
    it = m_prompts.find(promptId);
    if (it == m_prompts.end()) return false;
    m_prompts.erase(it);
    return true;
}

void
ControllerManager::destroy(unsigned int id)
{
    std::set<unsigned int>::iterator it = m_instances.find(id);
    if (it != m_instances.end()) m_instances.erase(it);

    // now if that was the active instance, let's reset the active
    // instance correctly
    if (id == m_currentInstance) {
        if (m_instances.size() > 0) {
            m_currentInstance = *(m_instances.begin());
        } else {
            m_currentInstance = NO_INSTANCE;
        }
    }
}

bool
ControllerManager::select(unsigned int id)
{
    std::set<unsigned int>::iterator it = m_instances.find(id);
    if (it != m_instances.end()) {
        m_currentInstance = id;
        return true;
    }
    
    return false;
}

unsigned int
ControllerManager::currentInstance()
{
    return m_currentInstance;
}

std::set<unsigned int>
ControllerManager::instances()
{
    return m_instances;
}

std::set<unsigned int>
ControllerManager::prompts()
{
    return m_prompts;
}

void
ControllerManager::onEnded(ServiceRunner::Controller *)
{
    std::cout << "spawned process ended" << std::endl;
}
