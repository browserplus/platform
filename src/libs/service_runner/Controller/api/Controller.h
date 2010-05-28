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
 * An abstraction which allows a controlling process (usually
 * BrowserPlusCore) fork, exec, examine, and interact with a
 * spawned process (a service instance).
 *
 * First introduced by Lloyd Hilaiel on 2009/01/14
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __SPAWNEDSERVICECONTROLLER_H__
#define __SPAWNEDSERVICECONTROLLER_H__

#include <string>

#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bptimer.h"
#include "BPUtils/bptr1.h"
#include "BPUtils/IPCServer.h"
#include "BPUtils/IPCChannel.h"
#include "BPUtils/IPCChannelServer.h"
#include "BPUtils/ServiceDescription.h"
#include "BPUtils/ServiceSummary.h"
#include "BPUtils/bpfile.h"


namespace ServiceRunner 
{
    // a utility function which can allow one to scan the standard
    // service installation directory for a satisfying provider service
    // used by many client programs, so available here
    //
    // a non-empty path to the provider service will be provided on
    // success.  upon failure an empty string will be returned with
    // 'err' populated with a human readable error message
    bp::file::Path determineProviderPath(const bp::service::Summary & summary,
                                         std::string & err);

    class IControllerListener  
    {
      public:
        virtual void initialized(class Controller * c,
                                 const std::string & service,
                                 const std::string & version,
                                 unsigned int apiVersion) = 0;

        // Callback invoked when controller's service is determined to be gone.
        // - called if service process dies while waiting for IPC
        //   connection to be established.
        // - called if IPC connection goes down.
        virtual void onEnded(class Controller * c) = 0;

        virtual void onDescribe(class Controller * c,
                                const bp::service::Description & desc) = 0;
        virtual void onAllocated(class Controller * c,
                                 unsigned int allocationId,
                                 unsigned int instanceId) = 0;
        virtual void onInvokeResults(class Controller * c,
                                     unsigned int instanceId,
                                     unsigned int tid,
                                     const bp::Object * results) = 0;
        virtual void onInvokeError(class Controller * c,
                                   unsigned int instanceId,
                                   unsigned int tid,
                                   const std::string & error,
                                   const std::string & verboseError) = 0;
        virtual void onCallback(class Controller * c,
                                unsigned int instanceId,
                                unsigned int tid,
                                long long int callback,
                                const bp::Object * value) = 0;
        virtual void onPrompt(class Controller * c,
                              unsigned int instanceId,
                              unsigned int promptId,
                              const bp::file::Path & pathToDialog,
                              const bp::Object * arguments) = 0;

        virtual ~IControllerListener() { }
    };

    /**
     *  A ServiceRunner::Controller is an object which manages
     *  a spawned service.  This object is a handle to the
     *  remote running service and is used to initially spawn
     *  the process and abstracts all IPC providing an asynchronous
     *  interface.
     *
     *  requirements:
     *  A Controller must be allocated and owned with a boost
     *  shared pointer, as weak pointers are used internally.
     */
    class Controller : public bp::ipc::IChannelListener,
                       public bp::time::ITimerListener,
                       public std::tr1::enable_shared_from_this<Controller>
    {
      public:
        /**
         * Instantiate a Controller for a installed service.  Path
         * to the service will be assumed to be standard installation
         * path, determined using  bp::paths::getServiceDirectory()
         */
        Controller(const std::string & service, const std::string & version);
        /**
         * Instantiate a Controller for a service.  Path is explicitly
         * provided.
         */
        Controller(const bp::file::Path & pathToService);

        ~Controller();
        
        // valid after constructor invoked with the full path to the
        // service.  Depending on the constructor invoked this is either
        // calculated or specified.
        bp::file::Path path() { return m_path; }

        // return the name of the service we manage.
        // This is currently only valid after IPC connection succeeds
        std::string serviceName() { return m_service; }

        // return the version of the service we manage.
        // This is currently only valid after IPC connection succeeds
        std::string serviceVersion() { return m_version; }
        
        void setListener(IControllerListener * listener);

        // spawn and run the service.  To do this a harness program is
        // required that will be exec'd, load the service, and establish
        // an IPC channel with the host.  This harness must accept certain
        // flags as command line arguments.  If empty, the harness will
        // be BrowserPlusCore.  Other programs may also link the
        // ServiceRunner library and serve as harnesses.  A feature which
        // can be used to build completely standalone execution shells,
        // useful for building test programs.
        //
        // When running a dependent service, it is required to explicitly
        // supply the path to the provider service.
        //
        // serviceTitle will be set as the name of the child process as
        // visible through 'ps' and friends
        bool run(const bp::file::Path & pathToHarness,
                 const bp::file::Path & pathToProvider,
                 const std::string & serviceTitle,
                 const std::string & logLevel,
                 const bp::file::Path & logFile, 
                 std::string & err);

        // get a description of the service
        void describe();

        // allocate an instance of a service, passing in context.
        // returns an allocation id that will be passsed back in the
        // onAllocated call.
        // returns ((unsigned int) -1) on failure
        unsigned int allocate(const std::string & uri,
                              const bp::file::Path & data_dir,
                              const bp::file::Path & temp_dir,
                              const std::string & locale,
                              const std::string & userAgent,
                              unsigned int clientPid);

        // destroy an instance by ID
        void destroy(unsigned int id);

        // invoke a service function on the specified instance with the
        // specified arguments
        unsigned int invoke(unsigned int instanceId,
                            const std::string & function,
                            const bp::Object * arguments);

        // send a user's response to an html prompt
        void sendResponse(unsigned int promptId,
                          const bp::Object * arguments);

        // exit code of the process we manage.
        // currently only valid in the premature exit case.
        int processExitCode() { return m_processExitCode; }

        // Returns a nice name for the service we manage.
        // If service has connected it'll be "name (version)",
        // otherwise it'll be the service path.
        std::string friendlyServiceName();
                
      private:
        std::string m_service;
        std::string m_version;
        bp::file::Path m_path;
        int m_pid;
        bp::process::spawnStatus m_spawnStatus;
        
        static std::tr1::shared_ptr<bp::ipc::ChannelServer> m_server;
        IControllerListener * m_listener;

        // used for rough timing of various actions
        bp::time::Stopwatch m_sw;

        // invoked by the Connector when an IPC channel is allocated
        // with a spawned service instance
        void onConnected(bp::ipc::Channel * c,
                         const std::string & name,
                         const std::string & version,
                         unsigned int apiVersion);
        friend class Connector;

        // container for the channel once onConnected called
        std::tr1::shared_ptr<bp::ipc::Channel> m_chan;

        // implemented methods from bp::ipc::IChannelListener
        // for handling of events that come in over the IPC channel
        // from the service
        void channelEnded(bp::ipc::Channel * c,
                          bp::ipc::IConnectionListener::TerminationReason why,
                          const char * errorString);
        void onMessage(bp::ipc::Channel * c, const bp::ipc::Message & m);
        bool onQuery(bp::ipc::Channel * c, const bp::ipc::Query & query,
                     bp::ipc::Response & response);
        void onResponse(bp::ipc::Channel * c,
                        const bp::ipc::Response & response);

        // id, assigned by Connector, that allows us to connect up with the
        // spawned service.
        unsigned int m_id;

        // after spawning we'll asynchronously poll the spawned process
        // to detect premature exit (before IPC channel is established)
        bp::time::Timer m_spawnCheckTimer;
        void timesUp(bp::time::Timer *);

        std::tr1::shared_ptr<class Connector> m_serviceConnector;

        // whether controlled service ever connected
        // note currently serviceName/Version are not valid until connected.
        bool m_everConnected;
        
        // Process exit code (currently only set in premature exit case).
        int m_processExitCode;

        // Information about termination of the IPC channel (currently
        // only set in channelEnded case).
        bp::ipc::IConnectionListener::TerminationReason m_chanTermReason;
        std::string m_chanTermErrorString;

        // Temp dirs that were provided to instances.
        // We purge at end in case service does not.
        std::vector<bp::file::Path> m_tempDirs;
    };
}

#endif
