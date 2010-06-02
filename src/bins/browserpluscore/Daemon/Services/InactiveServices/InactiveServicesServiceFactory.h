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
 * InactiveServicesServiceFactory
 *
 * A factory implementation for the "core" service
 */

#ifndef __INACTIVESERVICESSERVICEFACTORY_H__
#define __INACTIVESERVICESSERVICEFACTORY_H__

#include "ServiceManager/ServiceManager.h"

class InactiveServicesServiceFactory : virtual public ServiceFactory
{
public:
    InactiveServicesServiceFactory();
    virtual ~InactiveServicesServiceFactory();

    virtual bp::service::BuiltInSummary summary();

    virtual std::tr1::shared_ptr<ServiceInstance>
        instantiateInstance(std::tr1::weak_ptr<ServiceExecutionContext> context);
  private:
    std::tr1::shared_ptr<ServiceRegistry> m_registry;
};

#endif
