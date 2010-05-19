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

#ifndef __INSTANCEMANAGER_H__
#define __INSTANCEMANAGER_H__

#include "CoreletManager/CoreletManager.h"

class InstanceManager : public CoreletExecutionContext,
                        public ICoreletRegistryListener,
                        public ICoreletInstanceListener,
                        public std::tr1::enable_shared_from_this<InstanceManager>
{
  public:
    InstanceManager(class CommandExecutor * cmdExec);
    ~InstanceManager();    
    
    void destroy(unsigned int allocationId);

    std::tr1::shared_ptr<CoreletInstance> findInstance(unsigned int id);

  private:
    void onAllocationSuccess(unsigned int allocationId,
                             std::tr1::shared_ptr<CoreletInstance> instance);
    
    void onAllocationFailure(unsigned int allocationId);

    void executionComplete(unsigned int tid, const bp::Object & results);

    void executionFailure( unsigned int tid, const std::string & error,
                           const std::string & verboseError);

    std::map<unsigned int, std::tr1::shared_ptr<CoreletInstance> > m_instances;
    CommandExecutor * m_cmdExec;
};

#endif
