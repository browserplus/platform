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

#ifndef __DYNAMICSERVICEINSTANCE_H__
#define __DYNAMICSERVICEINSTANCE_H__

#include "ServiceRunnerLib/ServiceRunnerLib.h"
#include "CoreletManager/CoreletInstance.h"
#include "CoreletManager/CoreletRegistry.h"

// a dumb shim class that represents a client's handle on a service
class DynamicServiceInstance : public CoreletInstance
{
  public:
    ~DynamicServiceInstance();

    void execute(unsigned int tid,
                 const std::string & function,
                 const bp::Object & args);
  private:
    DynamicServiceInstance(std::tr1::weak_ptr<CoreletExecutionContext> context);

    unsigned int m_instanceId;

    bp::service::Summary m_summary;
    
    friend class DynamicServiceManager;
    friend class DynamicServiceState;

    // an id returned from DynamicServiceManager::allocate that allows
    // clients to correctly associate allocations.
    // after passing ownership of the service back to the registry
    // listener, the instantiate id and listener are not used
    unsigned int m_instantiateId;
    std::tr1::weak_ptr<ICoreletRegistryListener> m_registryListener;

    // a map which maps underlying (servicerunner) transaction ids onto
    // client selected transaction ids.
    std::map<unsigned int, unsigned int> m_tidMap;

    // some utility functions to ease management of tid mapping
    // tid is an in/out param
    bool mapToClientTid(unsigned int & tid);
    void addClientTid(unsigned int tid, unsigned int clientTid);
    void removeTid(unsigned int tid);

    // used by destructor to call back into manager
    DynamicServiceManager * m_manager;

    virtual void onUserResponse(unsigned int, const bp::Object&);
};

#endif
