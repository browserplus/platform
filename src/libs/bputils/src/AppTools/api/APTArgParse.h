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
 * TODO: + integrate configuration file parsing
 *       + further validation features: integer args, enum args.
 *           
 */
#ifndef __APTARGPARSE_H__
#define __APTARGPARSE_H__

#include <string>
#include <map>
#include <vector>


/**
 * symbols that should be used when defining an APTArgDefintion
 * structure to improve readability
 */
namespace APT
{
    /** false value for APTArgDefinition::acceptsArgument */
    extern const bool NO_ARG;
    /** true value for APTArgDefinition::acceptsArgument */
    extern const bool TAKES_ARG;    
    /** empty value for APTArgDefinition::defaultValue */
    extern const char* NO_DEFAULT;
    /** false value for APTArgDefinition::required */
    extern const bool NOT_REQUIRED;
    /** true value for APTArgDefinition::required */
    extern const bool REQUIRED;    
    /** false value for APTArgDefinition::isInteger */
    extern const bool NOT_INTEGER;
    /** true value for APTArgDefinition::required */
    extern const bool IS_INTEGER;    
    /** false value for APTArgDefinition::allowMultipleOccurances */
    extern const bool MAY_NOT_RECUR;    
    /** true value for APTArgDefinition::allowMultipleOccurances */
    extern const bool MAY_RECUR;    
};


#ifdef WIN32
// tell windows compiler not to whine about ANSI C.
#pragma warning ( push )
#pragma warning ( disable : 4510 )
#pragma warning ( disable : 4512 )
#pragma warning ( disable : 4610 )
#endif

/** 
 * A structure which defines the allowed arguments
 */
struct APTArgDefinition
{
    /** the character that comes after the '-' */
    std::string flag;
    /** whether the flag is a boolean flag or it accepts an argument  */    
    bool acceptsArgument;
    /** the default value for the argument, empty specifies
     *  that there is no default. Only meaningful if acceptsArgument is
     *  true */
    const std::string defaultValue;
    /** wether the argument is required.  only meaningful if acceptsArgument
     *  is true. */
    bool required;
    /** instruct APTArgParse to validate that the argument is a valid
     *  integer.  only meaningful if acceptsArgument is true. */
    bool isInteger;
    /** allow the argument to occur multiple times */
    bool allowMultipleOccurances;
    /** documentation for the argument */
    std::string docString;
};

#ifdef WIN32
#pragma warning ( pop )
#endif


/**
 * A cross platform command parser providing the following features:
 *  - simple, terse usage
 *  - argument parsing and validation
 *  - automatic generation of usage
 *  - robust argument parsing, supporting multiple means of
 *    specifying arguments -xFOO -x FOO -x=FOO
 */
class APTArgParse
{
  public:
    /**
     * allocate an argument parser.
     *
     * \param programName - the name of the program, used to generate usage.
     * \param programDocumentation - brief summary of what the program
     *           does, used to generate usage.
     */
    APTArgParse(const std::string& programDocumentation);
    virtual ~APTArgParse();

    /** parse arguments from the command line, applying the definition.
     *  Note, APTArgParse will not generate an error if it cannot parse
     *  all command line arguments.  Many programs are designed so that
     *  they accept several flags, and after flags a paramter.  This
     *  is supported by the return value.  argc - return value gives you
     *  how many unparsed arguments are on the command line.
     *  for convenience, in this scenario the error is set to complain
     *  about the last argument on the command line...  So if you do
     *  not allow arguments other than option flags, the following
     *  code will handle all cases:
     *
     *  \code
     *  unsigned int rv = parser->parse(...);
     *  if (rv < 0 || rv != argc) {
     *    std::cout << parser->errorString() << std::endl;
     *    exit(1);
     *  }
     * \endcode
     *
     * If you do allow some non-option arguments, it's up to the client
     * to check that the correct number are available and generate an
     * error message.
     *
     * \param definition - array of APTArgDefinition structures
     * \param defsize - length of definition array
     *
     *  \return The number of arguments successfully parsed, or -1 on a
     *          parse error.
     *
     *
     * \warning 'h' is a flag reserved by the implementation for
     *          generation of usage
     */
    int parse(unsigned int defsize,
              const APTArgDefinition * definition,
              int argc, const char ** argv);

    /** fetch an argument by flag.  empty is returned if the flag is
     *  not defined.  If an argument is denoted required and has no
     *  default, parsing will fail.  If APTArgDefinition::allowMultipleOccurances
     *  was true and multiple arguments were specified on the command
     *  line, the last occurrence will be returned */
    std::string argument(const std::string& flag) const;

    /** fetch all values associated with an argument.  empty is
     *  returned if the flag is not defined.  If an argument is
     *  denoted required and has no default, parsing will fail unless
     *  the argument is present on the command line.  Thus it is not
     *  neccesary to check return codes */
    std::vector<std::string> argumentValues(const std::string& flag) const;

    /** Attain an argument as an integer.  If isInteger is specified
     *  in the APTArgDefinition, the argument will have been verified
     *  at parsing time. */ 
    int argumentAsInteger(const std::string& flag) const;

    /** return wether a flag was specified on the command line */ 
    bool argumentPresent(const std::string& flag) const;

    /** Attain the error string.  This is never empty (set to 'no error')
     *  if no error was encountered */
    std::string error() const;

    /** Attain the usage.  Built up at parsing time, so will return an empty
     *  string if APTArgParse::parse() has not been called. */
    std::string usage() const;
    
  private:
    std::string m_usage;
    std::string m_programDoc;
    std::string m_error;
    std::map<std::string, std::vector<std::string> > m_args;
};

#endif 
