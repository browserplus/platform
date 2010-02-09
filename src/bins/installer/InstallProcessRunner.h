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
 * A simple proxy abstraction which runs BrowserPlusUpdater in a 
 * separate process and gets status/progress/errors via IPC
 */

#ifndef __INSTALL_PROCESS_RUNNER_H__
#define __INSTALL_PROCESS_RUNNER_H__

#include "BPUtils/bptr1.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/IPCChannelServer.h"
#include "BPInstaller/BPInstaller.h"
#include "BPUtils/bptimer.h"


class InstallProcessRunner : public bp::time::ITimerListener,
                             public bp::ipc::IChannelServerListener,
                             public bp::ipc::IChannelListener,
                             public std::tr1::enable_shared_from_this<InstallProcessRunner>

{
  public:
    InstallProcessRunner(const bp::file::Path& logPath,
                         const std::string& logLevel);
    virtual ~InstallProcessRunner();

    void setListener(std::tr1::weak_ptr<bp::install::IInstallerListener> listener);
                
    // spawn BrowserPlusUpdater
    void start(const bp::file::Path& dir,
               bool deleteWhenDone = false);

  private:
    // implementation of ITimerListener interface
    void timesUp(bp::time::Timer* t);

    // implementation of IChannelServerListener interface
    void gotChannel(bp::ipc::Channel* c);
    void serverEnded(bp::ipc::IServerListener::TerminationReason,
                     const char*) const;

    // implementation of IChannelListener interface
    void channelEnded(bp::ipc::Channel* c,
                      bp::ipc::IConnectionListener::TerminationReason why,
                      const char* errorString);
    void onMessage(bp::ipc::Channel* c,
                   const bp::ipc::Message& m);
    bool onQuery(bp::ipc::Channel* c,
                 const bp::ipc::Query& query,
                 bp::ipc::Response& response);
    void onResponse(bp::ipc::Channel* c,
                    const bp::ipc::Response& response);

    bp::file::Path m_logPath;
    std::string m_logLevel;
    std::tr1::weak_ptr<bp::install::IInstallerListener> m_listener;
    std::string m_ipcName;
    mutable std::tr1::shared_ptr<bp::ipc::ChannelServer> m_server;
    bp::process::spawnStatus m_procStatus;
    bp::time::Timer m_timer;
    unsigned int m_timerFires;
    bool m_connectedToUpdater;
};

#endif
