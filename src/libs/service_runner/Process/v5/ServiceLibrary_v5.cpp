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

#include "ServiceLibrary_v5.h"
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

// BEGIN call-in points for dynamically loaded services

/**
 * a structure holding data from instance runloop back to 
 * main runloop
 */
struct InstanceResponse
{
    enum { T_Results, T_Error, T_CallBack, T_Prompt, T_MainThreadCallback } type;

    // common
    unsigned int tid;

    // threadTransfer
    BPCMainThreadCallbackPtr mainThreadCallback;

    // results
    bp::Object * o;

    // error
    std::string error;
    std::string verboseError;    

    // callback
    long long int callbackId;

    // prompt
	bp::file::Path dialogPath;
    BPUserResponseCallbackFuncPtr responseCallback;
    void * responseCookie;
    unsigned int promptId;

    InstanceResponse()
        : type(T_Results), tid(0), o(NULL), callbackId(0),
          dialogPath(), responseCallback(NULL),
          responseCookie(NULL), promptId(0)  {  }
    ~InstanceResponse() { if (o) delete o; }
};

static ServiceLibrary_v5 * s_libObjectPtr = NULL;
    
void
ServiceLibrary_v5::postResultsFunction(unsigned int tid,
                                       const struct BPElement_t * results)
{
    InstanceResponse * ir = new InstanceResponse;
    ir->type = InstanceResponse::T_Results;
    ir->tid = tid;
    ir->o = (results ? bp::Object::build(results) : NULL);
    s_libObjectPtr->hop(ir);
}

void
ServiceLibrary_v5::postErrorFunction(unsigned int tid,
                                     const char * error,
                                     const char * verboseError)
{
    InstanceResponse * ir = new InstanceResponse;
    ir->type = InstanceResponse::T_Error;
    ir->tid = tid;
    if (error) ir->error.append(error);
    if (verboseError) ir->verboseError.append(verboseError);    
    s_libObjectPtr->hop(ir);
}

void
ServiceLibrary_v5::logFunction(unsigned int level, const char * fmt, ...)
{
// copying va_args.
#ifndef va_copy
# ifdef __va_copy
#  define va_copy(DEST,SRC) __va_copy((DEST),(SRC))
# else
#  define va_copy(DEST,SRC) ((DEST) = (SRC))
# endif
#endif
    
    va_list ap;
    va_start(ap, fmt);
    
    // how big a string do we need?
    char* buf = NULL;
    unsigned int sz = 0;
    va_list test;
    va_copy(test, ap);
#ifdef WIN32
    // ok to disable warning, this call doesn't actually write any data
#pragma warning(push)
#pragma warning(disable:4996)
#endif
    sz = vsnprintf(buf, 0, fmt, test);
#ifdef WIN32
#pragma warning(pop)
#endif
    va_end(test);

    // limit size of log entry.  no evil services
    // trying to get us to malloc a gigabyte!
    if (sz > 4095) {
        BPLOG_WARN_STRM("log entry reduced from " << sz << " to 4096 bytes");
        sz = 4095;  // a sz++ will be done below, yielding max of 4k
    }
    
    if (sz > 0) {
        sz++;
        buf = new char[sz];
        if (!buf) {
            BPLOG_ERROR("unable to allocate char buffer");
            return;
        }
#ifdef WIN32
        sz = vsnprintf_s(buf, sz, _TRUNCATE, fmt, ap);
#else
        sz = vsnprintf(buf, sz, fmt, ap);
#endif
    }

    // Handle buf=NULL.
    std::string str = bp::strutil::safeStr(buf);

    s_libObjectPtr->logServiceEvent(level, str);
    
    delete[] buf;
    va_end(ap);
}


static bp::log::Level BPLevelFromServiceLevel(unsigned int level)
{
    switch (level)
    {
        case BP_DEBUG:  return bp::log::LEVEL_DEBUG;
        case BP_INFO:   return bp::log::LEVEL_INFO;
        case BP_WARN:   return bp::log::LEVEL_WARN;
        case BP_ERROR:  return bp::log::LEVEL_ERROR;
        case BP_FATAL:  return bp::log::LEVEL_FATAL;
        default:        return bp::log::LEVEL_ALL;
    }
}


///////////////////////////////////////////////////////////////////////////
// Setup logging done by the service proper.  That is, logging done by
// the actual service dll through the service api, as oppposed to logging done
// by the "harness" exe.  Note this also distinguishes code written by
// service authors from platform code written by the browserplus team.
// 
// If the serviceLogMode setting is "combined" (default) this method
// will take no action and logging from the harness and the dll will
// both go to the destination setup by Process.cpp: setupLogging().
// 
// If the serviceLogMode setting is "separate", this method will setup
// a separate destination for logging from the dll.
//
void
ServiceLibrary_v5::setupServiceLogging()
{
    bp::log::Configurator cfg;
    cfg.loadConfigFile();

    // If "combined" mode, nothing for us to do.
    m_serviceLogMode = cfg.getServiceLogMode();
    if (m_serviceLogMode == bp::log::kServiceLogCombined) {
        return;
    }

    bp::log::Destination dest = cfg.getDestination();
    if (dest == bp::log::kDestFile) {
        string fileName = name().empty() ? "service" : name();
        bpf::Path p(fileName);
        p.replace_extension(bpf::nativeFromUtf8("log"));
        cfg.setPath(p);
    }
	else if (dest == bp::log::kDestConsole) {
        string nameVer = description().nameVersionString();
        string consoleTitle = nameVer.empty() ? "BrowserPlus Service"
            : nameVer + " Service";
        cfg.setConsoleTitle(consoleTitle);
    }
    else {
    }
    
    // Configure the logging system.
    cfg.configure(m_serviceLogger);
}


//////////////////
// Log events from the dll over the Service API come in here.
void
ServiceLibrary_v5::logServiceEvent(unsigned int level, const std::string& msg)
{
    static bool s_firstTime = true;

    if (s_firstTime) {
        setupServiceLogging();
        s_firstTime = false;
    }

    // Convert the level.
    bp::log::Level bpLevel = BPLevelFromServiceLevel(level);
    
    if (m_serviceLogMode == bp::log::kServiceLogSeparate)
    {
        // Send event to "separate" destination.
        BPLOG_LEVEL_LOGGER(bpLevel, m_serviceLogger, msg);
    }
    else
    {
        // Prepend service name/version to the log message, if we know it.
        string nameVer = description().nameVersionString();
        string logMsg = nameVer.length() ? string("(") + nameVer + ") " + msg
            : msg;

        // Send event to "combined" destination.
        BPLOG_LEVEL(bpLevel, logMsg);
    }
}


void
ServiceLibrary_v5::invokeCallbackFunction(unsigned int tid,
                                          long long int callbackHandle,
                                          const struct BPElement_t * results)
{
    InstanceResponse * ir = new InstanceResponse;
    ir->type = InstanceResponse::T_CallBack;
    ir->tid = tid;
    ir->callbackId = callbackHandle;
    ir->o = (results ? bp::Object::build(results) : NULL);
    s_libObjectPtr->hop(ir);
}

void
ServiceLibrary_v5::invokeOnMainThreadFunction(BPCMainThreadCallbackPtr cb)
{
    InstanceResponse * ir = new InstanceResponse;
    ir->type = InstanceResponse::T_MainThreadCallback;
    ir->mainThreadCallback = cb;
    s_libObjectPtr->hop(ir);
}

unsigned int
ServiceLibrary_v5::promptUserFunction(
    unsigned int tid,
    const BPPath pathToHTMLDialog,
    const BPElement * args,
    BPUserResponseCallbackFuncPtr responseCallback,
    void * cookie)
{
    // management of unique prompt ids
    static unsigned int s_promptId = 1000;
    static bp::sync::Mutex s_lock;

    // allocate a new unique prompt id
    unsigned int cpid;
    {
        bp::sync::Lock _lock(s_lock);
        cpid = s_promptId++;
    }

    InstanceResponse * ir = new InstanceResponse;
    ir->type = InstanceResponse::T_Prompt;
    ir->tid = tid;
    if (pathToHTMLDialog) ir->dialogPath /= pathToHTMLDialog;
    ir->o = (args ? bp::Object::build(args) : NULL);
    ir->responseCallback = responseCallback;
    ir->responseCookie = cookie;
    ir->promptId = cpid;

    s_libObjectPtr->hop(ir);

    return cpid;
}
// END call-in points for dynamically loaded services

// the static callback function table
const void *
ServiceLibrary_v5::getFunctionTable()
{
    static BPCFunctionTable * table = NULL;
    static BPCFunctionTable tableData;
    if (table != NULL) return table;
    memset((void *) &tableData, 0, sizeof(tableData));
    tableData.postResults = postResultsFunction;
    tableData.postError = postErrorFunction;
    tableData.log = logFunction;
    tableData.invoke = invokeCallbackFunction;
    tableData.prompt = promptUserFunction;
    tableData.invokeOnMainThread = invokeOnMainThreadFunction;

    table = &tableData;
    return (void *) table;
}

ServiceLibrary_v5::ServiceLibrary_v5() :
    m_currentId(1), m_handle(NULL), m_funcTable(NULL),
    m_desc(), m_serviceAPIVersion(0), m_instances(), m_listener(NULL),
    m_promptToTransaction(), 
    m_serviceLogMode( bp::log::kServiceLogCombined ), m_serviceLogger()
{
    s_libObjectPtr = this;
}
        
ServiceLibrary_v5::~ServiceLibrary_v5()
{
    const BPPFunctionTable * funcTable = (const BPPFunctionTable *) m_funcTable;

    // deallocate all instances
    while (m_instances.size() > 0)
    {
        std::map<unsigned int, void *>::iterator it;
        it = m_instances.begin();
        if (funcTable->destroyFunc != NULL)
        {
            funcTable->destroyFunc(it->second);
        }
        m_instances.erase(it);
    }

    // shutdown the library
    shutdownService(true);

    s_libObjectPtr = NULL;
}

std::string
ServiceLibrary_v5::version()
{
    return m_desc.versionString();
}

std::string
ServiceLibrary_v5::name()
{
    return m_desc.name();
}

// load the service
bool
ServiceLibrary_v5::load(const bp::service::Summary &summary,
                        const bp::service::Summary &provider,
                        void * functionTable)
{
    const BPPFunctionTable * funcTable = NULL;
     
    bool success = true;
    // meaningful when success == false;
    bool callShutdown = true;
    
    BPASSERT(m_handle == NULL);
    BPASSERT(m_funcTable == NULL);    

    m_summary = summary;
    m_handle = functionTable;
    m_funcTable = functionTable;

    // now let's determine the path to the shared library.  For
    // dependent services this will be extracted from the manifest
    bpf::Path servicePath;
    bpf::Path dependentPath;
    const BPElement * dependentParams = NULL;    
    bp::Map dependentParamsStorage;
    
    if (m_summary.type() == bp::service::Summary::Dependent)
    {
        dependentPath = m_summary.path();
        servicePath = provider.path();

        // now populate dependent params
        std::map<std::string, std::string> summaryArgs = m_summary.arguments();
        std::map<std::string, std::string>::iterator it;
        for (it = summaryArgs.begin(); it != summaryArgs.end(); it++)
        {
            dependentParamsStorage.add(it->first.c_str(), new bp::String(it->second));
        }
        dependentParams = dependentParamsStorage.elemPtr();
    }
    else
    {
        // leave dependent path empty()
        servicePath = m_summary.path();
    }
    servicePath = bpf::canonicalPath(servicePath);
    dependentPath = bpf::canonicalPath(dependentPath);
    
    funcTable = (const BPPFunctionTable *) m_funcTable;

    if (funcTable == NULL || funcTable->initializeFunc == NULL)
    {
        BPLOG_WARN_STRM("invalid service, NULL initialize function ("
                        << m_summary.name() << " | " << m_summary.version()<< ")");
        success = false;
        // still call shutdown
    }
    else
    {
        m_serviceAPIVersion = funcTable->serviceAPIVersion;

        BPASSERT(m_serviceAPIVersion == 5);
            
        const BPServiceDefinition * def = NULL;
        
        def = funcTable->initializeFunc(
            (const BPCFunctionTable *) getFunctionTable(),
            (const BPPath) (servicePath.external_file_string().c_str()),
            (const BPPath) (dependentPath.empty() ? NULL : dependentPath.external_file_string().c_str()),
            dependentParams);
            
        if (def == NULL)
        {
            BPLOG_WARN_STRM("invalid service, NULL return from "
                            << "initialize function ("
                            << m_summary.name() << " | " << m_summary.version()<< ")");
            success = false;
            // don't call shutdown.  This service has already
            // violated a contract
            callShutdown = false;
        }
        else if (!m_desc.fromBPServiceDefinition(def))
        {
            BPLOG_WARN_STRM("couldn't populate Description "
                            "from returned service structure ");
            success = false;
            callShutdown = true;
        }
    }

    if (!success) shutdownService(callShutdown);

    return success;
}

// shutdown the service, NULL out m_handle, and m_def
void
ServiceLibrary_v5::shutdownService(bool callShutdown)
{
    const BPPFunctionTable * table = (const BPPFunctionTable *) m_funcTable;
    
    if (NULL != m_handle)
    {
        // first call shutdown
        if (callShutdown && table != NULL && table->shutdownFunc != NULL) {
            table->shutdownFunc();
        }

        BPLOG_INFO_STRM("unloading service library: "
                        << m_summary.serviceLibraryPath());

        m_handle = NULL;
    }
    m_funcTable = NULL;
}

unsigned int
ServiceLibrary_v5::allocate(std::string uri, bpf::Path dataDir,
                            bpf::Path tempDir, std::string locale,
                            std::string userAgent, unsigned int clientPid)
{
    unsigned int id = m_currentId++;

    // client popupulated context ptr
    void * cookie = NULL;

    const BPPFunctionTable * funcTable =
        (const BPPFunctionTable *) m_funcTable;    

    if (funcTable->allocateFunc != NULL)
    {
        bpf::Path serviceDir = m_summary.path();
        
        int rv = funcTable->allocateFunc(
            &cookie,
            (const BPString) uri.c_str(),
            (const BPPath) serviceDir.external_file_string().c_str(),
            (const BPPath) dataDir.external_file_string().c_str(),
            (const BPPath) tempDir.external_file_string().c_str(),
            (const BPString) locale.c_str(),
            (const BPString) userAgent.c_str(),
            clientPid);

        if (rv != 0) {
            BPLOG_ERROR_STRM(
                "Failed to allocate instance of "
                << name() << " (" << version()
                << ") - BPPAllocate returns non-zero code: "
                << rv);

            return 0;
        }

    }

    // add the instance
    m_instances[id] = cookie;

    return id;
}

void
ServiceLibrary_v5::destroy(unsigned int id)
{
    const BPPFunctionTable * funcTable = (const BPPFunctionTable *) m_funcTable;

    std::map<unsigned int, void *>::iterator it;

    it = m_instances.find(id);

    if (it != m_instances.end())
    {
        if (funcTable->destroyFunc != NULL)
        {
            funcTable->destroyFunc(it->second);
        }
        m_instances.erase(it);
    }
}

bool
ServiceLibrary_v5::invoke(unsigned int id, unsigned int tid,
                          const std::string & function,
                          const bp::Object * arguments,
                          std::string & err)
{
    // first we'll add the transaction to the transaction map
    // (removed in postError or postResults functions)
    beginTransaction(tid, id);

    // argument validation. does function exist?  are parameters
    // correct?
    bp::service::Function funcDesc;
    if (!m_desc.getFunction(function.c_str(), funcDesc))
    {
        std::stringstream ss;
        ss << "no such function: " << function;
        err = ss.str();
        postErrorFunction(tid, "bp.noSuchFunction", err.c_str());
        return true;
    }

    if (arguments && arguments->type() != BPTMap) {
        err.append("arguments must be a map");
        postErrorFunction(tid, "bp.invokeError", err.c_str());
        return true;
    }

    // now we've got the arguments and description, we're in a
    // position where we can validate the args.
    err = bp::service::validateArguments(funcDesc, (bp::Map *) arguments);
    
    if (!err.empty()) {
        postErrorFunction(tid, "bp.invokeError", err.c_str());
        return true;
    }
    
    // finally, does the specified instance exist?
    std::map<unsigned int, void *>::iterator it;
    it = m_instances.find(id);

    if (it == m_instances.end()) {
        std::stringstream ss;
        ss << "no such instance: " << id;
        err = ss.str();
        postErrorFunction(tid, "bp.invokeError", err.c_str());
        return true;
    }
    
    // good to go!  now we're ready to actually invoke the function!
    const BPPFunctionTable * funcTable = (const BPPFunctionTable *) m_funcTable;

    if (funcTable->invokeFunc != NULL)
    {
        funcTable->invokeFunc(it->second,
                              function.c_str(),
                              tid,
                              ((arguments && arguments->type() == BPTMap) ? arguments->elemPtr() : NULL));
    }

    return true;
}

void
ServiceLibrary_v5::promptResponse(unsigned int promptId,
                                  const bp::Object * arguments)
{
    PromptContext ctx;
    unsigned int instance;
    std::map<unsigned int, void *>::iterator it;
    
    if (!findContextFromPromptId(promptId, ctx)) {
        BPLOG_ERROR_STRM("prompt response with unknown prompt ID: "
                         << promptId);
    } else if (!findInstanceFromTransactionId(ctx.tid, instance)) {
        BPLOG_ERROR_STRM("prompt response associated with unknown transaction "
                         "id: " << ctx.tid);
    } else if ((it = m_instances.find(instance)) == m_instances.end()) {
        BPLOG_ERROR_STRM("prompt response associated with unknown instance "
                         "id: " << instance);
    } else if (ctx.cb) {
        // invoke the client callback
        ctx.cb(ctx.cookie, promptId, (arguments ? arguments->elemPtr() : NULL));
    }

    // regardless of the outcome, let's ensure we've removed our
    // record of the prompt
    (void) endPrompt(promptId);
}

void
ServiceLibrary_v5::onHop(void * context)
{
    InstanceResponse * ir = (InstanceResponse *) context;
    BPASSERT(ir != NULL);

    // all InstanceResponses have a populated tid, all need an
    // instance id to go with it.
    unsigned int instance;
        
    if (!findInstanceFromTransactionId(ir->tid, instance)) {
        BPLOG_ERROR_STRM("function call from service with bogus tid: "
                         << ir->tid << " - not associated with any instance");
        if (ir) delete ir;        
        return;
    }
    
    if (m_listener) {    
        switch (ir->type) {
            case InstanceResponse::T_Results: {
                m_listener->onResults(instance, ir->tid, ir->o);

                // remove the transaction
                endTransaction(ir->tid);
                break;
            }
            case InstanceResponse::T_Error: {
                m_listener->onError(instance, ir->tid, ir->error,
                                    ir->verboseError);

                // remove the transaction
                endTransaction(ir->tid);
                break;
            }
            case InstanceResponse::T_CallBack: {
                m_listener->onCallback(instance, ir->tid,
                                       ir->callbackId, ir->o);
                break;
            }
            case InstanceResponse::T_MainThreadCallback: {
                if (ir->mainThreadCallback) ir->mainThreadCallback();
                break;
            }
            case InstanceResponse::T_Prompt: {
                if (!transactionKnown(ir->tid)) {
                    BPLOG_ERROR_STRM("can't send prompt from, unknown "
                                     "transaction: " << ir->tid);
                } else {
                    // squirrel away what we need to correctly associate
                    // the response with the correct instance
                    beginPrompt(ir->promptId, ir->tid, ir->responseCallback,
                                ir->responseCookie);
                    m_listener->onPrompt(instance, ir->promptId,
                                         bpf::Path(ir->dialogPath), 
                                         ir->o);
                }
                break;
            }
        }
    }
    
    if (ir) delete ir;
}

void
ServiceLibrary_v5::setListener(IServiceLibraryListener * listener)
{
    m_listener = listener;
}
    
bool
ServiceLibrary_v5::transactionKnown(unsigned int tid)
{
    std::map<unsigned int, unsigned int>::iterator i;
    i = m_transactionToInstance.find(tid);
    return (i != m_transactionToInstance.end());
}


void
ServiceLibrary_v5::beginTransaction(unsigned int tid, unsigned int instance)
{
    if (transactionKnown(tid)) {
        BPLOG_ERROR_STRM("duplicate transaction id detected: " << tid);
    } else {
        m_transactionToInstance[tid] = instance;
    }
}

void
ServiceLibrary_v5::endTransaction(unsigned int tid)
{
    std::map<unsigned int, unsigned int>::iterator i;
    i = m_transactionToInstance.find(tid);
    if (i == m_transactionToInstance.end()) {
        BPLOG_ERROR_STRM("attempt to end unknown transaction: " << tid);
    } else {
        m_transactionToInstance.erase(i);
    }

    // as an added bonus, let's purge record of all prompts associated
    // with this completed transaction
    std::map<unsigned int, PromptContext>::iterator it;
    unsigned int numPurged = 0;
    for (it = m_promptToTransaction.begin();
         it != m_promptToTransaction.end();
         it++)
    {
        if (it->second.tid == tid) {
            numPurged++;
            // erase and restart iteration
            m_promptToTransaction.erase(it);
            it = m_promptToTransaction.begin();
        }
    }

    if (numPurged > 0) {
        BPLOG_INFO_STRM("Transaction ends (" << tid << ") with "
                        << numPurged << " outstanding user prompts");        
    }
}

bool
ServiceLibrary_v5::findInstanceFromTransactionId(unsigned int tid,
                                                 unsigned int & instance)
{
    instance = 0;
    
    std::map<unsigned int, unsigned int>::iterator i;
    i = m_transactionToInstance.find(tid);
    if (i == m_transactionToInstance.end()) return false;
    instance = i->second;
    return true;
}

bool
ServiceLibrary_v5::promptKnown(unsigned int promptId)
{
    std::map<unsigned int, PromptContext>::iterator i;
    i = m_promptToTransaction.find(promptId);
    return (i != m_promptToTransaction.end());
}

void
ServiceLibrary_v5::beginPrompt(unsigned int promptId, unsigned int tid,
                               BPUserResponseCallbackFuncPtr cb, void * cookie)
{
    if (promptKnown(promptId)) {
        BPLOG_ERROR_STRM("duplicate prompt id detected: " << promptId);
    } else {
        PromptContext ctx;
        ctx.tid = tid;
        ctx.cb = cb;
        ctx.cookie = cookie;
        m_promptToTransaction[promptId] = ctx;
    }
}

void
ServiceLibrary_v5::endPrompt(unsigned int promptId)
{
    std::map<unsigned int, PromptContext>::iterator i;
    i = m_promptToTransaction.find(promptId);
    if (i == m_promptToTransaction.end()) {
        BPLOG_ERROR_STRM("attempt to end unknown prompt: " << promptId);
    } else {
        m_promptToTransaction.erase(i);
    }
}

bool
ServiceLibrary_v5::findContextFromPromptId(unsigned int promptId,
                                           PromptContext & ctx)
{
    std::map<unsigned int, PromptContext>::iterator i;
    i = m_promptToTransaction.find(promptId);
    if (i == m_promptToTransaction.end()) return false;
    ctx = i->second;
    return true;
}

unsigned int
ServiceLibrary_v5::apiVersion()
{
    return m_serviceAPIVersion;
}
