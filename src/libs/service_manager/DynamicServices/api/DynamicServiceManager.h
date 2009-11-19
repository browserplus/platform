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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * DynamicServiceManager
 *
 * An object responsible for loading and searching of dynamic
 * corelets.
 *
 */

#ifndef __DYNAMICSERVICEMANAGER_H__
#define __DYNAMICSERVICEMANAGER_H__

#include "ServiceRunnerLib/ServiceRunnerLib.h"
#include "CoreletInstance.h"
#include "CoreletExecutionContext.h"
#include "DynamicServiceInstance.h"
#include "DynamicServiceState.h"


/**
 * a dynamic service manager
 */
class DynamicServiceManager : public ServiceRunner::IControllerListener
{
  public:
    // Allocate a dynamic service manager passing in the loglevel
    // and logfile into which spawned services should log
    DynamicServiceManager(const std::string & loglevel,
                          const bp::file::Path & logfile);
    virtual ~DynamicServiceManager();   

    /**
     * Add a directory to "watch" for plugins
     */
    void setPluginDirectory(const bp::file::Path & path);

    /**
     * Clear all plugin directories
     */
    void clearPluginDirectories(void);

    /**
     * Attain a list of the descriptions of available dynamic services.
     */
    std::vector<bp::service::Description> availableServices();

    /**
     * Attain a description of a specified service.  Returned value 
     * has empty name on failure.
     */
    bool describe(const std::string & name,
                  const std::string & version,
                  const std::string & minversion,
                  bp::service::Description & oDescription);
    
    /**
     * Attain a summary of a specified service.  Returned value 
     * has empty name on failure.
     */
    bool summary(const std::string & name,
                 const std::string & version,
                 const std::string & minversion,
                 bp::service::Summary & oSummary);

    /**
     * do we have a service that can satisfy the requires statement
     */
    bool haveService(const std::string & name, const std::string & version,
                     const std::string & minversion);

    /*
     * Attain a list of all summaries
     */
    std::vector<bp::service::Summary> availableServiceSummaries();

   /**
     * By default the ServiceManager library is conservative with
     * touching the disk to rescan services.  This means that any
     * time a service is installed or deleted, one must explicitly
     * force a rescan to cause a running BrowserPlusCore to notice
     * updated services on disk.
     */
    void forceRescan();

    /** 
     * Determine if the DynamicServiceManager is busy (and may not be shut
     * down), this may be true if services have requested a delayed
     * shutdown, or other quick non-interruptable operations are in
     * progress.
     */
    bool isBusy();

    /**
     * Purge a service from disk.
     */
    bool purgeService(const std::string & name, const std::string & version);
    
    /** 
     * asynchronously instantiate an instance of a service.  This will
     * cause the service to be spawned if required.  An integer "allocation
     * id" is return which will be delivered to the listener's
     * gotInstance callback.
     *
     * \returns
     * zero will be returned if the allocation fails, or no such
     * service is installed.  This can result from an external program
     * modifying or removing services on disk.
     */
    unsigned int instantiate(
        const std::string & name,
        const std::string & version,
        std::tr1::weak_ptr<CoreletExecutionContext> context,
        std::tr1::weak_ptr<ICoreletRegistryListener> listener);


    unsigned int getUniqueInstantiateId()
    {
        return m_instantiateId++;
    }
    
  private:
    // the configured plugin directory
      bp::file::Path m_pluginDirectory;

    // detected installed services
    std::map<bp::service::Summary, bp::service::Description> m_services;

    std::string m_logLevel;
    bp::file::Path m_logFile;

    // monotonically increasing ids returned by instance calls to allow
    // client to correlate
    unsigned int m_instantiateId;

    // search the m_services map and find a service satisfying the
    // require specification
    bool internalFind(const std::string & name,
                      const std::string & version,
                      const std::string & minversion,
                      bp::service::Summary & oSummary,
                      bp::service::Description & oDescription);

    // data structures and utility functions to manage all of our
    // complex state
    DynamicServiceState m_state;

    // start an allocation, adding to running allocations
    void startAllocation(std::tr1::shared_ptr<ServiceRunner::Controller> c,    
                         std::tr1::shared_ptr<DynamicServiceInstance> instance,   
                         unsigned int majorVer);

    // implemented methods from ServiceRunner::IControllerListener
    void initialized(ServiceRunner::Controller * c, const std::string & service,
                     const std::string & version, unsigned int apiVersion);
    void onEnded(ServiceRunner::Controller * c);
    void onDescribe(ServiceRunner::Controller * c,
                    const bp::service::Description & desc);
    void onAllocated(ServiceRunner::Controller * c,
                     unsigned int allocationId,
                     unsigned int instance);
    void onInvokeResults(ServiceRunner::Controller * c,
                         unsigned int instance, unsigned int tid,
                         const bp::Object * results);
    void onInvokeError(ServiceRunner::Controller * c,
                       unsigned int instance, unsigned int tid,
                       const std::string & error,
                       const std::string & verboseError);
    void onCallback(ServiceRunner::Controller * c,
                    unsigned int instance, unsigned int tid,
                    long long int callback, const bp::Object * value);
    void onPrompt(ServiceRunner::Controller * c,
                  unsigned int instance, unsigned int promptId,
                  const bp::file::Path & pathToDialog,
                  const bp::Object * arguments);

    /* how instances call back into us */
    void onInstanceShutdown(DynamicServiceInstance * instance);
    void onInstanceExecute(DynamicServiceInstance * instance,
                           unsigned int clientTid,
                           const std::string & function,
                           const bp::Object & args);
    void onPromptResponse(DynamicServiceInstance * instance,
                          unsigned int promptId,
                          const bp::Object& response);

    friend class DynamicServiceInstance;
};
    
#endif
