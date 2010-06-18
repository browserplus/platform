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

/**
 * InactiveServicesService
 *
 * A built-in which allows for querying of distribution servers
 */

#ifndef __INACTIVESERVICESSERVICE_H__
#define __INACTIVESERVICESSERVICE_H__

#include "DistributionClient/DistributionClient.h"
#include "ServiceManager/ServiceManager.h"
//#include "BPUtils/BPUtils.h"

class InactiveServicesService : virtual public ServiceInstance,
                                virtual public IDistQueryListener
{
public:
    InactiveServicesService(
        std::tr1::weak_ptr<ServiceExecutionContext> context);
    virtual ~InactiveServicesService();    

    virtual void execute(unsigned int tid,
                         const std::string & function,
                         const bp::Object & args);

    /**
     * Because the "InactiveServices" service is a synthetic service, the
     * interface description is returned from this static rather
     * than dynamically determined from probing a plugin.
     * This keeps the service definition and implementation close
     * together.
     */
    static const bp::service::Description * getDescription();
    
  private:
    // implemented methods from IDistQueryListener interface
    void onTransactionFailed(unsigned int tid,
                             const std::string& msg);
    void gotServiceDetails(unsigned int tid,
                           const bp::service::Description & desc);
    void gotAvailableServices(unsigned int tid,
                              const ServiceList & list);

    DistQuery * m_distQuery;

    // map distquery ids to transaction ids
    std::map<unsigned int, unsigned int> m_distToTransMap;
};

#endif
