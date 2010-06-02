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

#include "InstanceManager.h"
#include <iostream>
#include "CommandExecutor.h"

using namespace std;
using namespace std::tr1;


InstanceManager::InstanceManager(CommandExecutor * cmdExec)
    : m_cmdExec(cmdExec)
{
}

InstanceManager::~InstanceManager()
{
}

void
InstanceManager::onAllocationSuccess(unsigned int allocationId,
                                     shared_ptr<ServiceInstance> instance)
{
    std::cout << "instance allocated: " << allocationId << std::endl;
    instance->setListener(shared_from_this());
    m_instances[allocationId] = instance;
    m_cmdExec->onSuccess();
}

void
InstanceManager::onAllocationFailure(unsigned int allocationId)
{
    // TODO: handle
}

shared_ptr<ServiceInstance>
InstanceManager::findInstance(unsigned int id)
{
    shared_ptr<ServiceInstance> ptr;
    std::map<unsigned int, shared_ptr<ServiceInstance> >::iterator it;
    if ((it = m_instances.find(id)) != m_instances.end()) ptr = it->second;
    return ptr;
}

void
InstanceManager::destroy(unsigned int allocationId)
{
    std::map<unsigned int, shared_ptr<ServiceInstance> >::iterator it;
    it = m_instances.find(allocationId);

    if (it == m_instances.end()) {
        std::cout << "no such instance: " << allocationId << std::endl;        
    } else {
        m_instances.erase(it);
        std::cout << "instance destroyed " << allocationId << std::endl;
    }
}

void
InstanceManager::executionComplete(unsigned int tid,
                                   const bp::Object & results)
{
    std::cout << results.toPlainJsonString(true) << std::endl;        
    m_cmdExec->onSuccess();
}


void
InstanceManager::executionFailure(unsigned int tid, const std::string & error,
                                  const std::string & verboseError)
{
    std::cout << "failure: (" << error << ") "
              << verboseError << std::endl;        
    m_cmdExec->onSuccess();
}

