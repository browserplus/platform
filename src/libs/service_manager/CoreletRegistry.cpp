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
 * CoreletRegistry
 *
 * A collection of available corelets from which corelets may be
 * described, and instances of corelets may be attained.
 */

#include "CoreletRegistry.h"
#include "BPUtils/BPLog.h"
#include "DynamicServiceManager.h"

using namespace std;
using namespace std::tr1;


CoreletRegistry::CoreletRegistry(const std::string & loglevel,
                                 const bp::file::Path & logfile)
{
    m_dynamicManager.reset(new DynamicServiceManager(loglevel, logfile));
}

CoreletRegistry::~CoreletRegistry()
{
}

bool
CoreletRegistry::registerCorelet(const bp::service::Description & desc,
                                 shared_ptr<CoreletFactory> factory)
{
    if (getReg(desc.name(), desc.versionString(), std::string()).second != NULL)
    {
        return false;
    }

    DescFactPair reg(desc, factory);
	reg.first.setIsBuiltIn(true);
    m_registrations.push_back(reg);

    return true;
}

bool 
CoreletRegistry::purgeCorelet(const std::string & name,
                              const std::string & version)
{
    return m_dynamicManager->purgeService(name, version);
}

void
CoreletRegistry::forceRescan()
{
    m_dynamicManager->forceRescan();
}

bool
CoreletRegistry::isBusy()
{
    return m_dynamicManager->isBusy();
}

void
CoreletRegistry::unregisterAll()
{
    m_registrations.clear();
}

std::list<bp::service::Description>
CoreletRegistry::availableCorelets()
{
    std::list<bp::service::Description> x;
    std::list<DescFactPair>::iterator it;

    // append descriptions of all synthetic corelets.
    for (it = m_registrations.begin(); it != m_registrations.end(); it++)
    {
        x.push_back(it->first);
    }
    
    // now append all available dynamic services
    std::vector<bp::service::Description> dyn =
        m_dynamicManager->availableServices();

    for (unsigned int i = 0; i < dyn.size(); i++) {
        x.push_back(dyn[i]);
    }

    return x;
}

std::list<bp::service::Summary>
CoreletRegistry::availableCoreletSummaries()
{
    // first get descriptions of the dynamic corelets
    std::list<bp::service::Summary> x;

    // now the builtins
    std::list<DescFactPair>::iterator it;
    for (it = m_registrations.begin(); it != m_registrations.end(); it++)
    {
        x.push_back(it->second->summary());
    }

    // now append all available dynamic services
    std::vector<bp::service::Summary> dyn =
        m_dynamicManager->availableServiceSummaries();

    for (unsigned int i = 0; i < dyn.size(); i++) {
        x.push_back(dyn[i]);
    }
    
    return x;
}

bool
CoreletRegistry::describe(const std::string & name,
                          const std::string & version,
                          const std::string & minversion,
                          bp::service::Description & oDescription)
{
	DescFactPair reg = getReg(name, version, minversion);

	if (reg.second == NULL) {
        return m_dynamicManager->describe(name, version, minversion,
                                          oDescription);
    }
    oDescription = reg.first;

    return true;
}

bool
CoreletRegistry::summary(const std::string & name,
                         const std::string & version,
                         const std::string & minversion,
                         bp::service::Summary & oSummary)
{
	DescFactPair reg = getReg(name, version, minversion);
	if (reg.second != NULL) {
        oSummary = reg.second->summary();
        return true;
    }
    return m_dynamicManager->summary(name, version, minversion, oSummary);
}

bool
CoreletRegistry::haveCorelet(const std::string & name,
                             const std::string & version,
                             const std::string & minversion)
{
    // check built-in corelets
	DescFactPair reg = getReg(name, version, minversion);
	if (reg.second != NULL) return true;

    // otherwise check dynamic 
    return m_dynamicManager->haveService(name, version, minversion);
}

struct CRInstanceContext
{
    shared_ptr<CoreletInstance> inst;
    weak_ptr<ICoreletRegistryListener> listener;
    unsigned int instantiateId;
    
};

void
CoreletRegistry::onHop(void * x)
{
    CRInstanceContext * ctx = (CRInstanceContext *) x;
    assert(ctx != NULL);

    shared_ptr<ICoreletRegistryListener> regListener;
    regListener = ctx->listener.lock();

    if (regListener == NULL) {
        BPLOG_ERROR("service allocation completed, but listener has "
                    "dissapeared.  deleting newly allocated instance.");
    } else {
        // this call will transfer ownership of the instance to the listener
        regListener->gotInstance(ctx->instantiateId, ctx->inst);
    }

    delete ctx;
}

unsigned int
CoreletRegistry::instantiate(
    const std::string & name,
    const std::string & version,
    weak_ptr<CoreletExecutionContext> context,
    weak_ptr<ICoreletRegistryListener> listener)
{
    shared_ptr<CoreletFactory> fact =
        getReg(name, version, std::string()).second;

    if (fact != NULL)
    {
        shared_ptr<CoreletInstance> inst;
        inst = fact->instantiateInstance(context);
        if (inst == NULL) return 0;

        // now we're ready to hop!  we must convey the instance, the
        // listener, and the id.  This is all so we don't call our
        // listener before this function returns
        unsigned int instantiateId =
            m_dynamicManager->getUniqueInstantiateId();

        CRInstanceContext * ctx = new CRInstanceContext;
        ctx->inst = inst;
        ctx->listener = listener;
        ctx->instantiateId = instantiateId;
        hop((void *) ctx);
        return instantiateId;
    }

    return m_dynamicManager->instantiate(name, version, context, listener);
}


CoreletRegistry::DescFactPair
CoreletRegistry::getReg(const std::string & name,
                        const std::string & version,
                        const std::string & minversion)
{
    DescFactPair found;
    std::list<DescFactPair>::iterator it;

    // the version we want
    bp::ServiceVersion wantver;
    // the minimum version we want
    bp::ServiceVersion wantminver;
    // the version we've found
    bp::ServiceVersion got;

    if (name.empty()) {
        return found;
    }

    if (!version.empty() && !wantver.parse(version))
    {
        return found;
    }
    
    if (!minversion.empty() && !wantminver.parse(minversion))
    {
        return found;
    }

    for (it = m_registrations.begin(); it != m_registrations.end(); it++)
    {
        if (!it->first.name().compare(name))
        {
            // we've found a corelet with the correct name.  now check
            // it's version.  To be a winner it must:
            // 1. be newer than what we've alread found
            // 2. match the wantver
            // 3. be greater than the minver

            // the version we're considering
            bp::ServiceVersion current;
            current.setMajor((int) it->first.majorVersion());
            current.setMinor((int) it->first.minorVersion());
            current.setMicro((int) it->first.microVersion());

            // is this a newer match than what we've already got?
            if (!bp::ServiceVersion::isNewerMatch(current, got,
                                                  wantver, wantminver))
            {
                continue;
            }

            // passed our tests! 
            found = *it;
            got = current;
        }
    }
    
    return found;
}

void
CoreletRegistry::setPluginDirectory(const bp::file::Path & path)
{
    m_dynamicManager->setPluginDirectory(path);
}
