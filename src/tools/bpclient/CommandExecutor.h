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

#ifndef __COMMANDEXECUTOR_H__
#define __COMMANDEXECUTOR_H__

#include "ConsoleLib/ConsoleLib.h"
#include "BPProtocol/BPProtocol.h"

// a unique string which designates the bpcommand line client
// program.  Used to compose the URI parameter to BPConnect
#define BPCLIENT_UUID "9A440933-D16C-4D81-844A-791B9C4D4880"
#define BPCLIENT_APPNAME "BrowserPlus Command Line Tool"

// a class which handles commands typed at the command line client
class CommandExecutor : public CommandHandler
{
public:
    CommandExecutor();
    ~CommandExecutor();    

    BP_DECLARE_COMMAND_HANDLER(connect);
    BP_DECLARE_COMMAND_HANDLER(execute);
    BP_DECLARE_COMMAND_HANDLER(describe);
    BP_DECLARE_COMMAND_HANDLER(enumerate);
    BP_DECLARE_COMMAND_HANDLER(require);
    BP_DECLARE_COMMAND_HANDLER(getState);
    BP_DECLARE_COMMAND_HANDLER(setState);
    
private:    
    BPProtoHand m_hand;

    static void stateCB(BPErrorCode ec, void * cookie,
                           const BPElement * value);

    static void installedCB(BPErrorCode ec, void * cookie,
                            const BPElement * value);

    static void descriptionCB(BPErrorCode ec,
                              void * cookie,
                              const BPCoreletDefinition * def,
                              const char * error,
                              const char * verboseError);

    static void requireCB(BPErrorCode ec,
                          void * cookie,
                          const BPCoreletDefinition ** defs,
                          unsigned int numDefs,
                          const char * error,
                          const char * verboseError);

    static void connectCB(BPErrorCode ec, void * cookie,
                          const char * error, const char * verboseError);

    static void promptCB(void *, const char *, const BPElement *,
                         unsigned int);

    static void invokeCB(void *, unsigned int, long long int,
                         const BPElement *);
    
    static void resultsCB(void * ptr, unsigned int tid,
                          BPErrorCode ec, const BPElement * results);

};

#endif
