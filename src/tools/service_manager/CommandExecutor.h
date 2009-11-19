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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

#ifndef __COMMANDEXECUTOR_H__
#define __COMMANDEXECUTOR_H__

#include "ConsoleLib/ConsoleLib.h"
#include "CoreletManager/CoreletManager.h"
#include "BPUtils/bpfile.h"
#include "InstanceManager.h"

// a class which handles commands typed at the command line client
class CommandExecutor : public CommandHandler
{
  public:
    CommandExecutor(const std::string & loglevel, const bp::file::Path& logfile);

    ~CommandExecutor();    

    BP_DECLARE_COMMAND_HANDLER(available);
    BP_DECLARE_COMMAND_HANDLER(describe);
    BP_DECLARE_COMMAND_HANDLER(summarize);
    BP_DECLARE_COMMAND_HANDLER(have);
    BP_DECLARE_COMMAND_HANDLER(instantiate);
    BP_DECLARE_COMMAND_HANDLER(destroy);
    BP_DECLARE_COMMAND_HANDLER(execute);
    BP_DECLARE_COMMAND_HANDLER(rescan);
    BP_DECLARE_COMMAND_HANDLER(purge);

  private:
    std::tr1::shared_ptr<DynamicServiceManager> m_servMan;
    std::tr1::shared_ptr<InstanceManager> m_instanceMan;
    friend class InstanceManager;
    unsigned int m_currentTid;
};

#endif
