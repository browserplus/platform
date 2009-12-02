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

/**
 * DynamicServiceManager
 *
 * An object responsible for loading and searching of dynamic
 * corelets.
 */

#include "DynamicServiceManager.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/ServiceInterfaceCache.h"
#include "DiskScanner.h"

using namespace std;
using namespace std::tr1;


/**
 * a dynamic service manager
 */
DynamicServiceManager::DynamicServiceManager(const std::string & loglevel,
                                             const bp::file::Path & logfile)
    : m_logLevel(loglevel), m_logFile(logfile), m_instantiateId(10000)
{
}

DynamicServiceManager::~DynamicServiceManager()
{
}

void
DynamicServiceManager::setPluginDirectory(const bp::file::Path & path)
{
    // no support for multiple plugin directories?
    m_pluginDirectory = path;
    m_services = DiskScanner::scanDiskForServices(
        path, m_services, m_state.runningServices(), m_logLevel, m_logFile);
}

void
DynamicServiceManager::clearPluginDirectories(void)
{
}

std::vector<bp::service::Description>
DynamicServiceManager::availableServices()
{
    std::vector<bp::service::Description> v;

    std::map<bp::service::Summary, bp::service::Description>::iterator i;
    for (i = m_services.begin(); i != m_services.end(); i++)
    {
        v.push_back(i->second);
    }

    return v;
}

bool
DynamicServiceManager::describe(const std::string & name,
                                const std::string & version,
                                const std::string & minversion,
                                bp::service::Description & d)
{
    bp::service::Summary s;    
    return internalFind(name, version, minversion, s, d);

}

bool
DynamicServiceManager::summary(const std::string & name,
                               const std::string & version,
                               const std::string & minversion,
                               bp::service::Summary & s)
{
    bp::service::Description d;    
    return internalFind(name, version, minversion, s, d);
}

bool
DynamicServiceManager::haveService(const std::string & name,
                                   const std::string & version,
                                   const std::string & minversion)
{
    bp::service::Summary s;    
    bp::service::Description d;    
    return internalFind(name, version, minversion, s, d);
}

std::vector<bp::service::Summary>
DynamicServiceManager::availableServiceSummaries()
{
    std::vector<bp::service::Summary> v;

    std::map<bp::service::Summary, bp::service::Description>::iterator i;
    for (i = m_services.begin(); i != m_services.end(); i++)
    {
        v.push_back(i->first);
    }

    return v;
}

bool
DynamicServiceManager::isBusy()
{
    return m_state.runningServices().size() > 0;
}

void
DynamicServiceManager::forceRescan()
{
    std::set<bp::service::Summary> running = m_state.runningServices();
    
    // rescan services
    m_services = DiskScanner::scanDiskForServices(
        m_pluginDirectory, m_services, running,
        m_logLevel, m_logFile);
    
    // now any running services that no longer exist on disk, we'll
    // kill immediatedly
    std::set<bp::service::Summary>::iterator i;
    for (i = running.begin(); i != running.end(); i++) 
    {
        if (m_services.find(*i) == m_services.end()) {
            BPLOG_WARN_STRM(i->name() << " - " << i->version() << " was "
                            "running but has been removed from disk, stopping "
                            "all active instances");
            m_state.stopService(*i);
        }
    }
}

bool
DynamicServiceManager::purgeService(const std::string & name,
                                    const std::string & version)
{
    bp::service::Summary summary;
    bp::service::Description description;
    // first let's confirm that we have a satisfying service installed
    if (!internalFind(name, version, std::string(), summary, description)) 
    {
        return false;
    }

    BPLOG_INFO_STRM("Purging service from disk: "
                    << summary.name() << " - " << summary.version());

    /* purge the cached interface */
    bp::serviceInterfaceCache::purge(summary.name(), summary.version());

    m_state.stopService(summary);
    (void) bp::file::remove(summary.path());    
    forceRescan();

	return true;
}


bool
DynamicServiceManager::internalFind(const std::string & name,
                                    const std::string & versionString,
                                    const std::string & minversionString,
                                    bp::service::Summary & oSummary,
                                    bp::service::Description & oDescription)
{
    bp::ServiceVersion version, minversion;
    if (!version.parse(versionString)) return false;
    if (!minversion.parse(minversionString)) return false;

    bp::ServiceVersion bestVer;
    bool found = false;

    std::map<bp::service::Summary, bp::service::Description>::iterator i;
    for (i = m_services.begin(); i != m_services.end(); i++)
    {
        if (0 != name.compare(i->first.name())) continue;
        
        if (bp::ServiceVersion::isNewerMatch(
                i->second.version(), bestVer, version, minversion))
        {
            oSummary = i->first;
            oDescription = i->second;
            bestVer = oDescription.version();
            found = true;
        }
    }

    return found;
}

// given a dependant summary and a set of provider summaries, attain
// the path to the best match.  returns empty string if there is
// no viable match
static bp::file::Path
getBestProvider(const bp::service::Summary & dep,
                const std::map<bp::service::Summary, bp::service::Description>
                     & installed)
{
    std::string name = dep.usesCorelet();
    bp::ServiceVersion version = dep.usesVersion();
    bp::ServiceVersion minversion = dep.usesMinversion();

    bp::service::Summary winner;
    bp::ServiceVersion winnerVer;        

    std::map<bp::service::Summary, bp::service::Description>::const_iterator i;

    for (i = installed.begin(); i != installed.end(); i++)
    {
        if (bp::ServiceVersion::isNewerMatch(i->second.version(), winnerVer,
                                             version, minversion))
        {
            winner = i->first;
            winnerVer = i->second.version();
        }
    }

    if (!winner.name().empty()) return winner.path();
    
    return bp::file::Path();
}

unsigned int
DynamicServiceManager::instantiate(
    const std::string & name,
    const std::string & version,
    weak_ptr<CoreletExecutionContext> contextWeak,
    weak_ptr<ICoreletRegistryListener> listener)
{
    unsigned int instantiateId = m_instantiateId++;

    bp::service::Summary summary;
    bp::service::Description description;

    // first let's confirm that we have a satisfying service installed
    if (!internalFind(name, version, std::string(), summary, description)) 
    {
        return 0;
    }

    // convert context ptr to strong
    shared_ptr<CoreletExecutionContext> context = contextWeak.lock();
    if (context == NULL) return 0;

    // if we don't have a Controller allocated for this service we'll
    // need to start one.  otherwise we'll allocate a new instance on
    // the existing Controller
    shared_ptr<ServiceRunner::Controller> controller;
    controller = m_state.getRunningController(summary);

    if (controller == NULL)
    {
        // let's see if there's a pending Controller instantiation
        // that we can use (no double loading of services)!
        controller = m_state.getPendingController(summary);

        if (controller == NULL) {
            // we must start up the controller
            controller.reset(new ServiceRunner::Controller(summary.path()));
            controller->setListener(this);

            // get a provider for dependent services
            bp::file::Path providerPath;
            if (summary.type() == bp::service::Summary::Dependent)
            {
                providerPath = getBestProvider(summary, m_services);
                if (providerPath.empty()) return 0;
            }

            // get a reasonable title for the spawned process
            std::string processTitle, ignore;            
            if (!summary.localization(context->locale(), processTitle, ignore))
            {
                processTitle.append("BrowserPlus: Spawned Service");
            }
            else
            {
                processTitle = (std::string("BrowserPlus: ") + processTitle);
            }
        
            std::string err;
            if (!controller->run(bp::paths::getRunnerPath(),
                                 providerPath, processTitle, 
                                 m_logLevel, m_logFile, err))
            {
                BPLOG_WARN_STRM("Couldn't load " << summary.name() << " - "
                                << summary.version() << ": " << err);
                return 0;
            }
        }

        // now we must record this allocation request. which will be serviced
        // after the controller starts up.
        shared_ptr<DynamicServiceInstance> instance;
        instance = m_state.createInstance(this, contextWeak, listener,
                                          instantiateId, summary);
        m_state.addPendingAllocation(controller, instance);
    }
    else
    {
        shared_ptr<DynamicServiceInstance> instance;
        instance = m_state.createInstance(this, contextWeak,
                                          listener, instantiateId,
                                          summary);

        // start the allocation!
        startAllocation(controller, instance,
                        (unsigned int) description.version().majorVer());
    }
    
    return instantiateId;
}

void
DynamicServiceManager::initialized(ServiceRunner::Controller * c,
                                   const std::string & service,
                                   const std::string & version,
                                   unsigned int)
{
    // now we must begin allocations for all items on this service's
    // PendingInitiation list 
    bp::service::Summary summary;
    bp::service::Description desc;
    if (!internalFind(service, version, std::string(), summary, desc)) {
        BPLOG_ERROR_STRM("INTERNAL ERROR: Initalized service with unknown "
                         "service/version: " << service << " - " << version);
        // TODO: perhaps in this case we should throw fatal?
        return;
    }

    std::set<shared_ptr<DynamicServiceInstance> > s;
    shared_ptr<ServiceRunner::Controller> controller;
    m_state.popPendingAllocations(summary, s, controller);

    // start all of these allocations
    if (s.size() > 0) {
        std::set<shared_ptr<DynamicServiceInstance> >::iterator i;
        for (i = s.begin(); i != s.end(); i++) {
            startAllocation(controller, *i, desc.version().majorVer());
        }
    } else {
        // yikes, this is possibly internal corruption!
        BPLOG_ERROR_STRM("Initalized service with no "
                         "pending allocations: " << service << " - "
                         << version);
    }
}

void
DynamicServiceManager::startAllocation(
    shared_ptr<ServiceRunner::Controller> controller,    
    shared_ptr<DynamicServiceInstance> instance,   
    unsigned int majorVer)
{
    shared_ptr<CoreletExecutionContext> context =
        instance->m_context.lock();

    if (context == NULL) {
        BPLOG_ERROR_STRM("allocate called with invalid context pointer");
        return;
    }

    // generate a temporary directory
    bp::file::Path tmpdir =
        bp::file::getTempPath(bp::file::getTempDirectory(), "ServiceData");

    unsigned int aid = controller->allocate(
        context->URI(),
        bp::paths::getCoreletDataDirectory(instance->m_summary.name(),
                                           majorVer),
        tmpdir,
        context->locale(),
        context->userAgent(),
        context->clientPid());

    if (aid != (unsigned int) -1) {
        m_state.addRunningAllocation(controller, aid, instance);
    } else {
        // TODO: we must message our listener, but we don't want to
        // do it before the allocate function returns to them.
        BPLOG_ERROR_STRM("unimplemented error handling for failed allocations");
    }
}

void
DynamicServiceManager::onEnded(ServiceRunner::Controller * c)
{
    BPLOG_ERROR_STRM(c->serviceName() << " (" << c->serviceVersion() << ") " <<
                     "ended unexpectedly.");

    // TODO: is there more handling that should be done?
}

void
DynamicServiceManager::onDescribe(ServiceRunner::Controller * c,
                                  const bp::service::Description & desc)
{
    // Unused!  see DiskScanner
    abort();
}

void
DynamicServiceManager::onAllocated(ServiceRunner::Controller * c,
                                   unsigned int aid,
                                   unsigned int id)
{
    // let's pull the allocated service off or our list, correctly
    // set its instance id, and notify the listener

    // this call will transfer ownership of the instance to us.
    shared_ptr<DynamicServiceInstance> instance = 
        m_state.allocationComplete(c, aid, id);

    if (instance == NULL) {
        BPLOG_ERROR_STRM("onAllocate called with unknown allocation id");
        return;
    }

    instance->m_instanceId = id;

    // let's call back into our listener
    shared_ptr<ICoreletRegistryListener> regListener;
    regListener = instance->m_registryListener.lock();

    if (regListener == NULL) {
        BPLOG_ERROR("service allocation completed, but listener has "
                    "dissapeared.  deleting newly allocated instance.");
        // returning will destruct the instance which will call back
        // into us.
        return;
    }

    // this call will transfer ownership of the instance to the listener
    regListener->gotInstance(instance->m_instantiateId, instance);
}

void
DynamicServiceManager::onInvokeResults(ServiceRunner::Controller * c,
                                       unsigned int instance, 
                                       unsigned int tid,
                                       const bp::Object * results)
{
    shared_ptr<DynamicServiceInstance> sptr =
        m_state.findInstance(c, instance);

    if (sptr != NULL) {
        unsigned int origTid = tid;

        // we must map the transaction id
        if (sptr->mapToClientTid(tid)) {
            bp::Null n;
            if (results == NULL) results = &n;
            sptr->sendComplete(tid, *results);
            sptr->removeTid(origTid);
        } else {
            BPLOG_WARN_STRM("received invocation results for a transactions "
                            "that longer exists: " << tid);
        }
    } else {
        BPLOG_WARN_STRM("received invocation results for an instance that "
                        "no longer exists: " << instance);
    }
}

void
DynamicServiceManager::onInvokeError(ServiceRunner::Controller * c,
                                     unsigned int instance, 
                                     unsigned int tid,
                                     const std::string & error,
                                     const std::string & verboseError)
{
    shared_ptr<DynamicServiceInstance> sptr =
        m_state.findInstance(c, instance);

    if (sptr != NULL) {
        unsigned int origTid = tid;
        
        // we must map the transaction id
        if (sptr->mapToClientTid(tid)) {
            sptr->sendFailure(tid, error, verboseError);
            sptr->removeTid(origTid);
        } else {
            BPLOG_WARN_STRM("received invocation results for a transactions "
                            "that longer exists: " << tid);
        }
    } else {
        BPLOG_WARN_STRM("received invocation results for an instance that "
                        "no longer exists: " << instance);
    }
}

void
DynamicServiceManager::onCallback(ServiceRunner::Controller * c,
                                  unsigned int instance, 
                                  unsigned int tid,
                                  long long int callback,
                                  const bp::Object * value)
{
    shared_ptr<DynamicServiceInstance> sptr =
        m_state.findInstance(c, instance);

    if (sptr != NULL) {
        // we must map the transaction id
        if (sptr->mapToClientTid(tid)) {
            // map in results
            bp::Map m;
            m.add("callback", new bp::Integer(callback));
            if (value != NULL) m.add("parameters", value->clone());
            sptr->invokeCallback(tid, m);
        } else {
            BPLOG_WARN_STRM("received callback invocation for a transactions "
                            "that longer exists: " << tid);
        }
    } else {
        BPLOG_WARN_STRM("received callback invocation for an instance that "
                        "no longer exists: " << instance);
    }
}

void
DynamicServiceManager::onPrompt(ServiceRunner::Controller * c,
                                unsigned int instance, 
                                unsigned int promptId,
                                const bp::file::Path & pathToDialog,
                                const bp::Object * arguments)
{
    shared_ptr<DynamicServiceInstance> sptr =
        m_state.findInstance(c, instance);

    if (sptr != NULL) {
        shared_ptr<CoreletExecutionContext> context =
            sptr->m_context.lock();

        if (context == NULL) {
            BPLOG_WARN_STRM("received prompt request, however execution "
                            "context (client), has gone away, dropping.");
        } else {
            context->promptUser(sptr, promptId, pathToDialog, arguments);
        }
    } else {
        BPLOG_WARN_STRM("received prompt request for an instance that "
                        "no longer exists: " << instance);
    }
}

void
DynamicServiceManager::onInstanceShutdown(DynamicServiceInstance * instance)
{
    // first let's instruct the controller that this instance has been
    // destroyed
    shared_ptr<ServiceRunner::Controller> controller = 
        m_state.getRunningController(instance->m_summary);

    if (controller != NULL) {
        // inform the controller of the destruction of the instance
        controller->destroy(instance->m_instanceId);
    }

    // now erase all references to this instance
    m_state.removeInstance(instance);
}

void
DynamicServiceManager::onInstanceExecute(DynamicServiceInstance * instance,
                                         unsigned int clientTid,
                                         const std::string & function,
                                         const bp::Object & args)
{
    // we'll pass this call off to the controller and log an
    // entry in the tid map stored on the client
    shared_ptr<ServiceRunner::Controller> controller = 
        m_state.getRunningController(instance->m_summary);

    if (controller != NULL) {
        unsigned int tid;
        
        // inform the controller of the destruction of the instance
        tid = controller->invoke(instance->m_instanceId,
                                 function, &args);

        if (tid == (unsigned int) -1) {
            // couldn't find the correct controller!  Assume service crashed
            instance->sendFailure(clientTid, "bp.instanceError",
                                  "couldn't execute function, service no "
                                  "longer available");
        } else {
            instance->addClientTid(tid, clientTid);
        }
    } else {
        // couldn't find the correct controller!  Assume service crashed
        instance->sendFailure(clientTid, "bp.instanceError",
                              "couldn't execute function, service no "
                              "longer available");
    }
}

void
DynamicServiceManager::onPromptResponse(DynamicServiceInstance * instance,
                                        unsigned int promptId,
                                        const bp::Object& response)

{
    // we'll pass this call off to the controller and log an
    // entry in the tid map stored on the client
    shared_ptr<ServiceRunner::Controller> controller = 
        m_state.getRunningController(instance->m_summary);

    if (controller != NULL) {
        controller->sendResponse(promptId, &response);
    } else {
        BPLOG_WARN_STRM("cannot relay user prompt response, instance no "
                        "longer exists");
    }
}

