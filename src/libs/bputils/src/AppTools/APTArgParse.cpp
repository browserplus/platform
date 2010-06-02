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
 * APTArgParse - a command line argument parser
 *
 * Created by Lloyd Hilaiel on Tues Feb 7 2007
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 *
 * This module was inspired by the WCConfigure class from WUF. 
 *
 * TODO: integrate configuration file parsing
 */

#include "api/APTArgParse.h"

#include <string.h>
#include <stdlib.h>

const bool APT::NO_ARG = false;
const bool APT::TAKES_ARG = true;
const char* APT::NO_DEFAULT = "";
const bool APT::NOT_REQUIRED = false;
const bool APT::REQUIRED = true;    
const bool APT::NOT_INTEGER = false;
const bool APT::IS_INTEGER = true;    
const bool APT::MAY_NOT_RECUR = false;    
const bool APT::MAY_RECUR = true;    

APTArgParse::APTArgParse(const std::string& programDocumentation)
{
    m_programDoc = programDocumentation;
}

APTArgParse::~APTArgParse()
{
    
}

/** get the argument to a flag.  returns empty if no argument was
 *  provided, else the arguemnt
 */
static std::string
getArgument(const std::string& flag, int & i, int argc, const char ** argv)
{

    /* cases are thus:
     *
     * 1. cmd -f -x
     * 2. cmd -f
     * 3. cmd -featme
     * 4. cmd -f eatme
     * 5. cmd -f=eatme
     */
                
    /* deal with cases 3 and 5 */
    if (strlen(argv[i]) > (flag.length() + 1)) {
        const char * ptr;
        ptr = (argv[i] + (flag.length() + 1));
        /* case 5 */
        if (*ptr == '=') ptr++;
        return std::string(ptr);
    } else if ((i+1>=argc) || ((*(argv[i+1]) == '-'))) {
        /* case 1 and 2, this function should not be called */
        return std::string();
    } else {
        /* which leaves us with case 4 */
        i++;
        return std::string(argv[i]);
    }
}

// try to format a doc string, breaking lines at 60 chars
#define BREAKAT 60
static void
formatDocString(std::string& inout)
{
    unsigned int ix;
    unsigned int space = 0;
    unsigned int lastBreak = 0;
    for (ix = 0; ix < inout.length(); ix++)
    {
        if (inout[ix] == ' ') space = ix;

        if (ix - lastBreak >= BREAKAT && space != 0)
        {
            inout.insert(space+1, "\n\t\t");
            ix += 3;
            lastBreak = ix;
            space = 0;
        }
    }
}

int
APTArgParse::parse(unsigned int elems, const APTArgDefinition * definition, 
                   int argc, const char ** argv)
{
    int i;
    const APTArgDefinition * def;

    // build up usage
    m_usage = argv[0];
    m_usage.append(":");
    m_usage.append(m_programDoc);
    m_usage.append("\nusage:\n"); 

    for (def = definition; def < definition + elems ; def++)
    {
        m_usage.append("    -");
        m_usage.append(def->flag);
        if (def->acceptsArgument) {
            if (def->required && def->defaultValue.empty())
            {
                m_usage.append(" <arg>\t");
            }
            else
            {
                m_usage.append(" [arg]\t");                
            }
        } else {
            m_usage.append("      \t");
        }
        
        {
            // now for documentation
            std::string doc(def->docString);
            if (def->defaultValue.empty() == false) {
                doc.append(" (default: \"");
                doc.append(def->defaultValue);
                doc.append("\")");
            }
            formatDocString(doc);
            m_usage.append(doc);
        }
        
        m_usage.append("\n");
    }

    // first pass, parse command line arguments
    for (i = 1; i < argc; i++)
    {
        if (*(argv[i]) == '-')
        {
            std::string flag(argv[i] + 1);

            // (0) special case for -h
            if (flag.compare("h") == 0)
            {
                m_error = usage();
                return -1;
            }

            // (1) we must verify that this is an allowable flag
            // furthermore, susan, we're looking for the longest
            // flag that matches.
            const APTArgDefinition * curdef;
            def = NULL;
            for (curdef = definition; curdef < definition + elems ; curdef++)
            {
                if ((flag.find(curdef->flag, 0) != std::string::npos) &&
                    (def == NULL || curdef->flag.length() > def->flag.length()))
                {
                    def = curdef;
                }
            }
            if (def == NULL)
            {
                // no such flag is supported
                m_error = "unsupported command line flag '";
                m_error.append(flag);
                m_error.append("'");
                i = -1;
                break;
            }

            // validate multiple occurances
            if (!def->allowMultipleOccurances && argumentPresent(def->flag))
            {
                m_error = "flag '" + def->flag + "' may not occur more than once";
                i = -1;
                break;
            }

            // (2) determine if argument is required
            if (!def->acceptsArgument) {
                // handle case where -xFOO is used with flag that
                // does not accept an arugment
                if (def->flag.length() != flag.length())
                {
                    m_error = "flag '" + def->flag + "' does not accept an argument";                    
                    i = -1; 
                    break;
                }
                // insert into map
                m_args[def->flag].push_back(std::string());
            } else {
                std::string arg = getArgument(def->flag, i, argc, argv);
                if (arg.empty())
                {
                    m_error = "flag '" + def->flag + "' requires an argument";
                    i = -1; 
                    break;
                }
                m_args[def->flag].push_back(arg);
            }
        }
        else
        {
            // hit a non-argument item
            m_error = "unexpected argument: '";
            m_error.append(argv[i]);
            m_error.append("'");
            // do not alter the return value, wether this is an error
            // condition is up to the client
            break;
        }
    }

    // second pass, insert default values
    for (def = definition; def < definition + elems ; def++)
    {
        if (!def->defaultValue.empty())
        {
            // check if the parameter was specified on the command line
            if (!argumentPresent(def->flag))
            {
                m_args[def->flag].push_back(def->defaultValue);
            }
        }
    }

    // third pass, validate all required arguments are supplied
    for (def = definition; i > 0 && def < definition + elems ; def++)
    {
        if (def->required && !argumentPresent(def->flag))
        {
            // hit a non-argument item
            m_error = "missing required command line flag: '" + def->flag + "'";
            i = -1;
            break;
        }
    }

    // if error is set, append usage hint
    if (!m_error.empty())
    {
        m_error.append("\n('");
        m_error.append(argv[0]);
        m_error.append(" -h' for usage)\n");
    }
    
    return i;
}

std::string
APTArgParse::argument(const std::string& flag) const
{
    std::map<std::string, std::vector<std::string> >::const_iterator it;
    std::string rv;
    it = m_args.find(flag);
    if (it != m_args.end()) {
        unsigned int sz = it->second.size();
        if (sz > 0) rv = it->second[sz-1];
    }
    return rv;
}

std::vector<std::string>
APTArgParse::argumentValues(const std::string& flag) const
{
    std::vector<std::string> rval;
    std::map<std::string, std::vector<std::string> >::const_iterator it;
    it = m_args.find(flag);
    if (it != m_args.end()) rval = it->second;
    return rval;
}

bool
APTArgParse::argumentPresent(const std::string& flag) const
{
    std::map<std::string, std::vector<std::string> >::const_iterator it;
    it = m_args.find(flag);
    return (it == m_args.end()) ? false : true;
}

int
APTArgParse::argumentAsInteger(const std::string& flag) const
{
    if (!argumentPresent(flag)) return 0;
    return atoi(argument(flag).c_str());
}

std::string
APTArgParse::error() const
{
    return (m_error.empty()) ? "no error" : m_error;
}

std::string
APTArgParse::usage() const
{
    return m_usage;
}
