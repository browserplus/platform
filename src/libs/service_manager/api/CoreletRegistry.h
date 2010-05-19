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
 * CoreletRegistry
 *
 * A collection of available corelets from which corelets may be
 * described, and instances of corelets may be attained.
 */

#ifndef __CORELETREGISTRY_H__
#define __CORELETREGISTRY_H__

#include "BPUtils/ServiceDescription.h"
#include "BPUtils/bpfile.h"
#include "CoreletManager/CoreletExecutionContext.h"
#include "CoreletManager/CoreletInstance.h"
#include "CoreletManager/CoreletFactory.h"


class ICoreletRegistryListener  
{
public:
    virtual ~ICoreletRegistryListener() { }
    
    virtual void
    onAllocationSuccess(unsigned int allocationId,
                        std::tr1::shared_ptr<CoreletInstance> instance) = 0;
    
    virtual void
    onAllocationFailure(unsigned int allocationId) = 0;
};


class CoreletRegistry : public bp::thread::HoppingClass
{
public:
    /**
     * Instantiate a corelet registry, specifying logfile and loglevel
     * which will be relayed to spawned services.
     */
    CoreletRegistry(const std::string & loglevel,
                    const bp::file::Path & logfile);

    /**
     * Destory a corelet registry
     */
    ~CoreletRegistry();    
    /**
     * Set the directory where dynamic services are installed
     */
    void setPluginDirectory(const bp::file::Path & path);

   /**
     * By default the CoreletManager library is conservative with
     * touching the disk to rescan services.  This means that any
     * time a corelet is installed or deleted, one must explicitly
     * force a rescan to cause a running BrowserPlusCore to notice
     * updated services on disk.
     */
    void forceRescan();

    /**
     * register a corelet with the registry
     *
     * \param description - an object which describes the corelet
     * \param factory - a factory which may produce instances of
     *                  the corelet 
     *
     * \returns false if a corelet with the same name and version
     *                is already registered.
     */
    bool registerCorelet(const bp::service::Description & description,
                         std::tr1::shared_ptr<CoreletFactory> factory);

    /** 
     * purge a specified corelet
     */
    bool purgeCorelet(const std::string & name,
                      const std::string & version);
    
    /**
     * unregister all registered corelets
     */
    void unregisterAll();

    /**
     * Attain a list of the descriptions of available corelets.
     */
    std::list<bp::service::Description> availableCorelets();
    
    /**
     * Attain a list of the summaries of available corelets
     */
    std::list<bp::service::Summary> availableCoreletSummaries();

    /**
     * Attain a description of a specified corelet.  On failure,
     * description's name will be empty
     */
    bool describe(const std::string & name,
                  const std::string & version,
                  const std::string & minversion,
                  bp::service::Description & oDescription);
    
    /**
     * Attain a summary of a specified corelet.  empty name returned
     * for builtin corelets
     */
    bool summary(const std::string & name,
                 const std::string & version,
                 const std::string & minversion,
                 bp::service::Summary & oSummary);

    /**
     * do we have a corelet that can satisfy the requires statement
     */
    bool haveCorelet(const std::string & name,
                     const std::string & version,
                     const std::string & minversion);
    
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

    /** 
     * Determine if the CoreletRegistry is busy, this may be true if
     * services have requested a delayed shutdown, or other quick
     * non-interruptable operations are in progress.
     */
    bool isBusy();

  private:
    typedef std::pair< bp::service::Description,
                       std::tr1::shared_ptr<CoreletFactory> >
        DescFactPair;
    std::list<DescFactPair> m_registrations;
    DescFactPair getReg(const std::string & name, const std::string & version,
                        const std::string & minversion);
    std::tr1::shared_ptr<class DynamicServiceManager> m_dynamicManager;

    // implemented from HoppingClass.  used to return allocated built-in
    // instances AFTER function return.
    void onHop(void * x);
};
    
#endif
