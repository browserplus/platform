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
 * DynamicServiceState
 *
 * The management of dynamic services requires complex data structures
 * and requires that a variety of queries be answerable in an efficient
 * fashion.  To ease implementation complexity, the DynamicServiceState
 * class encapsulates both the data structures and the queries that
 * are to be performed, breaking this complexity out of the
 * DynamicServiceManager.
 */

#include "DynamicServiceState.h"
#include "BPUtils/BPLog.h"
#include "DynamicServiceManager.h"

using namespace std;
using namespace std::tr1;


DynamicServiceState::DynamicServiceState()
{
    m_delayedShutdownTimer.setListener(this);
}

DynamicServiceState::~DynamicServiceState()
{
    m_delayedShutdownTimer.setListener(NULL);
    m_delayedShutdownTimer.cancel();
}

shared_ptr<ServiceRunner::Controller>
DynamicServiceState::getRunningController(const bp::service::Summary & s)
{
    ControllerMap::iterator i;
    if ((i = m_controllers.find(s)) != m_controllers.end())
    {
        return i->second.controller;
    }
    return shared_ptr<ServiceRunner::Controller>();
}

shared_ptr<ServiceRunner::Controller>
DynamicServiceState::getPendingController(const bp::service::Summary & s)
{
    shared_ptr<ServiceRunner::Controller> controller;
    PendControllerMap::iterator i;
    if ((i = m_pendingAllocations.find(s)) != m_pendingAllocations.end())
    {
        controller = i->second.first;
    }
    return controller;
}

void
DynamicServiceState::addPendingAllocation(
    shared_ptr<ServiceRunner::Controller> controller,
    shared_ptr<DynamicServiceInstance> instance)
{
    PendControllerMap::iterator i =
        m_pendingAllocations.find(instance->m_summary);

    if (i == m_pendingAllocations.end()) {
        m_pendingAllocations[instance->m_summary] =
            PendControllerServiceSetPair(
                controller,
                std::set<shared_ptr<DynamicServiceInstance> >());
        i = m_pendingAllocations.find(instance->m_summary);        
        BPASSERT(i != m_pendingAllocations.end());
    }

    i->second.second.insert(instance);
}

shared_ptr<DynamicServiceInstance>
DynamicServiceState::createInstance(
    DynamicServiceManager * manager,
    weak_ptr<CoreletExecutionContext> contextWeak,
    weak_ptr<ICoreletRegistryListener> listener,
    unsigned int instantiateId,
    const bp::service::Summary & summary)
{
    shared_ptr<DynamicServiceInstance>
        instance(new DynamicServiceInstance(contextWeak));
    instance->m_instanceId = 0;
    instance->m_instantiateId = instantiateId;
    instance->m_registryListener = listener;
    instance->m_manager = manager;
    instance->m_summary = summary;
    return instance;
}

void
DynamicServiceState::addRunningAllocation(
    shared_ptr<ServiceRunner::Controller> controller,
    unsigned int allocationId,
    shared_ptr<DynamicServiceInstance> instance)
{
    // add controller to the m_controllers map, and raw pointer of
    // instance to same for reference counting
    ControllerMap::iterator i = m_controllers.find(instance->m_summary);
    if (i == m_controllers.end()) {
        ControllerContext ctx;
        ctx.controller = controller;
        m_controllers[instance->m_summary] = ctx;
        i = m_controllers.find(instance->m_summary);        
        BPASSERT(i != m_controllers.end());
    } else {
        // reset the idle timer
        if (i->second.idleTime.elapsedSec() > 0.0) {
            BPLOG_INFO_STRM("Resetting idle timer for " << i->first.name()
                            << " " << i->first.version() << ", new instance "
                            << "allocated.");
            i->second.idleTime.reset();
        }
    }
    
    BPASSERT(i->second.instances.find(instance.get()) == 
           i->second.instances.end());
    i->second.instances.insert(instance.get());

    // now add to the running allocations table to retain the instance
    // while it's being connected
    BPASSERT(m_runningAllocations.find(allocationId) ==
           m_runningAllocations.end());
    BPASSERT(instance != NULL);

    m_runningAllocations[allocationId] = instance;
}

shared_ptr<DynamicServiceInstance>
DynamicServiceState::allocationComplete(ServiceRunner::Controller * c,
                                        unsigned int allocationId,
                                        unsigned int instanceId)
{
    shared_ptr<DynamicServiceInstance> instance;
    std::map<unsigned int, shared_ptr<DynamicServiceInstance> >::iterator
        i;
    i = m_runningAllocations.find(allocationId);
    if (i != m_runningAllocations.end()) {
        instance = i->second;
        instance->m_instanceId = instanceId;
        m_runningAllocations.erase(i);

        // now let's add this to the allocated instances map
        InstanceMap::iterator im = m_instances.find(c);
        if (im == m_instances.end()) {
            m_instances[c] = IdToInstanceMap();
            im = m_instances.find(c);
            BPASSERT(im != m_instances.end());
        }
        im->second[instanceId] = instance;
        
        // XXX: we could verify this is still on the m_controllers map
        //      for reference counting purposes
    }

    return instance;
}

void
DynamicServiceState::popPendingAllocations(
    const bp::service::Summary & summary,
    std::set<shared_ptr<DynamicServiceInstance> > & oPending,
    shared_ptr<ServiceRunner::Controller> & oController)
{
    PendControllerMap::iterator i = m_pendingAllocations.find(summary);
    if (i != m_pendingAllocations.end()) {    
        oController = i->second.first;
        oPending = i->second.second;
        m_pendingAllocations.erase(i);
    }
}

void
DynamicServiceState::popPendingAllocations(
    const ServiceRunner::Controller * c,
    set<shared_ptr<DynamicServiceInstance> > & oPending )
{
    for (PendControllerMap::iterator i = m_pendingAllocations.begin();
         i != m_pendingAllocations.end(); ++i) {
        if (i->second.first.get() == c) {
            oPending = i->second.second;
            m_pendingAllocations.erase(i);
            return;
        }
    }
}

void
DynamicServiceState::rescheduleIdleCheck()
{
    m_delayedShutdownTimer.cancel();

    // iterate through all controllers, any controllers with zero instances
    // will cause us to reset the idle check timer
    ControllerMap::iterator i;
    double nextCheck = 0.0;
    for (i = m_controllers.begin(); i != m_controllers.end(); i++)
    {
        if (i->second.instances.size() == 0) {
            int delay = i->first.shutdownDelaySecs();
            if (delay > 0) {
                if (nextCheck == 0.0 || nextCheck > (double) delay)
                {
                    // add 1/10th of a second to ensure we don't wake
                    // up early
                    nextCheck = (double) delay + 0.1;
                }
            }
        }
    }

    if (nextCheck > 0.0) {
        BPLOG_INFO_STRM("checking for idle services to shutdown in "
                        << nextCheck << "s");
        m_delayedShutdownTimer.setMsec((unsigned int) (nextCheck * 1000));
    }
}

void
DynamicServiceState::timesUp(bp::time::Timer *)
{
    // cancel the timer for good measure
    m_delayedShutdownTimer.cancel();

    // iterate through all controllers, any controllers with zero instances
    // who have expired time will get purged
    ControllerMap::iterator i = m_controllers.begin();
    while (i != m_controllers.end())
    {
        int delay = i->first.shutdownDelaySecs();
        if (i->second.instances.size() == 0 &&
            i->second.idleTime.elapsedSec() > (double) delay)
        {
            BPLOG_INFO_STRM("Shutting down " << i->first.name() << " " 
                            << i->first.version() << " after being idle for "
                            << i->second.idleTime.elapsedSec() << " seconds");
            m_controllers.erase(i);
            i = m_controllers.begin();
        } else {
            i++;
        }
    }

    rescheduleIdleCheck();
}

void
DynamicServiceState::removeInstance(DynamicServiceInstance * instance)
{
    shared_ptr<ServiceRunner::Controller> controller;

    // let's remove all record of this instance!
    
    // first, it's impossible that this instance was associated with an
    // allocation pending service initialization.  We're holding shared
    // pointers to all such instances
    BPASSERT(m_pendingAllocations.find(instance->m_summary) == 
           m_pendingAllocations.end());

    // second, this instance may not be in the process of being allocated,
    // cause we're STILL holding a smart pointer to it.

    ControllerMap::iterator i = m_controllers.find(instance->m_summary);

    if (i != m_controllers.end()) {
        controller = i->second.controller;

        // now erase the raw ptr
        std::set<DynamicServiceInstance *>::iterator rawIt;
        rawIt = i->second.instances.find(instance);
        if (rawIt != i->second.instances.end()) {
            i->second.instances.erase(rawIt);
        }

        // now if there are no more references to this controller, we'll
        // either delete it, or schedule it for deletion
        if (i->second.instances.size() == 0) {
            int delay = i->first.shutdownDelaySecs();
            if (delay > 0) {
                BPLOG_INFO_STRM("Delaying shutdown of service "
                                << i->first.name() << " "
                                << i->first.version() << " up to "
                                << delay << " seconds");
                i->second.idleTime.reset();
                i->second.idleTime.start();                
                rescheduleIdleCheck();
            } else {
                BPLOG_INFO_STRM("No more instances, shutting down service "
                                << i->first.name() << " "
                                << i->first.version());
                m_controllers.erase(i);
            }
        }
    }

    // now if we found the controller we can remove this guy from the
    // instance map
    if (controller != NULL) 
    {
        InstanceMap::iterator im = m_instances.find(controller.get());
        if (im != m_instances.end()) {
            IdToInstanceMap::iterator idtim =
                im->second.find(instance->m_instanceId);
            if (idtim != im->second.end()) {
                im->second.erase(idtim);
            }
            if (im->second.size() == 0) {
                m_instances.erase(im);
            }
        }
    }
}

shared_ptr<DynamicServiceInstance>    
DynamicServiceState::findInstance(ServiceRunner::Controller * c,
                                  unsigned int allocationId)
{
    shared_ptr<DynamicServiceInstance> instance;
    
    // now let's add this to the allocated instances map
    InstanceMap::iterator im = m_instances.find(c);
    if (im != m_instances.end()) {
        IdToInstanceMap::iterator idtim = im->second.find(allocationId);
        if (idtim != im->second.end()) {
            instance = idtim->second.lock();
        }
    }

    return instance;
}

std::set<bp::service::Summary>
DynamicServiceState::runningServices()
{
    std::set<bp::service::Summary> s;

    // fully allocated controllers
    ControllerMap::iterator ci;
    for (ci = m_controllers.begin(); ci != m_controllers.end(); ci++)
    {
        s.insert(ci->first);
    }

    // controllers in process of allocation
    PendControllerMap::iterator pi;
    for (pi = m_pendingAllocations.begin();
         pi != m_pendingAllocations.end(); pi++)
    {
        s.insert(pi->first);
    }

    return s;
}

void
DynamicServiceState::stopService(const bp::service::Summary & summary)
{
    // remove allocated controller if present
    ControllerMap::iterator ci = m_controllers.find(summary);
    if (ci != m_controllers.end())
    {
        m_controllers.erase(ci);
    }

    // controllers in process of allocation
    PendControllerMap::iterator pi = m_pendingAllocations.find(summary);
    if (pi != m_pendingAllocations.end())
    {
        m_pendingAllocations.erase(pi);
        // TODO: invoke allocation failed callback
    }

    // now any running allocations should be obliterated
    std::map<unsigned int, shared_ptr<DynamicServiceInstance> >::iterator
        rait = m_runningAllocations.begin();

    while (rait != m_runningAllocations.end()) 
    {
        if (!rait->second->m_summary.name().compare(summary.name()) &&
            !rait->second->m_summary.version().compare(summary.version()))
        {
            // TODO: invoke allocation failed callback
            m_runningAllocations.erase(rait);
            rait = m_runningAllocations.begin();
        }
        else 
        {
            rait++;
        }
    }
}
