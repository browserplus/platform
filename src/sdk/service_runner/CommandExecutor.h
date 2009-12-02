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

#ifndef __COMMANDEXECUTOR_H__
#define __COMMANDEXECUTOR_H__

#include "ServiceRunnerLib/ServiceRunnerLib.h"
#include "ConsoleLib/ConsoleLib.h"

#include "ControllerManager.h"

// a unique string which designates the bpcommand line client
// program.  Used to compose the URI parameter to allocate
#define BPCLIENT_UUID "3CFD49C8-171D-4687-AE36-AA34C7B4C590"
#define BPCLIENT_APPNAME "BrowserPlus ServiceRunner Command Line Tool"

// a class which handles commands typed at the command line client
class CommandExecutor : public CommandHandler
{
  public:
    CommandExecutor(std::tr1::shared_ptr<ServiceRunner::Controller> controller);
    ~CommandExecutor();    

    void start(const bp::service::Description & desc);

    BP_DECLARE_COMMAND_HANDLER(allocate);
    BP_DECLARE_COMMAND_HANDLER(destroy);
    BP_DECLARE_COMMAND_HANDLER(invoke);
    BP_DECLARE_COMMAND_HANDLER(show);
    BP_DECLARE_COMMAND_HANDLER(describe);
    BP_DECLARE_COMMAND_HANDLER(select);
    BP_DECLARE_COMMAND_HANDLER(respond);
    BP_DECLARE_COMMAND_HANDLER(prompts);

  private:
    std::tr1::shared_ptr<ServiceRunner::Controller> m_controller;
    bp::service::Description m_desc;
    // A class that wraps the controller and outputs to console.
    std::tr1::shared_ptr<ControllerManager> m_controlMan;
    friend class ControllerManager;
};

#endif
