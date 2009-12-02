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

#include "ConsoleLib/ConsoleLib.h"
#include <iostream>
#include "BPUtils/bprunloop.h"

using namespace std;
using namespace std::tr1;


// a simple command handler class
class MyHandler : public CommandHandler
{
public:
    BP_DECLARE_COMMAND_HANDLER(echo);
};

BP_DEFINE_COMMAND_HANDLER(MyHandler::echo)
{
    std::cout << command;
    for (unsigned int i = 0; i < tokens.size(); i++) {
        std::cout << " " << tokens[i];
    }
    std::cout << std::endl;
    onSuccess();
}

// a class that will stop our runloop when onUserQuit occurs
class MyStopper : public ICommandHandlerListener
{
public:
    MyStopper(bp::runloop::RunLoop *rl) : m_rl(rl) { }
    ~MyStopper() { }
    
    void onUserQuit() {
        m_rl->stop();
    }
private:    
    bp::runloop::RunLoop *m_rl;
};


int
main(void)
{
    // allocate and initialize a runloop
    bp::runloop::RunLoop rl;
    rl.init();

    // allocate a listener for onUserQuit events
    MyStopper stopper(&rl);

    // allocate a Parser
    CommandParserPtr parser(new CommandParser);
    
    // set the stopper as the listener of the parser
    parser->setListener(&stopper);

    // allocate a command handler
    shared_ptr<MyHandler> handler(new MyHandler);

    // register the command handler with the parser
    parser->registerHandler("echo", handler,
                            BP_COMMAND_HANDLER(MyHandler::echo),
                            0, (unsigned int) -1,
                            "echo a string to console");

    // start the parser up
    parser->start();

    // run our application's runloop
    rl.run();

    // stop the parser
    parser->stop();

    // shutdown the runloop
    rl.shutdown();

    return 0;
}
