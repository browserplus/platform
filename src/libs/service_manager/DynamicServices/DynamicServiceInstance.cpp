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

#include  "DynamicServiceInstance.h"
#include  "DynamicServiceManager.h"

using namespace std;
using namespace std::tr1;


DynamicServiceInstance::DynamicServiceInstance(
    weak_ptr<CoreletExecutionContext> context)
    : CoreletInstance(context), m_instanceId(0), 
      m_summary(), m_instantiateId(0), m_registryListener(),
      m_tidMap(), m_manager(NULL)
{
}

DynamicServiceInstance::~DynamicServiceInstance()
{
    m_manager->onInstanceShutdown(this);
}
    
void
DynamicServiceInstance::execute(unsigned int tid,
                                const std::string & function,
                                const bp::Object & args)
{
    m_manager->onInstanceExecute(this, tid, function, args);
}

bool
DynamicServiceInstance::mapToClientTid(unsigned int & tid)
{
    std::map<unsigned int, unsigned int>::iterator it;
    it = m_tidMap.find(tid);
    if (it == m_tidMap.end()) return false;
    tid = it->second;
    return true;
}

void
DynamicServiceInstance::addClientTid(unsigned int tid, unsigned int clientTid)
{
    m_tidMap[tid] = clientTid;
}

void
DynamicServiceInstance::removeTid(unsigned int tid)
{
    std::map<unsigned int, unsigned int>::iterator it = m_tidMap.find(tid);
    if (it != m_tidMap.end()) m_tidMap.erase(it);
}

void
DynamicServiceInstance::onUserResponse(unsigned int promptId,
                                       const bp::Object& response)
{
    m_manager->onPromptResponse(this, promptId, response);
}

