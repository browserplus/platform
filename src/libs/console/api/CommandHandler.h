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

/**
 * CommandHandler.h
 * Callback functions that handle commands (\see CommandParser.h)
 *
 * Created by Lloyd Hilaiel on Mon Oct 28 2005.
 * Copyright (c) 2005-2009 Yahoo!, Inc. All rights reserved.
 *
 */

#ifndef __COMMANDHANDLER_H__
#define __COMMANDHANDLER_H__

#include <vector>
#include <string>
#include "BPUtils/bptr1.h"


/**
 * A macro to simplify casting of routines derived from
 * CommandHandler to the expected format
 */
#define BP_COMMAND_HANDLER(cb) \
  (reinterpret_cast<CommandHandler::callback>(&cb))

/**
 * A utility macro to simplify declaration of command handlers
 * This macro is to be used in a class that derives from
 * CommandHandler:
 *
 * \code 
 * class SampleHandler : public CommandHandler
 * {
 *   public:
 *     SampleHandler();
 *     virtual ~SampleHandler();
 *   
 *     BP_DECLARE_COMMAND_HANDLER(myHandlerName);
 * };
 * \endcode
 *
 * Then an instance of this class may be registered on a 
 * CommandParser which handles keyboard input.
 */
#define BP_DECLARE_COMMAND_HANDLER(handlerName) \
void \
(handlerName)(const std::string & command,        \
              const std::vector<std::string> & tokens)

/**
 * A utility macro to simplify definition of command handlers
 * 
 * Note, command handlers _must not block_.  They indicate their
 * completion via callbacks.  In the simple case of a non-blocking
 * handler, it's sufficient to sendEvent(SuccessEvent) at the end of
 * the handler.
 *  
 * This macro is intended to be used in an implementation file when
 * defining the body of the command handler.
 * \code
 * BP_DEFINE_COMMAND_HANDLER(MyClass::myHandlerName)
 * {
 *    ... code ...
 *    sendEvent(SuccessEvent);
 * }
 * \endcode
 * In the body of your handler, you have access to the command name
 * in the "command" parameter.  You have access to an array of tokens
 * the "tokens" parameter, and the number of tokens in the numTokens
 * parameter.
 *
 * Command handlers are to raise SuccessEvent upon success, or
 * FailureEvent upon failure.
 */
#define BP_DEFINE_COMMAND_HANDLER(handlerName) \
        BP_DECLARE_COMMAND_HANDLER(handlerName) 

/**
 * A object which implements the CommandHandler interface may be
 * registered as a command handler using the
 * CommandParser::registerHandler() function.
 *
 * The handler will be called with a command is parsed whose name
 * matches the name passed at registration time.
 */
class CommandHandler
{
  public:
    // a function that should be invoked upon successful completion of
    // processing a command
    void onSuccess();
    // a function that should be invoked when we fail to process a command
    void onFailure();

    typedef bool (CommandHandler::* callback)(
        const std::string & command,
        const std::vector<std::string> & tokens);
    // everthing beneath this point is implementation detail
    // it could be moved out of the interface to keep things clean
    
    // an interface that allows the CommandParser to get onSuccess,
    // onFailure calls
    class IHandlerListener 
    {
      public:
        virtual void onSuccess() = 0;
        virtual void onFailure() = 0;
        virtual ~IHandlerListener() { };
    };

  private:
    
    void setListener(std::tr1::weak_ptr<IHandlerListener> listener);
    std::tr1::weak_ptr<IHandlerListener> m_listener;
    
    friend class CommandParser;
};

typedef std::tr1::shared_ptr<CommandHandler> CommandHandlerPtr;

#endif

