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

#include "CommandExecutor.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/OS.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/bperrorutil.h"
#include "CoreletManager/CoreletManager.h"
#include "Permissions/Permissions.h"

using namespace std;
using namespace std::tr1;


#ifdef WIN32
#pragma warning(disable:4100)
#endif

class CommandExecutorRunner : public IDistQueryListener
{
public:
    CommandExecutorRunner(std::list<std::string> distroServers)
        : m_dqHand(distroServers, PermissionsManager::get())
    {
        m_dqHand.setListener(this);
        m_registry.reset(new CoreletRegistry(std::string(), bp::file::Path()));
        m_registry->setPluginDirectory(bp::paths::getCoreletDirectory());
    }

    DistQuery m_dqHand;
    shared_ptr<CoreletRegistry> m_registry;

    virtual void onTransactionFailed(unsigned int tid);
    virtual void gotAvailableServices(unsigned int tid,
                                      const CoreletList & list);
    virtual void onServiceFound(unsigned int tid,
                                const AvailableCorelet & list);
    virtual void onDownloadProgress(unsigned int tid,
                                    unsigned int pct);
    virtual void onDownloadComplete(unsigned int tid,
                                    const std::vector<unsigned char> & buf);
    virtual void gotServiceDetails(unsigned int tid,
                                   const bp::service::Description & desc);
    virtual void gotPermissions(unsigned int tid,
                                std::vector<unsigned char> permBundle);
    virtual void onPageUsageReported(unsigned int tid);
    virtual void onRequirementsSatisfied(unsigned int tid,
                                         const CoreletList & clist);
    virtual void onCacheUpdated(unsigned int tid,
                                const CoreletList & updates);
    virtual void gotServiceSynopsis(unsigned int tid,
                                    const ServiceSynopsisList & sslist);
    virtual void gotLatestPlatformVersion(unsigned int tid,
                                          const std::string & latest);
    virtual void onLatestPlatformDownloaded(
        unsigned int tid,
        const LatestPlatformPkgAndVersion & pkgAndVersion);

    CommandExecutor * ce;

    void onSuccess() { ce->onSuccess(); }
    void onFailure() { ce->onFailure(); }
};


static bool
parseJSONRequires(std::list<CoreletRequireStatement> &reqStmts,
                  std::string json)
{
    using namespace bp;    

    // parse
    Object * obj = NULL;
    {
        std::string err;
        obj = Object::fromPlainJsonString(json, &err);
        if (!obj) {
            std::cout << "couldn't parse json:" << std::endl
                      << err.c_str() << std::endl;
            return false;
        }
    }

    // iterate and build up list
    if (obj->type() == BPTList) {
        List * l = dynamic_cast<List *>(obj);
        BPASSERT(l != NULL);
        
        for (unsigned int i = 0; i < l->size(); i++) {
            const Map * m = dynamic_cast<const Map *>(l->value(i));
            if (m == NULL) continue;

            CoreletRequireStatement reqStmt;
            
            const String * s = dynamic_cast<const String*>(m->value("name"));
            if (s == NULL) continue;
            reqStmt.m_name = s->value();

            s = dynamic_cast<const String*>(m->value("ver"));
            if (s != NULL) {
                reqStmt.m_version = s->value();
            }

            s = dynamic_cast<const String*>(m->value("minver"));
            if (s != NULL) {
                 reqStmt.m_minversion = s->value();
            }
            reqStmts.push_back(reqStmt);
        }
    }

    if (obj != NULL) delete obj;

    return true;
}

static CoreletList
parseJSONCoreletList(std::string json)
{
    using namespace bp;    

    CoreletList clp;

    // parse
    Object * obj = NULL;
    {
        std::string err;
        obj = Object::fromPlainJsonString(json, &err);
        if (!obj) {
            std::cout << "couldn't parse json:" << std::endl
                      << err.c_str() << std::endl;
            return CoreletList();
        }
    }

    // iterate and build up list
    if (obj->type() == BPTList) {
        List * l = dynamic_cast<List *>(obj);
        BPASSERT(l != NULL);
        
        for (unsigned int i = 0; i < l->size(); i++) {
            const Map * m = dynamic_cast<const Map *>(l->value(i));
            if (m == NULL) continue;

            std::string name, version;
            
            const String * s = dynamic_cast<const String*>(m->value("name"));
            if (s == NULL) continue;
            name = s->value();

            s = dynamic_cast<const String*>(m->value("version"));
            if (s == NULL) continue;
            version = s->value();

            clp.push_back(
                std::pair<std::string, std::string>(name, version));
        }
    }

    if (obj != NULL) delete obj;

    return clp;
}

static void
printReqStmts(const std::list<CoreletRequireStatement> & reqStmts)
{
    std::list<CoreletRequireStatement>::const_iterator it;
        
    for (it = reqStmts.begin(); it != reqStmts.end(); it++) {
        std::cout << " * " << it->m_name;
        if (!it->m_version.empty()) {
            std::cout << " v. " << it->m_version;
        }
        if (!it->m_minversion.empty()) {
            std::cout << " mv. " << it->m_minversion;
        }
        std::cout << std::endl;
    }
}

static void
printCoreletList(CoreletList cl)
{
    std::list<std::pair<std::string, std::string> >::iterator it;
    for (it = cl.begin(); it != cl.end(); it++)
    {
        std::cout << " * " << it->first << " - " << it->second << std::endl;
    }
}
    
CommandExecutor::CommandExecutor(std::list<std::string> distroServers)
    : CommandHandler(), m_runner(NULL)
{
    m_runner = new CommandExecutorRunner(distroServers);
    m_runner->ce = this;
}

CommandExecutor::~CommandExecutor()
{
    delete m_runner;
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::available)
{
    std::string platform;
    if (tokens.size() == 1) platform = tokens[0];
    else platform = bp::os::PlatformAsString();
    m_runner->m_dqHand.availableServices(platform);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::permissions)
{
    m_runner->m_dqHand.permissions();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::details)
{
    m_runner->m_dqHand.coreletDetails(tokens[0], tokens[1], tokens[2]);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::latestPlatformVersion)
{
    m_runner->m_dqHand.latestPlatformVersion(bp::os::PlatformAsString());
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::downloadLatestPlatform)
{
    m_runner->m_dqHand.downloadLatestPlatform(bp::os::PlatformAsString());
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::find)
{
    std::string platform, name, version, minversion;
    // allowed between 2 and 4 args
    platform = tokens[0];
    name = tokens[1];
    if (tokens.size() > 2) version = tokens[2];
    if (tokens.size() > 3) minversion = tokens[3];    
    m_runner->m_dqHand.findService(name, version, minversion, platform);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::platform)
{
    std::cout << "    platform:     " << bp::os::PlatformAsString()
              << std::endl;
    std::cout << "    version:      " << bp::os::PlatformVersion() << std::endl;
    std::cout << "    service pack: " << bp::os::ServicePack() << std::endl;
    std::cout << "    current user: " << bp::os::CurrentUser() << std::endl;
    std::cout << "    64 bit OS?:   " << bp::os::Is64Bit() << std::endl;

    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::satisfy)
{
    using namespace bp;
    std::string json;
    std::string platform;
    bool useInstalled = false;

    // if 2 args, first is options
    if (tokens.size() == 2) {
        std::string err;
        Object * obj = Object::fromPlainJsonString(tokens[0], &err);
        if (!obj) {
            std::cout << "couldn't parse json:" << std::endl
                      << err.c_str() << std::endl;
            onFailure();        
            return;
        }
        Map * m = dynamic_cast<Map*>(obj);
        BPASSERT(m != NULL);
        if (!m) {
            std::cout << "expected a map, got " << tokens[0] << std::endl;
            onFailure();
	    return;
        }
        const String * s = dynamic_cast<const String*>(m->value("platform"));
        if (s) {
            platform = s->value();
        }
        const Bool * b = dynamic_cast<const Bool*>(m->value("useInstalled"));
        if (b != NULL) useInstalled = b->value();
        json.append(tokens[1]);
    } else {
        json.append(tokens[0]);
    }
    if (platform.empty()) {
        platform = bp::os::PlatformAsString();
    }

    // now we must parse json and turn it into a list of
    // CoreletRequireStatements
    std::list<CoreletRequireStatement> reqStmts;
    if (!parseJSONRequires(reqStmts, json)) {
        onFailure();
        return;
    }

    // pass off control to DistQuery to satisfy requirements
    std::list<bp::service::Summary> installed;
    if (useInstalled) {
        installed = m_runner->m_registry->availableCoreletSummaries();
    }
    
    // output what we parsed
    std::cout << "attempting to satify " << reqStmts.size()
              <<  " requirements: (useInstalled = " << useInstalled << ")"
              << std::endl;
    printReqStmts(reqStmts);
    
    m_runner->m_dqHand.satisfyRequirements(platform, reqStmts, installed);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::installed)
{
    std::list<bp::service::Summary> installed =
        m_runner->m_registry->availableCoreletSummaries();

    std::list<bp::service::Summary>::iterator it;
    unsigned int i;
    for (i=1, it = installed.begin(); it != installed.end(); it++, i++)
    {
        std::cout << i << ": " << it->name() << " - "
                  << it->version() << std::endl;
    }

    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::cached)
{
    CoreletList cl = m_runner->m_dqHand.cachedCorelets();

    std::cout << cl.size()
              << " pending corelet updates"
              << (cl.size() ? ":" : "")
              << std::endl;
    printCoreletList(cl);
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::purgeCache)
{
    m_runner->m_dqHand.purgeCache();
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::isCached)
{
    bool cached = m_runner->m_dqHand.isCached(tokens[0], tokens[1]);
    std::cout << tokens[0] << " - " << tokens[1] << " "
              << (cached ? "" : "NOT ") << "found in cache" << std::endl;
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::installFromCache)
{
    bool instd = m_runner->m_dqHand.installCoreletFromCache(tokens[0], tokens[1]);
    std::cout << tokens[0] << " - " << tokens[1] << " "
              << (instd ? "successfully" : "could NOT be")
              << " installed" << std::endl;
    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::strings)
{
    std::cout << "platform: " << tokens[0] << std::endl
              << "locale:   " << tokens[1] << std::endl;
    
    CoreletList cl = parseJSONCoreletList(tokens[2]);

    m_runner->m_dqHand.serviceSynopses(tokens[0], tokens[1], cl);
}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::haveUpdates)
{
    std::list<CoreletRequireStatement> reqStmts;
    if (!parseJSONRequires(reqStmts, tokens[0])) {
        onFailure();
        return;
    }

    std::list<bp::service::Summary> installed =
        m_runner->m_registry->availableCoreletSummaries();

    std::cout << "attempting to determine if updates are available which "
              << "better satisfy reqs:" << std::endl;
    printReqStmts(reqStmts);
    CoreletList cl = m_runner->m_dqHand.haveUpdates(reqStmts, installed);
    
    std::cout << cl.size()
              << " pertinent corelet updates available"
              << (cl.size() ? ":" : "")
              << std::endl;
    printCoreletList(cl);
    onSuccess();

}

BP_DEFINE_COMMAND_HANDLER(CommandExecutor::updateCache)
{
    std::string platform = bp::os::PlatformAsString();

    std::list<CoreletRequireStatement> reqStmts;
    if (!parseJSONRequires(reqStmts, tokens[0])) {
        onFailure();
        return;
    }

    std::list<bp::service::Summary> installed =
        m_runner->m_registry->availableCoreletSummaries();

    std::cout << "attempting to download service updates using require "
              << "history:" << std::endl;
    printReqStmts(reqStmts);
    m_runner->m_dqHand.updateCache(platform, reqStmts, installed);
}

void
CommandExecutorRunner::onTransactionFailed(unsigned int tid)
{
    std::cout << "transaction failed" << std::endl;
    onFailure();
}

void
CommandExecutorRunner::gotAvailableServices(unsigned int tid,
                                         const CoreletList & list)
{
    std::cout << list.size() << " available corelets:" << std::endl;
    printCoreletList(list);
    onSuccess();
}


void
CommandExecutorRunner::onServiceFound(unsigned int tid,
                                   const AvailableCorelet & service)
{
    std::cout << "Found viable corelet: "
              << service.name
              << " ver. " 
              << service.version.asString()
              << std::endl;

    if (service.dependentCorelet) {
        const char * version = NULL;
        const char * minversion = NULL;
        const char * name= "(unknown)";
        if (!service.providerName.empty()) {
            name = service.providerName.c_str();
        }
        if (!service.providerVersion.empty()) {
            version = service.providerVersion.c_str();
        }
        if (!service.providerMinversion.empty()) {
            minversion = service.providerMinversion.c_str();
        }

        std::cout << "  requires '" << name;
        if (version) std::cout << ", ver. (" << version << ")";
        if (minversion) std::cout << ", minver. (" << minversion << ")";
        std::cout << std::endl;
    }
        
    onSuccess();
}


void
CommandExecutorRunner::onDownloadProgress(unsigned int tid,
                                          unsigned int pct)
{
    printf("%d%%\n", pct);
}


void
CommandExecutorRunner::onDownloadComplete(unsigned int tid,
                                       const std::vector<unsigned char> & buf)
{
}


void
CommandExecutorRunner::gotServiceDetails(unsigned int tid,
                                      const bp::service::Description & desc)
{
    bp::Object* o = desc.toBPObject();
    std::cout << o->toPlainJsonString(true) << std::endl;
    delete o;
    onSuccess();
}


void
CommandExecutorRunner::gotPermissions(unsigned int tid,
                                std::vector<unsigned char> permBundle)
{
    using namespace std;
    using namespace bp::paths;
    using namespace bp::strutil;
    using namespace bp;
    namespace bpf = bp::file;
        
    bpf::Path tmpPath = bpf::getTempPath(bpf::getTempDirectory(), "perms_pkg");
    try {
        // payload is bpkg of json
        string pkgStr((const char *) &(permBundle[0]), permBundle.size());
        if (!bp::strutil::storeToFile(tmpPath, pkgStr)) {
            throw runtime_error("unable to save data to " + tmpPath.externalUtf8());
        }
        string errMsg;
        string jsonStr;
        BPTime timestamp;
        if (!bp::pkg::unpackToString(tmpPath, jsonStr, timestamp, errMsg)) {
            throw runtime_error("unable to unpack permissions: " + errMsg);
        }

        // show the contents
        Map* m = dynamic_cast<Map*>(Object::fromPlainJsonString(jsonStr));
        if (!m) {
            throw runtime_error("malformed response");
        }

        // corelet blacklist is a list of coreletname/version pairs
        // gets turned into a map whose key is coreletname and whose value
        // is list of versions
        const List* l = dynamic_cast<const List*>(m->get("blacklist"));
        if (l) {
            cout << "blacklist:" << endl;
            for (unsigned int i = 0; l && i < l->size(); i++) {
                const List* p = dynamic_cast<const List*>(l->value(i));
                if (p && p->size() == 2) {
                    const String* name = dynamic_cast<const String*>(p->value(0));
                    const String* vers = dynamic_cast<const String*>(p->value(1));
                    if (name && vers) {
                        cout << "\t" << name->value() << " " << vers->value() << endl;
                    }
                }
            }
            cout << endl;
        }

        // platform blacklist is a list of platform strings
        l = dynamic_cast<const List*>(m->get("platformBlacklist"));
        if (l) {
            cout << "platformBlacklist:" << endl;
            for (unsigned int i = 0; l && i < l->size(); i++) {
                const String* s = dynamic_cast<const String*>(l->value(i));
                if (s) {
                    cout << "\t" << s->value() << endl;
                }
            }
            cout << endl;
        }
            
        delete m;
    } catch (const runtime_error& e) {
        cerr << "ERROR: " << e.what() << endl;
    }
    remove(tmpPath);
    onSuccess();
}


void
CommandExecutorRunner::onPageUsageReported(unsigned int tid)
{
}


void
CommandExecutorRunner::onRequirementsSatisfied(unsigned int tid,
                                            const CoreletList & clist)
{
    std::cout << clist.size() << " required corelets:" << std::endl;
    printCoreletList(clist);
    onSuccess();
}


void
CommandExecutorRunner::onCacheUpdated(unsigned int tid,
                                   const CoreletList & updates)
{
    if (updates.size()) {
        std::cout << "cache updated, "
                  << updates.size()
                  << " corelets downloaded:" << std::endl;            
        printCoreletList(updates);
    } else {
        std::cout << "no new updates available." << std::endl;
    }

    onSuccess();
}


void
CommandExecutorRunner::gotServiceSynopsis(unsigned int tid,
                                       const ServiceSynopsisList & lst)
{
    std::cout << "got localized descriptions:" << std::endl;
    std::list<ServiceSynopsis>::const_iterator it;

    for (it = lst.begin(); it != lst.end(); it++)
    {
        std::cout << " * " << it->m_name
                  << "|" << it->m_version
                  << ": \"" << it->m_title
                  << "\" -- " << it->m_summary
                  << std::endl;
        if (it->m_permissions.size() != 0) {
            std::cout << "   Permissions required: ";
            std::set<std::string>::const_iterator sit;
            for (sit = it->m_permissions.begin();
                 sit != it->m_permissions.end();
                 sit++)
            {
                std::cout << *sit << " ";
            }
            std::cout << std::endl;
        }
            

    }

    onSuccess();
}

void
CommandExecutorRunner::gotLatestPlatformVersion(unsigned int tid,
                                             const std::string & latest)
{
    std::cout << "Latest available platform is " << latest << std::endl;
    onSuccess();        
}

void
CommandExecutorRunner::onLatestPlatformDownloaded(
        unsigned int tid,
        const LatestPlatformPkgAndVersion & pkgAndVersion)
{
    std::cout << "Downloaded latest platform ("
              << pkgAndVersion.m_version
              << ") " << pkgAndVersion.m_pkg.size() << " bytes."
              << std::endl;
    onSuccess();        
}


