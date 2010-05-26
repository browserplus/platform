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
#include "BPUtils/bpfile.h"
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

    class ServiceLibrary
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
        const bp::service::Description & description();

        /** returns zero on failure (client allocate() function failed), or non-zero id
         *  upon success */
        unsigned int allocate(std::string uri, bp::file::Path dataDir,
                              bp::file::Path tempDir, std::string locale,
                              std::string userAgent, unsigned int clientPid);

        void destroy(unsigned int id);

        bool invoke(unsigned int id, unsigned int tid,
                    const std::string & function,
                    const bp::Object * arguments,
                    std::string & err);

        void promptResponse(unsigned int promptId,
                            const bp::Object * arguments);

      private:
        std::tr1::shared_ptr<class ServiceLibraryImpl> m_impl;
        
        bp::service::Summary m_summary;

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

        /* a pointer to handle returned by dlopenNP */
        void * m_handle;
    };
}

#endif
