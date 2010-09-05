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
 * DiskScanner.cpp - a utility for scanning the disk and detecting the
 *                 available services.
 *
 * Major XXXs:
 *  1. threading issues when services are running
 *     (If there's a service running controlled by the main thread,
 *      then we'll have cross thread interaction between unprotected
 *      classes.  A simple solution would be to have one ServiceServer
 *      per thread)
 *  2. verifying dependent services when providers change
 *  3. handling of currently running services (not allowed to refresh/
 *     reload)
 */

#include "DiskScanner.h"
#include <map>
#include <stack>
#include <string>
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloopthread.h"
#include "BPUtils/bptimer.h"
#include "Permissions/Permissions.h"
#include "platform_utils/ProductPaths.h"
#include "platform_utils/ServiceInterfaceCache.h"
#include "ServiceRunnerLib/ServiceRunnerLib.h"

using namespace std;
using namespace std::tr1;

#ifdef WIN32
#pragma warning(disable:4706)   // assignment within conditional expression
#endif


// given a set of service summaries, attain descriptions from
// disk cache where approriate, a set of services with no entries in
// the interface cache will be returned
static std::set<bp::service::Summary>
loadFromCache(const std::set<bp::service::Summary> & summaries,
              std::map<bp::service::Summary, bp::service::Description> & oDesc,
              bool noDependents = false)
{
    std::set<bp::service::Summary> noLove;
    std::set<bp::service::Summary>::const_iterator i;

    for (i = summaries.begin(); i != summaries.end(); i++)
    {
        // skip dependents if flag is specified
        if (noDependents && (i->type() == bp::service::Summary::Dependent)) {
            noLove.insert(*i);
            continue;
        }

        bp::Object * descJson = NULL;
        bp::service::Description d;
        if (bp::serviceInterfaceCache::isNewerThan(i->name(), i->version(),
                                                   i->modDate()) &&
            (descJson = bp::serviceInterfaceCache::get(i->name(),
                                                       i->version())) &&
            d.fromBPObject(descJson))
        {
            oDesc[*i] = d;
        } else {
            noLove.insert(*i);
        }
        if (descJson) delete descJson;
    }

    return noLove;
}

// store a description to cache
static void
storeToCache(const bp::service::Description & description)
{
    bp::Object * o = description.toBPObject();
    if (!bp::serviceInterfaceCache::set(description.name(),
                                        description.versionString(), o)) {
        BPLOG_WARN( "Caching of service description failed!" );
    }
    
    if (o) {
        delete o;
    }
}

// given a dependant summary and a set of provider summaries, attain
// the path to the best match.  returns empty string if there is
// no viable match
static bp::file::Path
getBestProvider(const bp::service::Summary & dep,
                const std::set<bp::service::Summary> & providers)
{
    std::string name = dep.usesService();
    bp::SemanticVersion version = dep.usesVersion();
    bp::SemanticVersion minversion = dep.usesMinversion();

    bp::service::Summary winner;
    bp::SemanticVersion winnerVer;

    std::set<bp::service::Summary>::const_iterator i;

    for (i = providers.begin(); i != providers.end(); i++)
    {
        // XXX: we shouldn't have to parse every time :(
        bp::SemanticVersion current;
        if (!current.parse(i->version())) continue;

        if (bp::SemanticVersion::isNewerMatch(current, winnerVer,
                                              version, minversion))
        {
            winner = *i;
            winnerVer = current;
        }
    }

    if (!winner.name().empty()) return winner.path();
    
    return bp::file::Path();
}


// given a set of summaries, attain descriptions for them
/** a class to listen for UserQuitEvents and stop the runloop upon
 *  their arrival */
class DescriptionAggregator :  public ServiceRunner::IControllerListener
{
public:
    DescriptionAggregator(bp::runloop::RunLoopThread * rlt,
                          const std::string & logLevel,
                          const bp::file::Path & logFile)
        : m_rlt(rlt), m_logLevel(logLevel), m_logFile(logFile)
    {
    }

    // the set of summaries corresponding to descriptions we must extract
    std::set<bp::service::Summary> workToDo;
    std::set<bp::service::Summary> providers;

    // the output data structurs
    std::map<bp::service::Summary, bp::service::Description> descriptions;    
    std::set<bp::service::Summary> bogusSummaries;    

    static void atRunloopStart(void * cookie, bp::runloop::Event) 
    {
        DescriptionAggregator * dag = (DescriptionAggregator *) cookie;
        dag->attainNextDescription();
    }
    
private:   
    typedef std::pair<bp::service::Summary,
                      shared_ptr<ServiceRunner::Controller> >
      SummaryMapPair;
    
    typedef std::map<ServiceRunner::Controller *, SummaryMapPair>
      ControllerMap;

    ControllerMap controllers;

    void attainNextDescription()
    {
        while (workToDo.size() > 0 && controllers.size() < 4) {
            bp::service::Summary s = *(workToDo.begin());
            workToDo.erase(workToDo.begin());

            shared_ptr<ServiceRunner::Controller> controller(
                new ServiceRunner::Controller(s.name(), s.version()));
            controller->setListener(this);

            // get a provider for dependent services
            bp::file::Path providerPath;
            if (s.type() == bp::service::Summary::Dependent)
            {
                providerPath = getBestProvider(s, providers);
                if (providerPath.empty()) continue;
            }

            // get a reasonable title
            std::string processTitle, summary;            
            // TODO: get the proper locale here. (not hardcoded 'en')
            if (!s.localization("en", processTitle, summary))
            {
                processTitle.append("BrowserPlus: Spawned Service");
            }
            else
            {
                processTitle = (std::string("BrowserPlus: ") + processTitle);
            }

            std::string err;
            if (!controller->run(bp::paths::getRunnerPath(),
                                 providerPath, 
                                 processTitle, 
                                 m_logLevel,
                                 m_logFile,
                                 err))
            {
                BPLOG_WARN_STRM("Couldn't load " << s.name() << " - "
                                << s.version() << ": " << err);
            }
            else
            {
                controllers[controller.get()] = SummaryMapPair(s, controller);
            }
        }

		// terminate self once we've attained all descriptions
        if (workToDo.size() == 0 && controllers.size() == 0)
        {
            m_rlt->stop();
            return;
        }
  
    }

    bp::runloop::RunLoopThread * m_rlt;

    void initialized(ServiceRunner::Controller * c,
                     const std::string & service,
                     const std::string & version,
                     unsigned int) 
    {
        c->describe();
    }
    void onEnded(ServiceRunner::Controller * c) 
    {
        BPLOG_ERROR("Spawned service ended unexpectedly!");

        ControllerMap::iterator i;
        i = controllers.find(c);
        BPASSERT(i != controllers.end());

        // add to bogus summaries list
        bogusSummaries.insert(i->second.first);

        controllers.erase(i);
        attainNextDescription();
    }
    void onDescribe(ServiceRunner::Controller * c,
                    const bp::service::Description & desc)
    {
        ControllerMap::iterator i;
        i = controllers.find(c);
        BPASSERT(i != controllers.end());
        // successful acquisition of description
        descriptions[i->second.first] = desc;
        controllers.erase(i);
        attainNextDescription();
    }

    // unused overrides (because we pass off the controller to
    // the controller manager once it's initialized)
    void onAllocated(ServiceRunner::Controller *, unsigned int,
                     unsigned int) { }
    void onInvokeResults(ServiceRunner::Controller *,
                         unsigned int, unsigned int,
                         const bp::Object *) { }
    void onInvokeError(ServiceRunner::Controller *,
                       unsigned int, unsigned int,
                       const std::string &,
                       const std::string &) { }
    void onCallback(ServiceRunner::Controller *,
                    unsigned int, unsigned int,
                    long long int, const bp::Object *) { }
    void onPrompt(ServiceRunner::Controller *, unsigned int, unsigned int,
                  const bp::file::Path &,
                  const bp::Object *) { }

    std::string m_logLevel;
    bp::file::Path m_logFile;    
};

// attain descriptions of the specified services by spawning an
// instance of the service and extracting the description
//
// successfully attained descriptions will be returned in the
// descriptions output map.
//
// Service summaries that failed to load will be contained in the
// bogusSummaries output set.
static void
attainDescriptions(const std::set<bp::service::Summary> & summaries,
                   const std::set<bp::service::Summary> & providers,
                   std::map<bp::service::Summary, bp::service::Description>
                     & descriptions,
                   std::set<bp::service::Summary> & bogusSummaries,
                   const std::string &logLevel,
                   const bp::file::Path &logFile)
{
    bp::runloop::RunLoopThread rlt;
    DescriptionAggregator dag(&rlt, logLevel, logFile);
    dag.workToDo = summaries;
    dag.providers = providers;

    rlt.setCallBacks(NULL, NULL, NULL, NULL,
                     DescriptionAggregator::atRunloopStart, &dag);

    rlt.run();
    rlt.sendEvent(bp::runloop::Event(NULL));
    rlt.join();

    descriptions.insert(dag.descriptions.begin(), dag.descriptions.end());
    bogusSummaries.insert(dag.bogusSummaries.begin(), dag.bogusSummaries.end());
}

std::map<bp::service::Summary, bp::service::Description>
DiskScanner::scanDiskForServices(
    const bp::file::Path & pathArg,
    std::map<bp::service::Summary, bp::service::Description> lastScan,
    std::set<bp::service::Summary> running,
    const std::string & logLevel,
    const bp::file::Path & logFile)
{
    // stats about what we loaded

    // services we actually loaded
    unsigned int newServices = 0; 

    // services who we've already loaded an have not changed on disk
    unsigned int alreadyLoadedServices = 0;

    // services whose cached descriptions are up to date
    unsigned int cachedServices = 0;

    // services who we reloaded because cached descriptions are out of date
    unsigned int refreshedServices = 0;

    // services whose interfaces we need to reload, but cannot
    // because they are currently running
    unsigned int postponedServices = 0;

    bp::time::Stopwatch sw;
    sw.start();

    // a mapping of summaries to descriptions built up this pass
    std::map<bp::service::Summary, bp::service::Description> thisScan;

    // we'll keep provider summaries separate, as we'll verify/describe
    // these services first, then do everything else in (throttled)
    // parallel

    // a full set of providers used when we scan other services in
    // parallel to satisfy dependencies
    std::set<bp::service::Summary> providerSummaries;

    // a set of providers that we discovered this pass and we must load to
    // extract descriptions
    std::set<bp::service::Summary> neededProviderSummaries;    

    // a set of services that we discovered this pass and we must load
    std::set<bp::service::Summary> neededSummaries;

    // a handle to the permissions manager so we can prune along the way
    PermissionsManager* pmgr = PermissionsManager::get();

    // first let's just build a list of summaries
    std::stack<bp::file::Path> dirStack;
    dirStack.push(pathArg);
    
    while (dirStack.size() > 0)
    {
        bp::file::Path path = dirStack.top();
        dirStack.pop();
        
        if (!bp::file::isDirectory(path))
        {
            BPLOG_INFO_STRM("skipping '" << path << "', not directory.");
        }
        else 
        {
            unsigned int loadedServices = 0;
            unsigned int subDirectories = 0;

            try {
                bp::file::tDirIter end;
                for (bp::file::tDirIter it(path); it != end; ++it) {
                    bp::file::Path subpath(it->path());
                    // silently skip dot directories
                    if (subpath.string().compare(0, 1, bp::file::nativeFromUtf8(".")) == 0) 
                        continue;

                    // verify it's a directory
                    if (!bp::file::isDirectory(subpath))
                        continue;
                    
                    // check to see if this is a valid service
                    std::string error;
                    bp::service::Summary summary;
                        
                    if (!summary.detectService(subpath, error)) {
                        dirStack.push(subpath);
                        subDirectories++;
                        continue;
                    }
                    
                    // if this service is blacklisted, nuke it

                    std::string version = bp::file::utf8FromNative(subpath.filename());
                    std::string name = bp::file::utf8FromNative(subpath.parent_path().filename());
                    if (!pmgr->serviceMayRun(name, version))
                    {
                        bp::file::remove(subpath);
                        BPLOG_WARN_STRM("blacklisted service " 
                                        << name << "/" << version << " removed");
                        std::ofstream ofs;
                        if (bp::file::openWritableStream(
                                ofs, bp::paths::getServiceLogPath(),
                                std::ios_base::app | std::ios::binary))
                        {
                            
                            BPTime now;
                            ofs << now.asString()
                                << ": Removed blacklisted service " 
                                << name << ", version " << version << std::endl;
                        }
                    } 
                    else
                    {
                        // increment the counter that signifies that this is a
                        // directory containing something useful, not to be
                        // cleaned up
                        loadedServices++;

                        // we've now got a valid summary.  if we have already
                        // loaded it and it is not out of date, then we can
                        // transition it straight to the output map
                        std::map<bp::service::Summary, bp::service::Description>
                            ::iterator loadedIt;
                        loadedIt = lastScan.find(summary);

                        // can we skip the loading of this service because it's
                        // already been scanned and is up to date
                        if (loadedIt != lastScan.end())
                        {
                            if (!loadedIt->first.outOfDate() ||
                                running.find(summary) != running.end())
                            {
                                // update the correct count given the reason
                                // why we can skip loading this service
                                // (currently running or up to date)
                                if (loadedIt->first.outOfDate()) {
                                    postponedServices++;
                                } else {
                                    alreadyLoadedServices++;
                                }

                                thisScan[loadedIt->first] = loadedIt->second;

                                if (summary.type() ==
                                    bp::service::Summary::Provider)
                                {
                                    providerSummaries.insert(summary);
                                }
                                continue;
                            }
                            
                            // we have already loaded this service, but it
                            // needs to be refreshed!
                            refreshedServices++;

                            // let's remove the interface from the disk cache
                            bp::serviceInterfaceCache::purge(name, version);
                        }

                        if (summary.type() == bp::service::Summary::Provider)
                        {
                            neededProviderSummaries.insert(summary);
                        }
                        else
                        {
                            neededSummaries.insert(summary);
                        }
                    }
                }
                if (!loadedServices && !subDirectories) {
                    bp::file::Path fullPath(bp::file::canonicalPath(path));

                    BPLOG_WARN_STRM("removing empty directory: " << fullPath);
                    bp::file::remove(fullPath);
                }
            } catch (const bp::file::tFileSystemError& e) {
                BPLOG_WARN_STRM("unable to iterate thru " << path
                                << ": " << e.what());
            }
        }
    }

    // at this point we've scanned the plugin directory and built up some
    // data structures, we have:
    // * thisScan: populated with summary/desc of services that we needn't
    //             reload
    // * providerSummaries: set of providers that we've already loaded
    // * neededProviderSummaries: set of providers that we must attain
    //                      descriptions for.
    // * neededSummaries: set of no providers that we need to attain
    //                      descriptions for.

    // BEGIN loading of provider services

    // now let's get cached descriptions where possible (cached description
    // is newer than mod date of manifest.json)
    {
        std::set<bp::service::Summary> needed;
        needed = loadFromCache(neededProviderSummaries, thisScan);
        cachedServices += neededProviderSummaries.size() - needed.size();
        // now all successfully loaded new providers will get added to the
        // provider set for depedency satisfaction on the next step
        std::set<bp::service::Summary>::iterator i;
        for (i = neededProviderSummaries.begin();
             i != neededProviderSummaries.end();
             i++)
        {
            if (thisScan.find(*i) != thisScan.end())
            {
                providerSummaries.insert(*i);
            }
        }
        neededProviderSummaries = needed;
    }

    // a data structure where we'll collect services which fail to
    // load (corrupted on disk, or invalid)
    std::set<bp::service::Summary> bogusServices;

    // First let's attain descriptions for provider summaries requiring
    // a scan.
    if (neededProviderSummaries.size() > 0)
    {
        unsigned int before = thisScan.size();
        attainDescriptions(neededProviderSummaries,
                           std::set<bp::service::Summary>(),
                           thisScan, bogusServices,
                           logLevel, logFile);
        newServices += thisScan.size() - before;

        // now all successfully loaded new providers will get added to the
        // provider set for depedency satisfaction on the next step
        std::set<bp::service::Summary>::iterator i;
        for (i = neededProviderSummaries.begin();
             i != neededProviderSummaries.end();
             i++)
        {
            if (bogusServices.find(*i) == bogusServices.end())
            {
                providerSummaries.insert(*i);
                // store interface description to cache
                storeToCache(thisScan[*i]);
            }
        }

        // in the case that a provider changes, we'll force rescanning
        // of all dependent services and disable the disk cache.
        // this is slightly overly conservative.
        std::map<bp::service::Summary, bp::service::Description>::iterator it;
        for (it = thisScan.begin(); it != thisScan.end(); it++)
        {
            if (it->first.type() == bp::service::Summary::Dependent) {
                neededSummaries.insert(it->first);
                alreadyLoadedServices--;
            }
        }
        for (i = neededSummaries.begin(); i != neededSummaries.end(); i++) {
            it = thisScan.find(*i);
            if (it != thisScan.end()) thisScan.erase(it);
        }
    }

    // BEGIN loading of standalone/dependent services

    // now let's get cached descriptions where of standalone and dependent
    // services.  
    {
        std::set<bp::service::Summary> needed;
        // don't load dependents from cache if providers have changed
        needed = loadFromCache(neededSummaries, thisScan,
                               neededProviderSummaries.size() != 0);
        cachedServices += neededSummaries.size() - needed.size();
        neededSummaries = needed;
    }

    // now let's try get descriptions of the remaining services
    if (neededSummaries.size() > 0) {
        unsigned int before = thisScan.size();
        attainDescriptions(neededSummaries, providerSummaries, thisScan,
                           bogusServices, logLevel, logFile);
        newServices += thisScan.size() - before;

        // now update disk interface cache
        std::set<bp::service::Summary>::iterator i;
        for (i = neededSummaries.begin(); i != neededSummaries.end(); i++)
        {
            if (bogusServices.find(*i) == bogusServices.end())
            {
                storeToCache(thisScan[*i]);
            }
        }
    }
    
    // now descs is a complete set of successfully loaded descriptions,
    // bogusSummaries is everything that's broken
    
    BPLOG_INFO_STRM("Service disk scan complete in "
                    << sw.elapsedSec() << "s, "
                    << thisScan.size() << " loaded");
    // now we're reporting basic stats in log at info level (info justified
    // because scanning is fairly rare, and probably interesting)
    if (newServices > 0) {
        if (refreshedServices > 0) {
            BPLOG_INFO_STRM(newServices << " service(s) loaded from disk ("
                            << refreshedServices << " had changed).");
        } else {
            BPLOG_INFO_STRM(newServices << " service(s) loaded from disk.");
        }
    } 
    
    if (alreadyLoadedServices > 0) {
        BPLOG_INFO_STRM(alreadyLoadedServices << " already loaded service(s) "
                        << "detected.");
    }

    if (cachedServices) {
        BPLOG_INFO_STRM(cachedServices << " service interface(s) hydrated "
                        "from disk cache");
    }

    if (postponedServices) {
        BPLOG_INFO_STRM(postponedServices << " service(s) changed on disk, but "
                        << "is/are currently running (update postponed)");
    }

    if (bogusServices.size()) {
        BPLOG_WARN_STRM(bogusServices.size() << " broken service(s) detected!");
        std::set<bp::service::Summary>::iterator i;

        for (i = bogusServices.begin(); i != bogusServices.end(); i++)
        {
            BPLOG_WARN_STRM("removing damaged service: " << i->path());
            bp::file::remove(i->path());
        }
    }
    
    return thisScan;
}

