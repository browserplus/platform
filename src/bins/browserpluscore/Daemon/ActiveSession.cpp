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
 * ActiveSession.h
 * Command handlers for presence server
 *
 * Created by Lloyd Hilaiel on or around Wed May  9 17:39:04 MDT 2007
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 *
 * Owner: Lloyd Trevor Hilaiel (LtH) 
 */

#include "ActiveSession.h"
#include "BPUtils/ProductPaths.h"
#include "BPUtils/ServicesUpdatedFile.h"
#include "I18n/idna.h"
#include "Permissions/Permissions.h"
#include "SystemConfig.h"
#include "UsageReporting.h"

using namespace std;
using namespace std::tr1;


static const char* sDefaultLocale = "en";

static void
populateErrorResponse(bp::ipc::Response & oResponse,
                      const std::string & error,
                      const std::string & verboseError = std::string())
{
    bp::Map m;

    if (!error.empty()) m.add("error", new bp::String(error));
    else m.add("error", new bp::String("BP.genericError"));

    if (!verboseError.empty()) {
        m.add("verboseError", new bp::String(verboseError));
    }

    m.add("success", new bp::Bool(false));
    
    oResponse.setPayload(m);
}

static void
populateSuccessResponse(bp::ipc::Response & oResponse,
                        const bp::Object * payload = NULL)
{
    bp::Map * m = new bp::Map;
    m->add("success", new bp::Bool(true));
    if (payload) m->add("value", payload->clone());
    oResponse.setPayload(m);
}

ActiveSession::ActiveSession(bp::ipc::Channel * session,
                             shared_ptr<CoreletRegistry> registry,
                             const std::string & primaryDistroServer,
                             const std::list<std::string> secondaryDistroServers)
    : CoreletExecutionContext(), m_sessionMessage(NULL),
      m_createSessionCalled(false), m_listener(NULL),
      m_primaryDistroServer(primaryDistroServer),
      m_secondaryDistroServers(secondaryDistroServers)
{
    m_locale = sDefaultLocale;  // default if not set
    m_clientPid = 0;
    m_session = session;
    m_registry = registry;
}


ActiveSession::~ActiveSession()
{
    m_requireRequests.clear();
    
    // all outstanding message requests now are failures
    // (lth) don't bother responding to them
    std::list<MessageContext*>::iterator lit;
    for (lit = m_messages.begin(); lit != m_messages.end(); ++lit) 
    {
        delete (*lit);
    }
    m_messages.clear();

    if (m_session) {
        m_session->setListener(NULL);
        m_session->disconnect();
        delete m_session;
        m_session = NULL;
    }
    
    std::map<unsigned int,
        std::pair<weak_ptr<ICoreletExecutionContextListener>,
                  unsigned int> >::iterator it;

    // all outstanding user prompt requests now are failures.
    for (it = m_promptRequests.begin(); it != m_promptRequests.end(); it++)
    {
        BPLOG_WARN_STRM("[" << m_session
                        << "] prompt user request ("
                        << it->second.second
                        << ") fails, session is being torn down");
        shared_ptr<ICoreletExecutionContextListener>
            ptr = it->second.first.lock();
        if (ptr) ptr->onUserResponse(it->second.second, bp::Null());
    }
    m_promptRequests.clear();

    // Report page usage now at session termination time so we do not add
    // any latency
    bp::usage::reportPageUsage( m_URI, m_userAgent );
    
    // just in case, release our hold on the require lock
    // noop if we don't have it
    RequireLock::releaseLock(m_thisWeak);
}

std::string
ActiveSession::locale()
{
    return m_locale;
}

std::string
ActiveSession::URI() 
{
    return m_URI;
}

std::string
ActiveSession::userAgent()
{
    return m_userAgent;
}

long
ActiveSession::clientPid()
{
    return m_clientPid;
}


std::string
ActiveSession::domain()
{
    std::string rval = "unknown";
    bp::url::Url url;
    std::string sErr;
    if (!url.parse(URI(),sErr)) {
        BPLOG_ERROR_STRM("bad requesting domain: " << sErr);
        return rval;
    }
    
    // special case bpclient and file schemes
    if (!url.scheme().compare("bpclient")) {
        // convention is "bpclient://guid/someGoodName", use someGoodName
        std::string path = url.path();
        if (path.length() > 0 && path[0] == '/') {
            path = path.substr(1);
        }
        rval = path;
    } else if (!url.scheme().compare("file")) {
        rval = url.path();
    } else {
        rval = url.host();
    }
    return rval;
}



void
ActiveSession::displayInstallPrompt(
    weak_ptr<ICoreletExecutionContextListener> listener,
    unsigned int cookie,
    const std::vector<std::string>& permissions,
    const ServiceSynopsisList & platformUpdates,
    const ServiceSynopsisList & componentsToInstall)
{
    bp::List * platUpdates = new bp::List;
    bp::List * services = new bp::List;
    bp::List * perms = new bp::List;

    // now we must serialize the localized permissions and service 
    // descriptions that will be displayed to the end user.
    for (unsigned int i = 0; i < permissions.size(); ++i) 
    {
        perms->append(new bp::String(permissions[i]));
    }
    
    std::list<ServiceSynopsis>::const_iterator it;

    if (permissions.empty() 
        && (platformUpdates.size() == 0)
        && (componentsToInstall.size() == 0)) 
    {
        BP_THROW_FATAL("no permissions, platform updates, or services specified");
    }
    
    for (it = platformUpdates.begin();
         it != platformUpdates.end();
         it++)
    {
        bp::Map * update = new bp::Map;
        
        update->add("name", new bp::String(it->m_name));
        update->add("version", new bp::String(it->m_version));
        update->add("title", new bp::String(it->m_title));
        update->add("summary", new bp::String(it->m_summary));
        update->add("update", new bp::Bool(it->m_isUpdate));
        update->add("downloadSize", new bp::Integer(it->m_sizeInBytes));
        
        platUpdates->append(update);
    }
        
    for (it = componentsToInstall.begin();
         it != componentsToInstall.end();
         it++)
    {
        bp::Map * service = new bp::Map;

        service->add("name", new bp::String(it->m_name));
        service->add("version", new bp::String(it->m_version));
        service->add("title", new bp::String(it->m_title));
        service->add("summary", new bp::String(it->m_summary));
        service->add("update", new bp::Bool(it->m_isUpdate));
        service->add("downloadSize", new bp::Integer(it->m_sizeInBytes));

        services->append(service);
    }
    
    // now let's pass the domain doing the requesting up as well
    std::string domain;
    {
        bp::url::Url url;
        if (url.parse(URI())) {
            if (!url.scheme().compare("file")) {
                // TODO: localize
                domain = "a local file";
            } else {
                // Displayed to user - use idnaToUnicode().
                domain = bp::i18n::idnaToUnicode( url.host() );
            }
        } else {
            // TODO: localize
            domain = "unknown";
        }
    }

    bp::Map args;
    args.add("platformUpdates", platUpdates);
    args.add("permissions", perms);
    args.add("services", services);
    args.add("domain", new bp::String(domain));

    promptUser(listener, cookie,
               bp::paths::getComponentInstallDialogPath(m_locale),
               &args);
}


shared_ptr<CoreletRegistry> 
ActiveSession::registry() 
{
	return m_registry;
}


void
ActiveSession::promptUser(
    weak_ptr<ICoreletExecutionContextListener> listener,
    unsigned int cookie,
    const bp::file::Path& pathToHTMLDialog,
    const bp::Object * arguments)
{
    bp::Map m;

    m.add("path", new bp::String(pathToHTMLDialog.utf8()));
    if (arguments != NULL) m.add("arguments", arguments->clone());
    
    sendPromptUserMessage(listener, cookie, &m);
}


void 
ActiveSession::invokeCallback(unsigned int tid, const bp::Map* cbInfo)
{
    bp::ipc::Message message;
    message.setCommand("InvokeCallback");
    
    bp::Map * payload = new bp::Map;

    payload->add("callbackInfo", new bp::Map(*cbInfo));
    payload->add("tid", new bp::Integer(tid));

    message.setPayload(payload);
    if (!m_session->sendMessage(message)) {
        BPLOG_WARN_STRM("[" << m_session << "] failed to send message");
    }
    BPLOG_DEBUG_STRM("[" << m_session << "] "
                     << "invoked callback for transaction " << tid
                     << ": "
                     << payload->toPlainJsonString());

}

bool
ActiveSession::doInvoke(MessageContext* ctx)
{
    bp::ipc::Response & r = ctx->m_response;
    bp::ipc::Query & q = ctx->m_query;

    // validate the incoming request
    if (q.payload() == NULL ||
        !q.payload()->has("service", BPTString) ||
        !q.payload()->has("function", BPTString) ||        
        !q.payload()->has("version", BPTString))
    {
        populateErrorResponse(r, "BP.invalidParameters"); 
        return true;    
    }
    
    std::string service = std::string(*q.payload()->get("service"));
    std::string function = std::string(*q.payload()->get("function"));
    std::string version = std::string(*q.payload()->get("version"));    
    
    // arguments come in as a bp::Map (all args are named)
    boost::scoped_ptr<bp::Map> m;
    if (q.payload()->has("arguments", BPTMap))
    {
        m.reset((bp::Map *) q.payload()->get("arguments")->clone());
    }

    // at this point we have the corelet name, version, function, and
    // arguments to that function.

    BPLOG_DEBUG_STRM("[" << m_session << "] (" << q.id() << 
                     ") Attempting to invoke " << service << "["
                     << version << "]." << function);

    // let's verify that the function exists. 
    bp::service::Description desc;
    
    if (!m_registry->describe(service, version, std::string(), desc))
    {
        populateErrorResponse(r, "BP.noSuchCorelet");            
        return true;
    }

    bp::service::Function funcDesc;
    if (!desc.getFunction(function.c_str(), funcDesc))
    {
        populateErrorResponse(r, "BP.noSuchFunction");
        return true;
    }

    // now we've got the arguments and description, we're in a
    // position where we can validate the args.
    std::string verboseError =
        bp::service::validateArguments(funcDesc, m.get());
    
    if (!verboseError.empty())
    {
        std::string errorString = desc.name() + ".invalidArguments";
        populateErrorResponse(r, errorString, verboseError);
        
        BPLOG_WARN_STRM("[" << m_session
                        << "] error invoking "
                        << service << "[" << version << "]."
                        << function << ": " << errorString
                        << ": " << verboseError);

        return true;
    }

    version = desc.versionString();
        
    // two cases exist:
    // 1. we have already allocated an instance of this service
    // 2. we have to asynchronously allocate an instance of this service
    shared_ptr<CoreletInstance> instance =
        attainInstance(service, version);

    if (instance != NULL)
    {
        // case 1.

        // acquire the transaction id
        unsigned int tid = (unsigned int) q.id();

        // do the execution
        doExecution(instance, tid, function, m.get());

        // response at a later time
        return false;
    }
    
    // case 2.

    // now we'll attempt to allocate an instance, possible outcomes:
    // 1. an allocation is already pending, add this execution to
    //    the list 
    // 2. no allocation is pending, allocate a new one and start the
    //    list
    // 3. #2 but the allocation fails
    std::map<std::pair<std::string, std::string>,
        std::pair<unsigned int, std::vector<PendingExecution> > >::iterator i;

    std::pair<std::string,std::string> serviceVersionPair(service, version);
    i = m_pendingExecutions.find(serviceVersionPair);

    if (i == m_pendingExecutions.end()) {
        // case #2  let's try to allocate
        unsigned int allocationId =
            m_registry->instantiate(
                service, version, 
                weak_ptr<CoreletExecutionContext>(shared_from_this()),
                weak_ptr<ICoreletRegistryListener>(shared_from_this()));

        if (allocationId == 0) {
            // case #3
            std::string errorString = desc.name() + ".allocationFailure";
            populateErrorResponse(r, errorString);
            
            BPLOG_WARN_STRM("[" << m_session << "] error invoking "
                            << service << "[" << version << "]."
                            << function << ": " << errorString);

            // error response right now.
            return true;
        }

        // allocation succeeds!  let's add a pending execution table entry
        // folding case 2 into case 1.
        m_pendingExecutions[make_pair(service, version)] =
            make_pair(allocationId, vector<PendingExecution>());
        i = m_pendingExecutions.find(serviceVersionPair);
        assert(i != m_pendingExecutions.end());
    } 

    // folded case #1 and #2
    PendingExecution pe;
    pe.tid = (unsigned int) q.id();
    pe.function = function;
    pe.args = NULL;
    if (m.get()) pe.args = m.get()->clone();
    i->second.second.push_back(pe);

    // response at a later time
    return false;
}

void
ActiveSession::onAllocationSuccess(unsigned int allocationId,
                                   shared_ptr<CoreletInstance> instance)
{
    // first we'll iterate to find this allocation by id
    std::map<std::pair<std::string, std::string>,
        std::pair<unsigned int, std::vector<PendingExecution> > >::iterator i;
    for (i = m_pendingExecutions.begin(); i != m_pendingExecutions.end();i++)
    {
        if (i->second.first == allocationId) break;
    }

    if (i == m_pendingExecutions.end()) {
        BPLOG_ERROR_STRM("Successfully allocated service instance, however "
                         "no pending function executions are registered, "
                         "dropping allocation");
        return;
    }

    // cool!  first we'll set ourselves as the listener of this instance
    instance->setListener(shared_from_this());

    // now we'll squirrel this instance away
    m_instanceMap[i->first] = instance;

    // start all of the pending executions
    for (unsigned int j = 0; j < i->second.second.size(); j++) 
    {
        doExecution(instance,
                    i->second.second[j].tid,
                    i->second.second[j].function,
                    i->second.second[j].args);
        
        if (i->second.second[j].args) delete i->second.second[j].args;
    }

    // now remove the entry from pending map
    m_pendingExecutions.erase(i);
}

void
ActiveSession::doExecution(shared_ptr<CoreletInstance> instance,
                           unsigned int tid,
                           const std::string & function,
                           const bp::Object * args)
{
    bp::Null n;
    if (args == NULL) args = &n;
    instance->execute(tid, function, *args);
}

void
ActiveSession::onAllocationFailure(unsigned int allocationId)
{
    // first we'll iterate to find this allocation by id
    PendingExecutionMap::iterator i;
    for (i = m_pendingExecutions.begin(); i != m_pendingExecutions.end(); ++i)
    {
        if (i->second.first == allocationId)
            break;
    }

    if (i == m_pendingExecutions.end()) {
        BPLOG_ERROR_STRM("Service allocation failed, however "
                         "no pending function executions are registered, "
                         "ignoring");
        return;
    }

    // fail all of the pending executions
    for (unsigned int j = 0; j < i->second.second.size(); j++) 
    {
        executionFailure(i->second.second[j].tid,
                         "bp.instanceError",
                         "couldn't execute function, "
                         "service allocation failed.");
    }

    // now remove the entry from pending map
    m_pendingExecutions.erase(i);
}    

bool
ActiveSession::doRequire(MessageContext* ctx)
{
    bp::ipc::Response & r = ctx->m_response;
    bp::ipc::Query & q = ctx->m_query;
    
    // payload is Map containing "services" (a list of services)
    // and optional "progressCallback"
    if (q.payload() == NULL ||
        q.payload()->type() != BPTMap ||
        !q.payload()->has("services", BPTList))
    {
        populateErrorResponse(r, "BP.invalidParameters");    
        return true;    
    }

    std::vector<const bp::Object *> services = *(q.payload()->get("services"));
    
    std::list<CoreletRequireStatement> requires;

    for (unsigned int i = 0; i < services.size(); i++)
    {
        const bp::Object * o = services[i];

        if (!o->has("service", BPTString)) {
            BPLOG_WARN("malformed requires statement, missing 'service' key"
                       ".  skipping...");
            continue;
        }
        
        CoreletRequireStatement s;
        s.m_name = std::string(*(o->get("service")));
        if (o->has("version", BPTString)) {
            s.m_version = std::string(*(o->get("version")));
        }
        if (o->has("minversion", BPTString)) {
            s.m_minversion = std::string(*(o->get("minversion")));
        }
        requires.push_back(s);
    }
    
    if (requires.size() == 0) {
        // that was easy.
        populateSuccessResponse(r);
        return true;   
    }
    
    // got work to do
    BPCallBack progressCallback = 0;
    if (q.payload()->has("progressCallback", BPTCallBack))
    {
        progressCallback = *(q.payload()->get("progressCallback"));
    }
    unsigned int tid = q.id();

    shared_ptr<RequireRequest> req(
        new RequireRequest(requires, progressCallback,
                           shared_from_this(), tid, m_primaryDistroServer,
                           m_secondaryDistroServers));
    req->setListener(shared_from_this());

    m_requireRequests.push_back(req);
    req->run();
        
    // don't response at this point.
    return false;  
}

bool
ActiveSession::doDescribe(MessageContext* ctx)
{
    bp::ipc::Response & r = ctx->m_response;
    bp::ipc::Query & q = ctx->m_query;

    const bp::Object * pload = q.payload();
    if (pload == NULL || !pload->has("name", BPTString))
    {
        populateErrorResponse(r, "BP.invalidParameters");    
        return true;    
    }

    std::string name, version, minversion;
    
    name = std::string(*(pload->get("name")));

    if (pload->has("version", BPTString)) {
        version = std::string(*(pload->get("version")));
    }
    if (pload->has("minversion", BPTString)) {
        minversion = std::string(*(pload->get("minversion")));
    }
    
    bp::service::Description desc;
    bool haveService = m_registry->describe(name, version, minversion, desc);
    bp::Object* obj = NULL;

    if (!haveService || NULL == (obj = desc.toBPObject())) {
        populateErrorResponse(r, "BP.noSuchCorelet");    
    } else {
        populateSuccessResponse(r, obj);
        delete obj;
    }
    
    return true;
}

bool
ActiveSession::doHave(MessageContext*)
{
    BPLOG_WARN("TODO: unimplemented function!!");
    return false;
}

bool
ActiveSession::doActiveServices(MessageContext* ctx)
{
    bp::ipc::Response & r = ctx->m_response;

    r.setCommand("ActiveServices"); // neccesary?
    
    std::list<bp::service::Description> clts;
    std::list<bp::service::Description>::iterator it;

    clts = m_registry->availableCorelets();    
    bp::List corelets;

    for (it = clts.begin(); it != clts.end(); it++) {
        bp::Map* m = new bp::Map;
        m->add("name", new bp::String(it->name()));
        m->add("version", new bp::String(it->versionString()));
        m->add("doc", new bp::String(it->docString()));

        
        std::string serviceType = "unknown";

        if (it->isBuiltIn()) {
            serviceType = "built-in";
        } else {
            bp::service::Summary sum;
            if (m_registry->summary(it->name(), it->versionString(),
                                    std::string(), sum)) 
            {
                serviceType = sum.typeAsString();
            }
        }
        
        m->add("type", new bp::String(serviceType));

        corelets.append(m);
    }
    
    // populate the response
    populateSuccessResponse(r, &corelets);
    
    return true;    
}

bool
ActiveSession::onQuery(bp::ipc::Channel *,
                       const bp::ipc::Query & query,
                       bp::ipc::Response & response)
{
    try {
        // NOTE: all requests must have permission.  Require requests are 
        //       special and handle all the required permissions inside
        //       the 'RequireRequest' class.  for all other types, we
        //       verify that 'AllowDomain' permission has been granted to
        //       the session, prompting if neccesary. 

        BPLOG_DEBUG_STRM("[" << m_session << "] received query ("
                         << query.command() << "/" << query.id()
                         << "): " << query.toHuman());

        std::vector<std::string> perms;
        // return value denotes wether or not the response output parameter
        // is populated with a response, or wether we'll send a response
        // later, asynchronously.
        bool rv = false;

        // here we handle the various supported "commands".  
        // NOTE: this are only the set of supported queries which originate
        //       from the client (protocol library) and elicit a response
        if (!query.command().compare("GetState"))
        {
            perms.push_back(PermissionsManager::kAllowDomain);
            rv = dispatchMessage(&ActiveSession::doGetState, perms,
                                 m_session, query, response);
        }
        else if (!query.command().compare("ActiveServices"))
        {
            // when ActiveServices is called we'll check to see if a service
            // rescanis needed.  This would happen often during development of
            // services when services are installed via the command line
            // ServiceInstaller program.  ActiveServices is sent often by
            // the ConfigPanel program and by not much else.  scanning here
            // gaurantees that when developing with the config panel program
            // open you'll see your newly installed services quite rapidly
            if (ServicesUpdated::servicesChanged()) {
                BPLOG_INFO("Detected changed services, rescanning.");
                m_registry->forceRescan();
            }

            perms.push_back(PermissionsManager::kAllowDomain);
            rv = dispatchMessage(&ActiveSession::doActiveServices, perms,
                                 m_session, query, response);
        }
        else if (!query.command().compare("Have"))
        {
            perms.push_back(PermissionsManager::kAllowDomain);
            rv = dispatchMessage(&ActiveSession::doHave, perms,
                                 m_session, query, response);
        }
        else if (!query.command().compare("Invoke"))
        {
            perms.push_back(PermissionsManager::kAllowDomain);
            rv = dispatchMessage(&ActiveSession::doInvoke, perms,
                                 m_session, query, response);
        }
        else if (!query.command().compare("Require"))
        {
            // require handles permissions, so we send empty perms here
            rv = dispatchMessage(&ActiveSession::doRequire, perms,
                                 m_session, query, response);
        }
        else if (!query.command().compare("Describe"))
        {
            perms.push_back(PermissionsManager::kAllowDomain);
            rv = dispatchMessage(&ActiveSession::doDescribe, perms,
                                 m_session, query, response);
        }
        else if (!query.command().compare("CreateSession"))
        {
            // CreateSession should be called exactly once at startup
            if (m_createSessionCalled) {
                BPLOG_WARN("Client called createSession twice!  Closing session.");
                // close the session immediately
                m_session->disconnect();
                return false;
            }

            // when a session is created we'll check to see if a service rescan
            // is needed.  This would happen often during development of
            // services when services are installed via the command line
            // ServiceInstaller program
            if (ServicesUpdated::servicesChanged()) {
                BPLOG_INFO("Detected changed services, rescanning.");
                m_registry->forceRescan();
            }

            const bp::Object * payload = query.payload();

            if (payload != NULL && payload->type() == BPTMap) {
                std::map<std::string, const bp::Object *> pl = *payload;
                std::map<std::string, const bp::Object *>::iterator it;

                for (it = pl.begin(); it != pl.end(); it++) 
                {
                    if (!it->first.compare("uri") &&
                          it->second->type() == BPTString)
                    {
                        m_URI = normalizeClientURI(*(it->second));
                    }
                    else if (!it->first.compare("locale") &&
                             it->second->type() == BPTString)
                    {
                        m_locale = std::string(*(it->second));
                        if (m_locale.empty()) m_locale = sDefaultLocale; 
                    }
                    else if (!it->first.compare("userAgent") &&
                             it->second->type() == BPTString)
                    {
                        m_userAgent = std::string(*(it->second));
                    }
                    else if (!it->first.compare("clientPid") &&
                             it->second->type() == BPTInteger)
                    {
                        const bp::Integer* iObj =
                            dynamic_cast<const bp::Integer*>(it->second);
                        m_clientPid = (long) iObj->value();
                    }
                    else
                    {
                        BPLOG_WARN_STRM("Unrecognized key name/value type in "
                                        << " CreateSession message: "
                                        << it->first << "("
                                        << bp::typeAsString(it->second->type())
                                        << ")");
                    }
                }
            }

            // permissions checks. 
            // if we need to get new permissions, response will be sent from
            // our processEvent()
            std::string errStatus;
            PermissionsManager* pmgr = PermissionsManager::get();
            if (pmgr->upToDateCheck(this)) {
                errStatus = checkPermissions();
            } else {
                m_sessionMessage = new bp::ipc::Query(query);
                return false; // no return at this time, thanks
            }

            // Setup our response.            
            if (errStatus.compare("ok") == 0) {
                // all happy, return proto message is a map containing status
                bp::Map m;
                m.add("success", new bp::Bool(true));
                response.setPayload(m);

                BPLOG_INFO("New session successfully created: ");
                BPLOG_DEBUG_STRM("\tURI:       \t" << m_URI);
                BPLOG_INFO_STRM("\tUser Agent:\t" << m_userAgent);
                BPLOG_INFO_STRM("\tLocale:    \t" << m_locale);
                BPLOG_INFO_STRM("\tClientPid: \t" << m_clientPid);

            } else {
                populateErrorResponse(response, errStatus);
            } 

            m_createSessionCalled = true;

            return true;
        } else {
            std::string s("unrecognized protocol message: ");
            s.append(query.command());
            s.append(", terminating session");
            m_session->disconnect();
            return false;
        }

        return rv;
    } catch (const bp::error::Exception& e) {
        BP_REPORTCATCH(e);
        m_session->disconnect();
        return false;
    }
}


bool
ActiveSession::doGetState(MessageContext* ctx)
{
    bp::ipc::Response & r = ctx->m_response;
    bp::ipc::Query & q = ctx->m_query;

    const bp::Object * pload = q.payload();
    if (pload == NULL || pload->type() != BPTString)
    {
        populateErrorResponse(r, "BP.invalidParameters");    
        return true;    
    }

    std::string s = std::string(*pload);

    bp::Object * obj = SystemConfig::getState(m_locale.c_str(), s.c_str());
    assert(obj != NULL);
    populateSuccessResponse(r, obj);
    delete obj;
    
    return true;
}

bool
ActiveSession::doSetState(MessageContext* ctx)
{
    // force a response to be surpressed.  this is a fire and forget
    // message 
    ctx->m_isMessage = true;
    
    bp::ipc::Response & r = ctx->m_response;
    bp::ipc::Query & q = ctx->m_query;

    const bp::Object * pload = q.payload();
    if (pload == NULL || pload->type() != BPTMap ||
        !pload->has("state", BPTString) || !pload->has("newValue"))
    {
        populateErrorResponse(r, "BP.invalidParameters");    
        return false; // noop, no responses are sent from "messages"
    }

    std::string s = std::string(*(pload->get("state")));
    const bp::Object * arg = pload->get("newValue");
    
    bp::Object* obj = SystemConfig::setState(s.c_str(), arg);
    assert(obj != NULL);
    populateSuccessResponse(r, obj);
    delete obj;
    
    return false; // noop, no responses are sent from "messages"
}

void
ActiveSession::gotRequireLock()
{
    BPLOG_DEBUG_STRM("got require lock");
    doNextDispatch();
}

void
ActiveSession::onUserResponse(unsigned int cookie, const bp::Object & resp)
{
    // find corresponding MessageContext
    // should be message on front of queue
    if (m_messages.empty()) {
        BPLOG_WARN_STRM("got unexpected UserConfirm/DenyEvent event with empty");  
        RequireLock::releaseLock(shared_from_this());
        return;
    }

    MessageContext* ctx = m_messages.front();
    if (ctx->m_cookie != cookie) {
        BPLOG_WARN_STRM("got UserConfirm/DenyEvent event with unknown cookie: "
                        << cookie);  
        RequireLock::releaseLock(shared_from_this());
        return;
    }
    m_messages.pop_front();

    // now parse out the user response.
    std::string response;
    if (resp.type() == BPTString) {
        response = (std::string) resp;
    }
        
    bool allow = (response.find("Allow") != std::string::npos);
    bool always = (response.find("Always") != std::string::npos);
        
    // if user said "always", update domain permissions, else
    // update session's transient permissions
    if (always) {
        std::string d = domain();
        PermissionsManager* pmgr = PermissionsManager::get();
        std::vector<std::string>::const_iterator si;
        for (si = ctx->m_perms.begin(); si != ctx->m_perms.end(); ++si) {
            const std::string& perm = *si;
            if (perm.compare(PermissionsManager::kAllowDomain) == 0) {
                if (allow) {
                    pmgr->allowDomain(d);
                } else {
                    pmgr->disallowDomain(d);
                }
            } else {
                if (allow) {
                    pmgr->addDomainPermission(d, perm);
                } else {
                    pmgr->revokeDomainPermission(d, perm);
                }
            }
        }
    } else {
        std::vector<std::string>::const_iterator si;
        for (si = ctx->m_perms.begin(); si != ctx->m_perms.end(); ++si) {
            setTransientPermission(*si, allow);
        }
    }

    // no responses for message
    if (!ctx->m_isMessage) {
        if (allow) {
            if ((this->*(ctx->m_func))(ctx)) {
                ctx->sendResponse();
            }
        } else {
            populateErrorResponse(ctx->m_response, "BP.permissionsError");
            ctx->sendResponse();
        }
    }
        
    RequireLock::releaseLock(shared_from_this());

    delete ctx;
}

void
ActiveSession::onComplete(unsigned int tid,
                          const bp::List & satisfyingServices)
{
    BPLOG_DEBUG_STRM("[" << m_session << "] "
                     << " transaction " << tid << " completed");
        
    bp::ipc::Response response(tid);
    response.setCommand("Require");
        
    populateSuccessResponse(response, &satisfyingServices);        

    if (!m_session->sendResponse(response))
    {
        BPLOG_WARN_STRM("[" << m_session << "] " << "failed to send response");
    }

    // remove request
    bool erased = false;
    std::list<shared_ptr<RequireRequest> >::iterator it = m_requireRequests.begin();
    while (it != m_requireRequests.end()) {
        if ((*it)->tid() == tid) {
            it = m_requireRequests.erase(it);
            erased = true;
        } else {
            ++it;
        }
    }
    if (!erased) {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "RequireCompletedEvent received with unknown tid = "
                        << tid);
    }  
}

void
ActiveSession::onFailure(unsigned int tid,
                         const std::string & error,
                         const std::string & verboseError)
{
    BPLOG_WARN_STRM("[" << m_session << "] "
                    << "require transaction " << tid << " failed");
        
    bp::ipc::Response response(tid);
    response.setCommand("Require"); 
        
    if (!error.empty()) {
        populateErrorResponse(response, error, verboseError);
    } 
    else 
    {
        populateErrorResponse(response, "BP.requireError");
    }
        
    if (!m_session->sendResponse(response))
    {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "failed to send message");
    }
        
    // remove request
    bool erased = false;
    std::list<shared_ptr<RequireRequest> >::iterator it = m_requireRequests.begin();
    while(it != m_requireRequests.end()) {
        if ((*it)->tid() == tid) {
            it = m_requireRequests.erase(it);
            erased = true;
        } else {
            ++it;
        }
    }

    if (!erased) {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "RequireFailedEvent received with unknown tid = "
                        << tid);
    }

}

// invoked when a service function executes to completion
void
ActiveSession::executionComplete(unsigned int tid,
                                 const bp::Object & results)
{
    BPLOG_DEBUG_STRM("[" << m_session << "] "
                     << "transaction " << tid << " completed ");

    bp::ipc::Response response(tid);
    response.setCommand("Invoke");

    populateSuccessResponse(response, &results);

    if (!m_session->sendResponse(response))
    {
        BPLOG_WARN_STRM("[" << m_session << "] " << "failed to send response");
    }
}

// invoked when a service function fails
void
ActiveSession::executionFailure(
    unsigned int tid, const std::string & error,
    const std::string & verboseError)
{
    BPLOG_WARN_STRM("[" << m_session << "] "
                    << "execute transaction " << tid << " failed ");
    BPLOG_INFO_STRM("[" << m_session << ":" << tid << "] " <<
                    error << ": " << verboseError);
    
    bp::ipc::Response response(tid);
    response.setCommand("Invoke");

    std::string s = (error.empty() ? "BP.coreletExecError" : error);

    populateErrorResponse(response, s, verboseError);
        
    if (!m_session->sendResponse(response))
    {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "failed to send message");
    }
}

void
ActiveSession::gotUpToDate()
{
    assert(m_sessionMessage != NULL);

    bp::ipc::Response response(m_sessionMessage->id());
    std::string errStatus = checkPermissions();
    if (errStatus.compare("ok") != 0) {
        populateErrorResponse(response, errStatus);
    } 
    if (!m_session->sendResponse(response)) {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "failed to send response");     
    }

    delete m_sessionMessage;
    m_sessionMessage = NULL;
}


void
ActiveSession::cantGetUpToDate()
{
    assert(m_sessionMessage != NULL);

    BPLOG_WARN_STRM("[" << m_session << "] "
                    << "Unable to check for updated permissions");

    bp::ipc::Response response(m_sessionMessage->id());

    populateErrorResponse(response, "BP.permissionsError",
                          "Unable to check for updated permissions");

    if (!m_session->sendResponse(response))
    {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "failed to send response");   
    }

    delete m_sessionMessage;
    m_sessionMessage = NULL;
}



// implementation of comparator function for a map with a key of two
// ycpstrings packed into a STL pair 
bool
ActiveSession::
TwoStringCompare::operator()(
    std::pair<std::string, std::string> lhs,
    std::pair<std::string, std::string> rhs) const
{
    std::string lhsstr = lhs.first + lhs.second;
    std::string rhsstr = rhs.first + rhs.second;

    if (lhsstr.compare(rhsstr) < 0)
    {
        return true;
    }

    return false;
}

shared_ptr<CoreletInstance>
ActiveSession::attainInstance(const std::string & corelet,
                              const std::string & version)
{
    shared_ptr<CoreletInstance> instance;
    
    std::map<std::pair<std::string, std::string>,
        shared_ptr<CoreletInstance>,
        TwoStringCompare>::iterator it;

    std::pair<std::string, std::string> key(corelet, version);
    
    it = m_instanceMap.find(key);
    if (it != m_instanceMap.end())
    {
        instance = it->second;
    }
    
    return instance;
}

std::vector< std::pair<std::string, std::string> >
ActiveSession::instances()
{
    std::vector< std::pair<std::string, std::string> > rv;
    
    std::map<std::pair<std::string, std::string>,
        shared_ptr<CoreletInstance>,
        TwoStringCompare>::iterator it;

    for (it = m_instanceMap.begin(); it != m_instanceMap.end(); it++)
    {
        std::pair<std::string, std::string> p(it->first.first,
                                              it->first.second);
        rv.push_back(p);
    }

    return rv;
}


PermissionsManager::Permission 
ActiveSession::transientPermission(const std::string& perm)
{
    PermissionsManager::Permission rval = PermissionsManager::eUnknown;
    std::map<std::string, bool>::const_iterator it;
    it = m_transientPermissions.find(perm);
    if (it != m_transientPermissions.end()) {
        rval = it->second ? PermissionsManager::eAllowed : PermissionsManager::eNotAllowed;
    }
    return rval;
}


void 
ActiveSession::setTransientPermission(const std::string& perm, bool val)
{
    m_transientPermissions[perm] = val;
}

std::string
ActiveSession::checkPermissions()
{
    PermissionsManager* pmgr = PermissionsManager::get();    
    if (!pmgr->mayRun()) {
        // This is way bad, we've been told that we're evil.
        // Autoshutdown will kill us.  Client should
        // nuke us from disk when it gets this error.
        return "BP.platformBlacklisted";
    }
    
    if (m_URI.empty()) {
        BPLOG_WARN_STRM("denying access to client, missing 'uri' key in "
                        << "createSession message");
        return "BP.unapprovedDomain";
    }

    // now let's remove pathing
    bp::url::Url url;

    if (!url.parse(m_URI)) {
        BPLOG_WARN_STRM("denying access to client, cannot parse uri: " << m_URI);
        return "BP.unapprovedDomain";
    }
    
    // The "domain" used for permissions is either the hostname or the
    // path for a local url
    std::string domain = url.host().empty() ? url.path() : url.host();
    if (pmgr->domainMayUseBrowserPlus(domain) ==
        PermissionsManager::eNotAllowed)
    {
        BPLOG_WARN_STRM("denying access to client, disallowed domain: "
                        << url.toString());
        return "BP.unapprovedDomain";
    }

    BPLOG_DEBUG_STRM("Client domain approved: " << url.toString());
    
    return "ok";
}


PermissionsManager::Permission 
ActiveSession::checkDomainPermission(const std::string& permission)
{
    // does permission need approval for our domain?
    std::string d = domain();
    if (d.compare("unknown") == 0) {
        return PermissionsManager::eNotAllowed;
    }
    PermissionsManager* pmgr = PermissionsManager::get();
    std::string resolvedDomain = pmgr->normalizeDomain(d);

    PermissionsManager::Permission rval =
        pmgr->queryDomainPermission(resolvedDomain, permission);

    if (rval == PermissionsManager::eUnknown) {
        rval = transientPermission(permission);
    }

    return rval;
}

bool
ActiveSession::dispatchMessage(tHandler func, 
                               const std::vector<std::string>& perms,
                               bp::ipc::Channel * session,
                               const bp::ipc::Query & query,
                               bp::ipc::Response & response)
{
    // find out which permissions we need to prompt for
    MessageContext* ctx = NULL;
    std::vector<std::string> neededPerms;
    for (unsigned int i = 0; i < perms.size(); i++) {
        switch (checkDomainPermission(perms[i])) {
            case PermissionsManager::eAllowed:
                break;
            case PermissionsManager::eNotAllowed:
                populateErrorResponse(response, "BP.permissionsError");    
                return true; 
            case PermissionsManager::eUnknown:
                neededPerms.push_back(perms[i]);
                break;
        }
    }
    
    bool rval = false;
    if (neededPerms.empty()) {
        // nothing needed, process message
        ctx = new MessageContext(func, session, query, response);
        rval = (this->*func)(ctx);
        // in this case we must alter the response message.  because
        // we've copied into messagecontext, we must copy back out
        // to our in/out parameter
        response = ctx->m_response;
        delete ctx;
    } else {
        // gotta get user permission.  push onto queue and attain
        // same lock used by RequireRequest.  Will get 
        // RequireLock::LockAttainedEvent when it's our turn.
        // This prevents us from double-prompting for permissions.
        ctx = new MessageContext(func, session, query, response);
        ctx->m_perms = neededPerms;
        m_messages.push_back(ctx);
        BPLOG_DEBUG_STRM("ActiveSession::dispatchMessage asks for lock");

        m_thisWeak = shared_from_this();
        RequireLock::attainLock(m_thisWeak);
        rval = false;
    }
    
    return rval;
}

void 
ActiveSession::doNextDispatch()
{
    if (m_messages.empty()) {
        RequireLock::releaseLock(shared_from_this());
        return;
    }
    MessageContext* ctx = m_messages.front();

    // do we still need to prompt?
    std::vector<std::string>::iterator it = ctx->m_perms.begin();
    while (it != ctx->m_perms.end()) {
        std::string p = *it;
        switch (checkDomainPermission(p)) {
            case PermissionsManager::eAllowed:
                it = ctx->m_perms.erase(it);
                break;
            case PermissionsManager::eNotAllowed: {
                m_messages.pop_front();
                populateErrorResponse(ctx->m_response, "BP.permissionsError"); 
                if (!ctx->m_isMessage) ctx->sendResponse();
                RequireLock::releaseLock(shared_from_this());
                delete ctx;
                return; 
            }
            case PermissionsManager::eUnknown:
                ++it;
                break;
        }
    }
    
    // yay, all of our perms have been approved by another prompt
    if (ctx->m_perms.empty()) {
        if ((this->*(ctx->m_func))(ctx) && !ctx->m_isMessage) {
            ctx->sendResponse();
        }
        RequireLock::releaseLock(shared_from_this());
        m_messages.pop_front();
        delete ctx;
        return; 
    }
    
    // still gotta prompt
    ServiceSynopsisList emptyList;
    ctx->m_cookie = rand();
    PermissionsManager* pmgr = PermissionsManager::get();
    std::vector<std::string> desc;
    for (unsigned int i = 0; i < ctx->m_perms.size(); i++) {
        std::string s = pmgr->getLocalizedPermission(ctx->m_perms[i],
                                                     m_locale);
        desc.push_back(s);
    }

    displayInstallPrompt(shared_from_this(),
                         ctx->m_cookie,
                         desc,
                         emptyList,
                         emptyList);
}


unsigned int
ActiveSession::sendPromptUserMessage(
    weak_ptr<ICoreletExecutionContextListener> listener,
    unsigned int cookie,
    const bp::Object * o)
{
    unsigned int tid = 0;

    bp::ipc::Query q;
    tid = q.id();
    q.setCommand("PromptUser");
    q.setPayload(*o);

    if (!m_session->sendQuery(q)) {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "failed to send prompt user message");   
    } else {
        // add this tid and listener to map
        m_promptRequests[tid] =
            std::pair<weak_ptr<ICoreletExecutionContextListener>,
                      unsigned int>(listener, cookie);

        BPLOG_WARN_STRM("[" << m_session << "] " << "sent user prompt");
    }

    return tid;
}


void
ActiveSession::handlePromptUserResponse(const bp::ipc::Response & resp)
{
    unsigned int sid = resp.responseTo();

    std::map<unsigned int,
        std::pair<weak_ptr<ICoreletExecutionContextListener>,
                  unsigned int> >::iterator it;

    it = m_promptRequests.find(sid);
    if (it == m_promptRequests.end()) {
        BPLOG_WARN_STRM("[" << m_session << "] "
                        << "got prompt user response, cannot find request. "
                        << "dropping..." << std::endl
                        << resp.serialize());
        return;
    }

    bp::Null n;
    const bp::Object * userResp = &n;
    if (resp.payload() != NULL) userResp = resp.payload();

    shared_ptr<ICoreletExecutionContextListener>
        ptr = it->second.first.lock();
    if (ptr) ptr->onUserResponse(it->second.second, *userResp);    

    m_promptRequests.erase(it);
}
            
void
ActiveSession::channelEnded(
    bp::ipc::Channel * c,
    bp::ipc::IConnectionListener::TerminationReason why,
    const char * errorString)
{
    std::stringstream ss;
    ss << "Session ended ("
       << bp::ipc::IConnectionListener::terminationReasonToString(why)
       << ")";
    if (errorString) ss << ": " << errorString;

    BPLOG_INFO(ss.str());    

    if (m_listener) m_listener->onSessionEnd(c);
    
}

void
ActiveSession::onMessage(bp::ipc::Channel *,
                         const bp::ipc::Message & m)
{
    BPLOG_DEBUG_STRM("[" << m_session << "] received message ("
                     << m.command() << "): " << m.toHuman());

    if (!m.command().compare("SetState"))
    {
        std::vector<std::string> perms;
        // bogus, reusing query/response logic (around permissions)
        bp::ipc::Query q;
        q.setCommand(m.command());
        if (m.payload()) q.setPayload(*m.payload());
        bp::ipc::Response r(0);
        perms.push_back(PermissionsManager::kAllowDomain);
        (void) dispatchMessage(&ActiveSession::doSetState, perms,
                               m_session, q, r);
    }
    else
    {
        BPLOG_WARN_STRM("Unexpected message received: " << m.command());
    }
}

void
ActiveSession::onResponse(bp::ipc::Channel *,
                          const bp::ipc::Response & response)
{
    BPLOG_WARN_STRM("[" << m_session << "] received response ("
                    << response.command() << "/" << response.responseTo()
                    << "): " << response.toHuman());


    if (!response.command().compare("PromptUser")) {
        handlePromptUserResponse(response);
    } else {
        BPLOG_WARN_STRM("Unexpected response received: "
                        << response.command());        
    }
}


std::string
ActiveSession::normalizeClientURI(const std::string& uri)
{
    // Passthrough non-http urls (e.g. pref-panel).
    if (!bp::url::isHttpOrHttpsUrl(uri))
        return uri;
    
    // Uri encoding is client-dependent.
    // IE7 sends UTF-8, XP FF sends IDNA ascii.
    // We use IDNA ascii internally.
    // We'll do a toAscii operation to ensure we have IDNA ascii encoding.
    // Note: Redundant IDNA tranformations have no effect (so FF case is ok).
    // Note: IDNA algo doesn't take kindly to scheme in url, so have to parse.
    bp::url::Url url(uri);
    std::string sHost_uni = bp::i18n::idnaToAscii(url.host());
    url.setHost(sHost_uni);
    
    return url.toString();
}
                     

