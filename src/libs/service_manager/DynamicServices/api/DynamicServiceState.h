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
 * DynamicServiceState
 *
 * The management of dynamic services requires complex data structures
 * and requires that a variety of queries be answerable in an efficient
 * fashion.  To ease implementation complexity, the DynamicServiceState
 * class encapsulates both the data structures and the queries that
 * are to be performed, breaking this complexity out of the
 * DynamicServiceManager.
 */

#ifndef __DYNAMICSERVICESTATE_H__
#define __DYNAMICSERVICESTATE_H__

#include "ServiceRunnerLib/ServiceRunnerLib.h"
#include "CoreletExecutionContext.h"
#include "CoreletRegistry.h"
#include "DynamicServiceInstance.h"


class DynamicServiceState : public bp::time::ITimerListener
{
  public:
    DynamicServiceState();
    ~DynamicServiceState();    

    // do we have a controller running and initialized for the
    // specified service?
    std::tr1::shared_ptr<ServiceRunner::Controller>
        getRunningController(const bp::service::Summary & s);

    // do we have a controller in the process of starting up for
    // the specified service?
    std::tr1::shared_ptr<ServiceRunner::Controller>
        getPendingController(const bp::service::Summary & s);

    // add an allocation that's pending based on the startup of a
    // specified service
    void addPendingAllocation(
        std::tr1::shared_ptr<ServiceRunner::Controller> controller,
        std::tr1::shared_ptr<DynamicServiceInstance> instance);

    // add an allocation to the running map, and register the associated
    // controller (if not registered).  this will retain both the
    // instance and the controller (if required).
    void addRunningAllocation(
        std::tr1::shared_ptr<ServiceRunner::Controller> controller,
        unsigned int allocationId,
        std::tr1::shared_ptr<DynamicServiceInstance> instance);

    // pull all entries off the pendingAllocation list for a given
    // service.  turn them all into instances.
    void popPendingAllocations(
        const bp::service::Summary & summary,
        std::set<std::tr1::shared_ptr<DynamicServiceInstance> > & oPending,
        std::tr1::shared_ptr<ServiceRunner::Controller> & oController);

    // create a DynamicServiceInstance and initialize data members
    std::tr1::shared_ptr<DynamicServiceInstance>
        createInstance(class DynamicServiceManager * manager,
                       std::tr1::weak_ptr<CoreletExecutionContext> contextWeak,
                       std::tr1::weak_ptr<ICoreletRegistryListener> listener,
                       unsigned int instantiateId,
                       const bp::service::Summary & summary);

    // pull an instance off the "running" list, returning a shared_ptr
    // to it.  this essentially promotes a instance in process of
    // allocation to a full blown allocation
    std::tr1::shared_ptr<DynamicServiceInstance>
        allocationComplete(ServiceRunner::Controller * c,
                           unsigned int allocationId,
                           unsigned int instanceId);

    // find an instance shared ptr given a controller raw pointer and
    // an allocation id.  This is what we have when called back
    // by the controller.  This function may fail if the client
    // has deleted the instance.
    std::tr1::shared_ptr<DynamicServiceInstance>    
        findInstance(ServiceRunner::Controller * c,
                     unsigned int allocationId);

    // to be called when an instance is destroyed.  This will remove
    // all references to the instance, and decrement the controller's
    // reference count, destroying it if neccesary.
    void removeInstance(DynamicServiceInstance * instance);

    // a set of summaries representing currently running services
    std::set<bp::service::Summary> runningServices();

    // remove all references to a service and end its process (if
    // running)  This will safely invalidate all outstanding instances.
    // This call causes an abrupt stopping of services, it will not
    // respect 'shutdownDelaySecs' parameters
    void stopService(const bp::service::Summary & summary);
    
  private:
    // a timer and callback function which will be invoked when the timer
    // expires.  This timer is used for services with delayed shutdown
    // parameters specified in their manifest
    void timesUp(bp::time::Timer * t);
    bp::time::Timer m_delayedShutdownTimer;
    // set the idle check to the soonest required time, or not at all
    void rescheduleIdleCheck();

    // a map containing half birthed instances, which are waiting for
    // a controller to be initialized.  We own these instances.
    typedef std::pair<std::tr1::shared_ptr<ServiceRunner::Controller>,
                      std::set<std::tr1::shared_ptr<DynamicServiceInstance> > >
        PendControllerServiceSetPair;

    // instances which are pending controller initialization
    typedef std::map<bp::service::Summary, PendControllerServiceSetPair>
        PendControllerMap;

    PendControllerMap m_pendingAllocations;

    typedef struct ControllerContext {
        // smart pointer to the controller
        std::tr1::shared_ptr<ServiceRunner::Controller> controller;
        // a set of all of the active instances
        std::set<DynamicServiceInstance *> instances;
        // the time since the last instance was destroyed
        bp::time::Stopwatch idleTime;
    };

    typedef std::map<bp::service::Summary, ControllerContext> ControllerMap;

    // a map containing running controllers and references to all of their
    // allocated or in-process-of-allocation instances.  We do not own
    // these instances.
    ControllerMap m_controllers;

    // a map containing instantiate() calls that are running,
    // mapping allocation ids from service runner to allocated
    // DynamicServiceInstances.  We'll retain ownership of these
    // instances until we're able to call onAllocated
    std::map<unsigned int, std::tr1::shared_ptr<DynamicServiceInstance> >
        m_runningAllocations;

    // a map allowing us to get from instance id to service instance.
    // The client managers lifetime of instances and we have a hook upon
    // their destruction.  
    typedef std::map<unsigned int, std::tr1::weak_ptr<DynamicServiceInstance> > 
        IdToInstanceMap;

    typedef std::map<ServiceRunner::Controller *, IdToInstanceMap>
        InstanceMap;
    
    InstanceMap m_instances;
};

#endif
