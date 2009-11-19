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
 * CommandParser.h
 * An abstraction meant to facilitate the development of console applications.
 * 
 * Created by Lloyd Hilaiel on Tues Oct 30 2005.
 * Copyright (c) 2005-2009 Yahoo!, Inc. All rights reserved.
 *
 * TODO:
 *  - extend self documentation facilities
 *  - some way to allow clients to keep the handler definition and the
 *    documentation & argument requirements together in one place
 *    (structure based registration?)
 */

#ifndef __BP_COMMANDPARSER_H__
#define __BP_COMMANDPARSER_H__


#include <map>
#include "BPUtils/bpthread.h"
#include "BPUtils/bpthreadhopper.h"
#include "CommandHandler.h"



class ICommandHandlerListener
{
  public:
    /** an event raised when the user types ^D or uses the
     *  built in 'quit' command */
    virtual void onUserQuit() = 0;
    virtual ~ICommandHandlerListener() { }
};

/**
 * An object meant to facilitate the development of console applications.
 * 
 * It parses commands off the command line and passes then to the correct
 * registered handler for execution.
 */
class CommandParser : public CommandHandler::IHandlerListener,
                      public std::tr1::enable_shared_from_this<CommandParser>,
                      public bp::thread::HoppingClass
{
public:
    /**
     * Constructor.
     */
    CommandParser();
    ~CommandParser();

    /** set the listener that will be notified when the parser is stopped
     *  via user interaction (^D or quit) */
    void setListener(ICommandHandlerListener * listener);

    /**
     * register a handler to handle a specific type of command
     *
     * A side effect of registration is that the handler's listener
     * will be set to the CommandParser object.  This means that
     * Handler objects may not, themselves emit events to objects other
     * than the command parser.
     *
     * \param error[out] For when things go wrong
     * \param commandName The command name to associate this handler with
     * \param handler a pointer to the handler to be invoked
     * \param cb a pointer to the method on the handler to be invoked
     * \param minArgs the minimun number of arguments allowed, if negative
     *                CommandParser does no validation  
     * \param maxArgs the maximum number of arguments allowed, if negative
     *                CommandParser does no validation  
     * \param documentation Documentation of the command.  Should be
     *                   formatted with linebreaks.  The first 60 chars
     *                   or so will be used as an abbreviated help output.
     */
    template <class T>
        bool registerHandler(const std::string & commandName,
                             std::tr1::shared_ptr<T> handler,
                             CommandHandler::callback cb,
                             unsigned int minArgs, unsigned int maxArgs,
                             const std::string & documentation);


    /**
     * start the command parser.  This will spawn a background
     * thread which will take ownership of stdin.  The thread
     * will consume commands, and cross post a parsed version of these
     * commands to the main runloop.  Command handlers will be executed
     * on the main runloop.  Upon completion of command handler
     * EXECTUTION, the parser thread will be re-run to read the next
     * command.
     */
    void start(void);

    /**
     * stop the parser, and unregister all commands (releasing boost
     * shared ptrs).  This is a (briefly) blocking call that waits
     * for the parser thread to exit.
     */
    void stop(void);

  private:
    /** the thread function for the background thread */
    static void * parse(void * ptrToSharedPtrOfCommandParser);

    /** how we get back events from the parser thread */
    virtual void onHop(void * context);

    bool registerHandler(const std::string & commandName,
                         void * rawptr, 
                         std::tr1::shared_ptr<CommandHandler> handler,
                         CommandHandler::callback cb,
                         unsigned int minArgs, unsigned int maxArgs,
                         const std::string & documentation);

    // IHandlerListener implementation
    // - how the parser is informed when command handlers complete execution
    virtual void onSuccess();
    virtual void onFailure();

    /** a map of registered handlers */
    std::map<std::string, std::tr1::shared_ptr<struct CommandRegistration> >
        m_handlers;

    /** the background thread */
    bp::thread::Thread m_thred;

    /** are we running? */
    bool m_running;

    /** are we in the process of executing a command callback?
     *  if not, we'll ignore errant calls to onSuccess/onFailure */
    bool m_waitingForCommandCompletion;

    /** have we registered our built-in functions, (quit and help)*/
    bool m_registeredBuiltins;

    /** a listener that will be notified when the parser is stopped
     *  via user interaction (^D or quit) */
    ICommandHandlerListener * m_listener;

    friend class CommandParserBuiltins;
};

template <class T>
bool
CommandParser::registerHandler(const std::string & commandName,
                               std::tr1::shared_ptr<T> handler,
                               CommandHandler::callback cb,
                               unsigned int minArgs, unsigned int maxArgs,
                               const std::string & documentation)
{
    return registerHandler(commandName, (void *) handler.get(),
                           handler, cb, minArgs, maxArgs, documentation);
}

typedef std::tr1::shared_ptr<CommandParser> CommandParserPtr;

#endif
