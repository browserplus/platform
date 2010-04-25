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

/*
 * An abstraction around the dynamic library that composes a service.
 * abstracts dlloading and all interaction.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/15
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __SERVICELIBRARY_H__
#define __SERVICELIBRARY_H__

#include <string>
#include "BPUtils/BPLogLevel.h"
#include "BPUtils/BPLogLogger.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bprunloopthread.h"
#include "BPUtils/bpthreadhopper.h"
#include "BPUtils/bptr1.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/LogConfigurator.h"
#include "BPUtils/ServiceDescription.h"
#include "BPUtils/ServiceSummary.h"
#include "ServiceAPI/bppfunctions.h"

namespace ServiceRunner 
{
    class IServiceLibraryListener 
    {
    public:
        virtual void onResults(unsigned int instance,
                               unsigned int tid, const bp::Object * o) = 0;
        virtual void onCallback(unsigned int instance,
                                unsigned int tid,
                                long long int callbackId,
                                const bp::Object * value) = 0;
        virtual void onError(unsigned int instance,
                             unsigned int tid,
                             const std::string & error,
                             const std::string & verboseError) = 0;

        virtual void onPrompt(unsigned int instance,
                              unsigned int promptId,
                              const bp::file::Path & pathToDialog,
                              const bp::Object * arguments) = 0;

        virtual ~IServiceLibraryListener() { }
    };

    class ServiceLibrary : public bp::thread::HoppingClass
    {
      public:
        ServiceLibrary();
        ~ServiceLibrary();

        void setListener(IServiceLibraryListener * listener);
        
        // parse the manifest from the cwd, populating name and version
        bool parseManifest(std::string & oError);

        // load the service
        // when service type is dependent, providerPath must be supplied,
        // and must be a valid path to a service that satisfies the
        // dependent's requirements.
        bool load(const bp::file::Path & providerPath,
                  std::string & oError);

        unsigned int apiVersion();
        
        std::string name();
        std::string version();
        const bp::service::Description & description() { return m_desc; }

        unsigned int allocate(const bp::Map & context);
        void destroy(unsigned int id);

        bool invoke(unsigned int id, unsigned int tid,
                    const std::string & function,
                    const bp::Object * arguments,
                    std::string & err);

        void promptResponse(unsigned int promptId,
                            const bp::Object * arguments);

      private:
        // current instance id.  we start counting at 1
        unsigned int m_currentId;

        // zero for standalone or providers, 1000 for dependents.  Because
        // now depndents always run in their own process space, this is
        // a bit of an unneccesary artifact that should be eliminated after
        // we rev and clean up the service API
        unsigned int m_attachId;

        bp::service::Summary m_summary;

        // curently allocated corelet
        void * m_handle;

        // pointer to the corelet instances function table
        const void * m_funcTable;

        // corelet description
        bp::service::Description m_desc;

        // the corelet api version, populated during load
        unsigned int m_coreletAPIVersion;

        // get the function table that we'll pass to corelets
        static const void * getFunctionTable();

        // shutdown the service and unload the library
        void shutdownCorelet(bool callShutdown);
    
        // All NP functions must be implemented once per platform.
        /**
         * open a shared library returning an opaque handle or NULL
         *  on failure
         */
        static void * dlopenNP(const bp::file::Path & path);
        /**
         * close and free a handle to a dynamic library
         */
        static void dlcloseNP(void * handle);
        /**
         * acquire a pointer to a symbol from a shared library, or NULL if
         * the symbol can not be found.
         */
        static void * dlsymNP(void * handle, const char * sym);

        // a map mapping instances to RunLoop thread handles.
        std::map<unsigned int,
                 std::tr1::shared_ptr<bp::runloop::RunLoopThread> > m_instances;

        // how instance threads call back into the main thread.
        void onHop(void * context);

        // entry points for services
        static void postResultsFunction(unsigned int tid,
                                        const struct BPElement_t * results);

        static void postErrorFunction(unsigned int tid,
                                      const char * error,
                                      const char * verboseError);

        static void logFunction(unsigned int level, const char * fmt, ...);
        
        static void invokeCallbackFunction(unsigned int tid,
                                           long long int callbackHandle,
                                           const struct BPElement_t * results);
        static unsigned int promptUserFunction(
            unsigned int tid,
            const char * utf8PathToHTMLDialog,
            const BPElement * args,
            BPUserResponseCallbackFuncPtr responseCallback,
            void * cookie);

        IServiceLibraryListener * m_listener;

        // transaction bookkeeping required for a couple reasons:
        // 1. mapping promptId to instance for user prompt responses.
        // 2. determining instance id for callbacks
        // 3. determining instance id for sending user prompts
        
        // mapping transaction ids to instance ids
        std::map<unsigned int, unsigned int> m_transactionToInstance;
        void beginTransaction(unsigned int tid, unsigned int instance);
        bool transactionKnown(unsigned int tid);
        void endTransaction(unsigned int tid);
        bool findInstanceFromTransactionId(unsigned int tid,
                                           unsigned int & instance);

        // mapping prompt ids to transaction ids & context
        // a table mapping prompt ids to callback and cookie
        struct PromptContext {
            unsigned int tid;
            BPUserResponseCallbackFuncPtr cb;
            void * cookie;
        };

        std::map<unsigned int, PromptContext> m_promptToTransaction;
        void beginPrompt(unsigned int promptId,
                         unsigned int tid,
                         BPUserResponseCallbackFuncPtr cb,
                         void * cookie);
        void endPrompt(unsigned int promptId);
        bool promptKnown(unsigned int promptId);
        bool findContextFromPromptId(unsigned int promptId,
                                     PromptContext & ctx);

        void setupServiceLogging();
        void logServiceEvent(unsigned int level, const std::string& msg);
        bp::log::ServiceLogMode m_serviceLogMode;
        bp::log::Logger m_serviceLogger;
        
    };
}

#endif
