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
 * An abstraction around the service side of the IPC protocol spoken
 * with spawned services.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/15
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __SERVICEPROTOCOL_H__
#define __SERVICEPROTOCOL_H__

#include "ServiceLibrary.h"
#include "bpipc/IPCChannel.h"
#include "BPUtils/bpfile.h"

#include <string>

namespace ServiceRunner 
{
    class ServiceProtocol : public bp::ipc::IChannelListener,
                            public IServiceLibraryListener 
    {
      public:
        ServiceProtocol(ServiceLibrary * lib, bp::runloop::RunLoop * rl,
                        const std::string & ipcName);
        bool connect();
        ~ServiceProtocol();
      private:
        // implemented methods from bp::ipc::IChannelListener
        void channelEnded(bp::ipc::Channel * c,
                          bp::ipc::IConnectionListener::TerminationReason why,
                          const char * errorString);
        void onMessage(bp::ipc::Channel * c, const bp::ipc::Message & m);
        bool onQuery(bp::ipc::Channel * c, const bp::ipc::Query & query,
                     bp::ipc::Response & response);
        void onResponse(bp::ipc::Channel * c,
                        const bp::ipc::Response & response);
        
        ServiceLibrary * m_lib;
        bp::ipc::Channel m_chan;
        bp::runloop::RunLoop * m_rl;

        // methods from IServiceLibraryListener
        void onResults(unsigned int instance, unsigned int tid,
                       const bp::Object * o);
        void onError(unsigned int instance, unsigned int tid,
                     const std::string & error,
                     const std::string & verboseError);
        void onCallback(unsigned int instance,
                        unsigned int tid,
                        long long int callbackId,
                        const bp::Object * o);
        void onPrompt(unsigned int instance,
                      unsigned int promptId,
                      const bp::file::Path & pathToDialog,
                      const bp::Object * arguments);

        std::string m_ipcName;
    };
};

#endif
