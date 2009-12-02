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

#include "ConsoleLib/ConsoleLib.h"
#include "DistributionClient/DistributionClient.h"
#include "CoreletManager/CoreletManager.h"

// a class which handles commands typed at the command line client
class CommandExecutor : public CommandHandler
{
public:
    CommandExecutor(std::list<std::string> distroServers);
    ~CommandExecutor();    

    BP_DECLARE_COMMAND_HANDLER(available);
    BP_DECLARE_COMMAND_HANDLER(platform);
    BP_DECLARE_COMMAND_HANDLER(permissions);
    BP_DECLARE_COMMAND_HANDLER(details);
    BP_DECLARE_COMMAND_HANDLER(find);
    BP_DECLARE_COMMAND_HANDLER(satisfy);
    BP_DECLARE_COMMAND_HANDLER(installed);
    BP_DECLARE_COMMAND_HANDLER(cached);
    BP_DECLARE_COMMAND_HANDLER(isCached);
    BP_DECLARE_COMMAND_HANDLER(updateCache);
    BP_DECLARE_COMMAND_HANDLER(purgeCache);
    BP_DECLARE_COMMAND_HANDLER(installFromCache);
    BP_DECLARE_COMMAND_HANDLER(haveUpdates);
    BP_DECLARE_COMMAND_HANDLER(strings);
    BP_DECLARE_COMMAND_HANDLER(latestPlatformVersion);
    BP_DECLARE_COMMAND_HANDLER(downloadLatestPlatform);
    
private:    
    class CommandExecutorRunner * m_runner;
};

#endif
