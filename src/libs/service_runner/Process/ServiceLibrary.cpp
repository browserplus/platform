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

/*
 * An abstraction around the dynamic library that composes a service.
 * abstracts dlloading and all interaction.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/15
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#include "ServiceLibrary.h"
#include "ServiceLibraryImpl.h"
#include "v5/ServiceLibrary_v5.h"
#include <ServiceAPI/bptypes.h>
#include <ServiceAPI/bpdefinition.h>
#include <ServiceAPI/bpcfunctions.h>
#include <ServiceAPI/bppfunctions.h>
#include "BPUtils/bpconfig.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/ProductPaths.h"

#include <stdarg.h>
#include <string.h>

using namespace std;
using namespace std::tr1;
using namespace ServiceRunner;
namespace bpf = bp::file;


ServiceLibrary::ServiceLibrary() : m_handle(NULL)
{
}
        
ServiceLibrary::~ServiceLibrary()
{
    if (m_handle)
    {
        dlcloseNP(m_handle);
        m_handle = NULL;
    }
}

// parse the manifest from the cwd, populating name and version
bool
ServiceLibrary::parseManifest(std::string & err)
{
    // XXX: service summary parsing is not properly within the service API
    //      version specific code.  This means that if we change the the
    //      format of the json manifest (i.e. summary), we'll have to introduce
    //      a fallback mechanism here so we can handle multiple manifest.json
    //      formats, *before* we've loaded the dynamic library and have access
    //      to the location where the service api version *really* lives.
    //      (yeah, we could include a version in manifest too, but that would
    //       be a DRY violation)
    return m_summary.detectService(bpf::canonicalPath(bpf::Path(".")), err);
}

std::string
ServiceLibrary::version()
{
    return m_impl->version();
}

std::string
ServiceLibrary::name()
{
    return m_impl->name();
}

// from now to infinity, the service API will require that a service
// exports a single function named BPPGetEntryPoints, who returns a
// structure whose first member is a 32 bit integer that is the
// service API version.

#define START_SYM_NAME "BPPGetEntryPoints"
struct FunctionTableBase 
{
    unsigned int serviceAPIVersion;
};

// load the service
bool
ServiceLibrary::load(const bpf::Path & providerPath, std::string & err)
{
    BPASSERT(m_handle == NULL);
    
    // now let's determine the path to the shared library.  For
    // dependent services this will be extracted from the manifest
    bpf::Path path;
    bp::service::Summary provider;
    
    if (m_summary.type() == bp::service::Summary::Dependent)
    {
        if (!provider.detectService(providerPath, err)) {
            BPLOG_ERROR_STRM("error loading dependent service: " << err);
            return false;
        }
        path = providerPath / provider.serviceLibraryPath();
    }
    else
    {
        path = m_summary.path() / m_summary.serviceLibraryPath();
    }
    path = bpf::canonicalPath(path);

    // now path contains the path to the shared library, regardless of whether
    // this is a native or dependent service.
    BPLOG_INFO_STRM("loading service library: " << bpf::utf8FromNative(path.filename()));

    m_handle = dlopenNP(path);

    if (m_handle == NULL)
    {
        BPLOG_ERROR_STRM("skipping '" << path
                         << "', failed to get address for "
                         "BPPGetEntryPoints symbol");
        return false;
    }

    void * (*entryPointFunc)(void);
    entryPointFunc = (void * (*)(void)) dlsymNP(m_handle, "BPPGetEntryPoints");

    if (entryPointFunc == NULL)
    {
        BPLOG_ERROR_STRM("skipping '" << path << "', failed to get address for "
                         "BPPGetEntryPoints symbol");
        dlcloseNP(m_handle);
        m_handle = NULL;
        return false;
    }

    void * funcTable = entryPointFunc();

    if (funcTable == NULL)
    {
        BPLOG_WARN_STRM("BPPGetEntryPoints returns invalid structure, bogus service");
        dlcloseNP(m_handle);
        m_handle = NULL;
        return false;
    }

    unsigned int version = ((FunctionTableBase * ) funcTable)->serviceAPIVersion;

    // now we know that the service is basically valid, and we have the version of the
    // service
    BPLOG_INFO_STRM("Attempting to load service with API version " << version);

    bool success = false;
    
    if (version == 5) 
    {
        m_impl.reset(new ServiceLibrary_v5);
        success = m_impl->load(m_summary, provider, funcTable);
    }

    if (!success) 
    {
        BPLOG_WARN_STRM("Failed to initialize service, unloading...");
        dlcloseNP(m_handle);
        m_handle = NULL;
    }
    
    return success;
}

unsigned int
ServiceLibrary::allocate(std::string uri, bpf::Path dataDir,
                         bpf::Path tempDir, std::string locale,
                         std::string userAgent, unsigned int clientPid)
{
    return m_impl->allocate(uri, dataDir, tempDir, locale, userAgent, clientPid);
}

void
ServiceLibrary::destroy(unsigned int id)
{
    m_impl->destroy(id);
}

bool
ServiceLibrary::invoke(unsigned int id, unsigned int tid,
                       const std::string & function,
                       const bp::Object * arguments,
                       std::string & err)
{
    return m_impl->invoke(id, tid, function, arguments, err);
}

void
ServiceLibrary::promptResponse(unsigned int promptId,
                               const bp::Object * arguments)
{
    m_impl->promptResponse(promptId, arguments);
}

void
ServiceLibrary::setListener(IServiceLibraryListener * listener)
{
    m_impl->setListener(listener);
}

unsigned int
ServiceLibrary::apiVersion()
{
    return m_impl->apiVersion();
}

const bp::service::Description &
ServiceLibrary::description()
{
    return m_impl->description();
}
