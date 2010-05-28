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
 * An abstract base class for a service library.  Fuels the mechanism that
 * lets us support multiple API versions simultaneously 
 */

#ifndef __SERVICELIBRARYIMPL_H__
#define __SERVICELIBRARYIMPL_H__

#include "ServiceLibrary.h"

#include <string>

namespace ServiceRunner 
{
    class ServiceLibraryImpl
    {
      public:
        ServiceLibraryImpl() { };
        virtual ~ServiceLibraryImpl() { };

        virtual void setListener(IServiceLibraryListener * listener) = 0;
        
        virtual bool load(const bp::service::Summary &summary,
                          const bp::service::Summary &provider,
                          void * functionTable) = 0;

        virtual unsigned int apiVersion() = 0;
        
        virtual std::string name() = 0;
        virtual std::string version() = 0;
        virtual const bp::service::Description & description() = 0;

        /** returns zero on failure (client allocate() function failed), or non-zero id
         *  upon success */
        virtual unsigned int allocate(std::string uri, bp::file::Path dataDir,
                                      bp::file::Path tempDir, std::string locale,
                                      std::string userAgent, unsigned int clientPid) = 0;

        virtual void destroy(unsigned int id) = 0;

        virtual bool invoke(unsigned int id, unsigned int tid,
                            const std::string & function,
                            const bp::Object * arguments,
                            std::string & err) = 0;

        virtual void promptResponse(unsigned int promptId,
                                    const bp::Object * arguments) = 0;

    };
}

#endif
