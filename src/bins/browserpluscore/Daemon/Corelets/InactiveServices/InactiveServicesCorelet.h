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
 * InactiveServicesCorelet
 *
 * A built-in which allows for querying of distribution servers
 */

#ifndef __INACTIVESERVICESCORELET_H__
#define __INACTIVESERVICESCORELET_H__

#include "DistributionClient/DistributionClient.h"
#include "CoreletManager/CoreletManager.h"
//#include "BPUtils/BPUtils.h"

class InactiveServicesCorelet : virtual public CoreletInstance,
                                virtual public IDistQueryListener
{
public:
    InactiveServicesCorelet(
        std::tr1::weak_ptr<CoreletExecutionContext> context);
    virtual ~InactiveServicesCorelet();    

    virtual void execute(unsigned int tid,
                         const std::string & function,
                         const bp::Object & args);

    /**
     * Because the "InactiveServices" corelet is a synthetic corelet, the
     * interface description is returned from this static rather
     * than dynamically determined from probing a plugin.
     * This keeps the corelet definition and implementation close
     * together.
     */
    static const bp::service::Description * getDescription();
    
  private:
    // implemented methods from IDistQueryListener interface
    void onTransactionFailed(unsigned int tid);
    void gotServiceDetails(unsigned int tid,
                           const bp::service::Description & desc);
    void gotAvailableServices(unsigned int tid,
                              const CoreletList & list);

    DistQuery * m_distQuery;

    // map distquery ids to transaction ids
    std::map<unsigned int, unsigned int> m_distToTransMap;
};

#endif
