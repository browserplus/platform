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

/**
 * CommandParser
 * An object meant to facilitate the development of console applications.
 * 
 * Created by Lloyd Hilaiel on Tues Sep 27 2005.
 * Copyright (c) 2005-2009 Yahoo!, Inc. All rights reserved.
 */

#include "CommandParser.h"
#include <iostream>
#include <set>
#include <stdexcept>
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"

using namespace std;
using namespace std::tr1;

#define CMDLINEMAX 2047
#define PROMPT "> "

#define MY_MAX(a,b) ((a>b) ? a : b)   
#define MY_MIN(a,b) ((a<b) ? a : b)   

// use libedit where available for history and command line editing.
#ifndef WIN32
#include "libedit/readline.h"
#include "libedit/histedit.h"

static EditLine *s_el;
static History *s_history;

static const char * prompt(EditLine *) { return "> "; }

static void editline_init()
{
    // initialize editline
    s_el = el_init("", stdin, stdout, stderr);
    el_set(s_el, EL_PROMPT, &prompt);
    el_set(s_el, EL_EDITOR, "emacs");

    // initialize history
    s_history = history_init();

    // set history to 800 lines
    HistEvent ev;
    history(s_history, &ev, H_SETSIZE, 800);

    // set up the call back functions for history functionality
    el_set(s_el, EL_HIST, history, s_history);
}

static void editline_shutdown()
{
    history_end(s_history);
    el_end(s_el);
}

static bool myGets(std::string & line)
{
    int count = 0;
    HistEvent ev;

    const char * ptr = el_gets(s_el, &count);

    if (count > 0) {
      line = bp::strutil::trim(std::string(ptr));
      if (!line.empty()) history(s_history, &ev, H_ENTER, line.c_str());
      return true;
    }
    return false;
}
#else
static void editline_init()
{
}

static void editline_shutdown()
{
}

static bool myGets(std::string & line)
{
    char buf[CMDLINEMAX];
    char *ret;

    fprintf(stdout, PROMPT);
    fflush(stdout);
    ret = fgets(buf, CMDLINEMAX, stdin);
    if (!ret) return false;
    line = bp::strutil::trim(std::string(buf));
    return true;
}
#endif

// a simple command handler class
class CommandParserBuiltins : public CommandHandler
{
public:
    /** the built-in help command */
    BP_DECLARE_COMMAND_HANDLER(help);
    /** the built-in quit command */
    BP_DECLARE_COMMAND_HANDLER(quit);
    CommandParser * parser;
};

struct CommandRegistration
{
    std::string m_commandName;
    CommandHandlerPtr m_handlerPtr;
    void * m_rawptr;
    unsigned int m_minArgs;
    unsigned int m_maxArgs;
    std::string m_doc;
    CommandHandler::callback m_cb;
};
typedef shared_ptr<CommandRegistration> CommandRegistrationPtr;


CommandParser::CommandParser()
    : m_running(false),
      m_waitingForCommandCompletion(false),
      m_registeredBuiltins(false),
      m_listener(NULL)
{
    editline_init();
}

CommandParser::~CommandParser()
{
    editline_shutdown();
}

bool
CommandParser::registerHandler(const std::string & commandName,
                               void * rawptr, 
                               shared_ptr<CommandHandler> handler,
                               CommandHandler::callback cb,
                               unsigned int minArgs, unsigned int maxArgs,
                               const std::string & documentation)
{
    if (m_handlers.find(commandName) != m_handlers.end()) {
        return false;
    }
    handler->setListener(weak_ptr<CommandHandler::IHandlerListener>(
                             shared_from_this()));

    // allocate a structure which holds data about this command handler
    CommandRegistrationPtr ptr(new CommandRegistration);
    ptr->m_commandName = commandName;
    ptr->m_handlerPtr = handler;
    ptr->m_rawptr = rawptr;
    ptr->m_cb = cb;
    ptr->m_minArgs = MY_MAX(minArgs, 0);
    ptr->m_maxArgs = maxArgs;
    ptr->m_doc = documentation;
    
    m_handlers[commandName] = ptr;

    return true;
}

/** parse a token out of the line and modify the string in
 * place */
bool
getTok(std::string & line, std::string & tok)
{
    const char * beg = line.c_str();
    const char * start = beg;
    const char * x;

    while (isspace(*beg)) beg++;

    x = beg;

    switch (*x) {
        case 0:
            return false;
        case '\'': case '\"': {
            char term = *x;
            beg = ++x;
            while (*x && *x != term) x++;
        }
            break;
        default:
            while (isgraph(*x)) x++;
            break;
    }

    // now beg is the beginning, x the end
    tok = line.substr(beg - start, x - beg);
    if ((unsigned int) (1 + x - start) >= line.size()) line.clear();
    else line = line.substr(1 + x - start);

    tok = bp::strutil::trim(tok);
    
    return true;
}

struct ParserEvent
{
    enum { T_ReadEvent, T_QuitEvent } type;
    std::string line;
    std::string command;        
    std::vector<std::string> toks;
};

/** parse commands from the command line until we get either
 *  EOF or a legitimate command */
void *
CommandParser::parse(void * ptrToCommandParser)
{
    CommandParserPtr cpp = *(CommandParserPtr *) ptrToCommandParser;
    delete (CommandParserPtr *) ptrToCommandParser;

    while (cpp->m_running)
    {
        ParserEvent * evt = new ParserEvent;
        
        if (!myGets(evt->line)) {
            // drop a newline on the screen so that the command prompt
            // occurs on the line _after_ the ^D is entered
            std::cout << std::endl;

            // now bundle this up into an object, post it, and exit
            evt->type = ParserEvent::T_QuitEvent;
            cpp->hop(evt);
            break;
        }
    
        // first peel off the command (skip blank commands)
        if (!getTok(evt->line, evt->command)) continue;
        
        std::string tok;
        while (getTok(evt->line, tok)) {
            evt->toks.push_back(tok);
        }
            
        // now bundle this up into an object, post it, and exit
        evt->type = ParserEvent::T_ReadEvent;
        cpp->hop(evt);
        break;
    }
    
    return NULL;
}

void
CommandParser::start()
{
    if (!m_registeredBuiltins) {
        shared_ptr<CommandParserBuiltins>
            ptr(new CommandParserBuiltins);
        ptr->parser = this;
        
        bool s;
    
        // register the built in help command
        s = registerHandler(
            std::string("help"),
            ptr,
            BP_COMMAND_HANDLER(CommandParserBuiltins::help),
            0, 1,
            "Displays command usage.  help <command> gives more\n"
            "verbose usage on a per-command basis\n");
    
        if (!s) throw std::runtime_error("couldn't register help command");

        // and the quit command
        s = registerHandler(std::string("quit"), ptr,
                            BP_COMMAND_HANDLER(CommandParserBuiltins::quit),
                            0, 0, "Quit the program");

        if (!s) throw std::runtime_error("couldn't register quit command");
        

        m_registeredBuiltins = true;
    }
    

    m_running = true;
    
    // allocate a smart pointer to ourself
    CommandParserPtr * cpp = new CommandParserPtr(shared_from_this());

    // spawn the background thread passing in smart pointer to self
    m_thred.run(parse, (void *) cpp);
}

void
CommandParser::stop()
{
    m_running = false;
    // now if the thread is running, we must kick it out of it's block
    // on stdin.
    // XXX: do so by writing a byte '\n' on stdin?

    // now free up memory
    m_handlers.clear();
}

void
CommandParser::onHop(void * context)
{
    BPASSERT(context != NULL);
    
    ParserEvent * evt = (ParserEvent *) context;

    if (evt->type == ParserEvent::T_ReadEvent)
    {
        // allow minimal disambiguous command typing, 'i' for 'info'
        std::set<std::string> candidates;
        std::map<std::string,
                 shared_ptr<struct CommandRegistration> >::iterator it;
        for (it = m_handlers.begin(); it != m_handlers.end(); it++)
        {
            if (!it->first.substr(0, evt->command.size()).compare(evt->command))
            {
                candidates.insert(it->first);
            }
        }

        // invoke command handler
        if (candidates.size() == 0) {
            std::cout << "no such command: " << evt->command << std::endl;
            // restart the parser thread
            m_thred.join();
            start();
        } else if (candidates.size() > 1) {
            std::cout << "'" << evt->command << "' is ambiguous, could be: ";
            std::set<std::string>::iterator i;
            for (i = candidates.begin(); i != candidates.end(); i++)
            {
                if (i != candidates.begin()) std::cout << " or ";
                std::cout << *i;
            }
            std::cout << std::endl;
            // restart the parser thread
            m_thred.join();
            start();
        } else {
            evt->command = *(candidates.begin());

            // gosh, member function pointers are hairy.
            CommandRegistrationPtr handler(m_handlers[evt->command]);

            // now let's check the number of arguments
            if (evt->toks.size() < handler->m_minArgs ||
                evt->toks.size() > handler->m_maxArgs) {
                std::cout << "command: " << evt->command
                          << " accepts between " << handler->m_minArgs
                          << " and " << handler->m_maxArgs
                          << " arguments" << std::endl;

                // restart the parser thread
                m_thred.join();
                start();
            } else {
                // call the handler!  expect a success or failure event
                CommandHandler * hand =
                    (CommandHandler *) handler->m_rawptr;
                CommandHandler::callback cb = handler->m_cb;
                m_waitingForCommandCompletion = true;
                (hand->*cb)(evt->command, evt->toks);
            }
        }
    }
    else if (evt->type == ParserEvent::T_QuitEvent)
    {
        stop();
        if (m_listener) m_listener->onUserQuit();
    }
    delete evt;
}

void
CommandParser::onSuccess()
{
    if (m_waitingForCommandCompletion) {
        // restart the parser thread
        m_thred.join();
        if (m_running) start();
        m_waitingForCommandCompletion = false;
    }
}

void
CommandParser::onFailure()
{
    if (m_waitingForCommandCompletion) {
        // restart the parser thread
        m_thred.join();
        if (m_running) start();
        m_waitingForCommandCompletion = false;
    }
}

void
CommandParser::setListener(ICommandHandlerListener * listener)
{
    m_listener = listener;
}

#ifdef WIN32
    //turn off some annoying warnings about unref'd formal params
#pragma warning(push)
#pragma warning(disable:4100)
#endif
 
BP_DEFINE_COMMAND_HANDLER(CommandParserBuiltins::help)
{
    std::map<std::string, CommandRegistrationPtr >::iterator it;
    
    if (tokens.size() == 1) {
        std::cout << "Verbose help for \"" << tokens[0]
                  << "\":" << std::endl;    

        it = parser->m_handlers.find(tokens[0]);
        if (it == parser->m_handlers.end()) {
            std::cout << "no such command" << std::endl;
        } else {
            CommandRegistrationPtr handler = it->second;

            std::cout << it->first << " [";
            if (handler->m_minArgs == handler->m_maxArgs) {
                std::cout << handler->m_minArgs << " arg"
                          << ((handler->m_minArgs > 1) ? "s" : "");
            } else {
                std::cout << handler->m_minArgs << "-" << handler->m_maxArgs
                          << " args";
            }
        
            // help string
            std::cout << "] " << std::endl << std::endl;
            std::cout << handler->m_doc << std::endl;
        }
    } else {
        std::cout << "Available commands:" << std::endl;    

        for (it = parser->m_handlers.begin();
             it != parser->m_handlers.end(); it++)
        {
            std::string name = it->first;
            CommandRegistrationPtr handler = it->second;

            std::cout << "  " <<  name << " [";
            if (handler->m_minArgs == handler->m_maxArgs) {
                std::cout << handler->m_minArgs << " arg"
                          << ((handler->m_minArgs != 1) ? "s" : "");
            } else if (handler->m_maxArgs == (unsigned int) -1) {
                std::cout << handler->m_minArgs << " or more args";
            } else {
                std::cout << handler->m_minArgs << "-" << handler->m_maxArgs
                          << " args";
            }
        
            // help string
            std::string docbuf;
            docbuf.append(handler->m_doc.substr(0, 50));
            std::cout << "]: " << docbuf << "..." << std::endl;
        }
    }

    onSuccess();
}

BP_DEFINE_COMMAND_HANDLER(CommandParserBuiltins::quit)
{
    /* stop the parser thread don't emit a prompt, don't pass go, etc */
    parser->stop();

    ParserEvent * evt = new ParserEvent;
    evt->type = ParserEvent::T_QuitEvent;
    parser->hop(evt);
}


#ifdef WIN32
#pragma warning(pop)
#endif
