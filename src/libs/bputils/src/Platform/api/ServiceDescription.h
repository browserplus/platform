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

/**
 * CoreletDescription
 *
 * A Class which describes the functionality provided by a corelet.
 */

#ifndef __SERVICEDESCRIPTION_H__
#define __SERVICEDESCRIPTION_H__

#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpserviceversion.h"
#include "ServiceAPI/bpdefinition.h" 

#include <list>
#include <map>

// default copy constructors and assignment operators sufficient

namespace bp { namespace service {

/**
 * an in memory representation of an argument to a function on a corelet
 */ 
class Argument
{
public:
    enum Type {
        None,
        Null,
        Boolean,
        Integer,
        Double,
        String,
        Map,
        List,
        CallBack,
        Path,
        Any
    };

    Argument();
    Argument(const char * name, Type type);
    ~Argument();

    std::string name() const;
    void setName(const char * name);

    Type type() const;
    void setType(Type type);

    /** get a string representation of the type */
    static const char * typeAsString(Type type);

    /** get a type from a string */
    static Type stringAsType(const char * str);

    bool required() const;
    void setRequired(bool required);

    std::string docString() const;
    void setDocString(const char * docString);    

    /** re-initialize the class */
    void clear();

    bool fromBPArgumentDefinition(const BPArgumentDefinition * bpcd);
private:
    std::string m_name;    
    std::string m_docString;
    Type m_type;
    bool m_required;
    
};

/**
 * an in memory representation of a function supported by a corelet
 */ 
class Function
{
public:
    Function();
    ~Function();    

    std::string name() const;
    void setName(const char * name);

    std::list<Argument> arguments() const;
    void setArguments(const std::list<Argument> & arguments);    

    bool getArgument(const char * name, Argument & oArg) const;

    std::string docString() const;
    void setDocString(const char * docString);    

    /** re-initialize the class */
    void clear();

    bool fromBPFunctionDefinition(const BPFunctionDefinition * bpcd);
private:
    std::string m_name;
    std::string m_docString;    
    std::list<Argument> m_arguments;
};

/**
 * an in memory description of a corelet's interface
 */ 
class Description
{
public:
    Description();
    ~Description();    

    std::string name() const;
    void setName(const char * name);

    /** get a string representation of major, minor, and
     *  micro joined with '.' */
    std::string versionString() const;

    /** get a string of the form: "name major.minor.version" */
    std::string nameVersionString() const;
    
    bp::ServiceVersion version() const;

    /** get corelet major version */
    unsigned int majorVersion() const;

    /** set corelet major version */
    void setMajorVersion(unsigned int majorVersion);    

    /** get corelet minor version */
    unsigned int minorVersion() const;
    /** set corelet minor version */
    void setMinorVersion(unsigned int minorVersion);    

    /** get corelet micro version */
    unsigned int microVersion() const;
    /** set corelet micro version */
    void setMicroVersion(unsigned int microVersion);    

    std::string docString() const;
    void setDocString(const char * docString);    

    std::list<Function> functions() const;
    void setFunctions(const std::list<Function> & functions);    

    bool hasFunction(const char * funcName) const;

    /** get the function description */
    bool getFunction(const char * funcName, Function & oFunc) const;

    /** generate a bp::Object representation of the corelet description.
     *  caller assumes ownership of returned value */
    bp::Object* toBPObject() const;
    
    /** populate the structure from a bp::Object representation.
     *  returns false on failure, true on success. */
    bool fromBPObject(const bp::Object* bp);

    /**
     * populate the structure from a BPCoreletDefinition memory structure
     * returns false on failure, true on success.
     */
    bool fromBPCoreletDefinition(const BPCoreletDefinition * bpcd);

    /** Does this corelet description describe a built in corelet? */
    bool isBuiltIn() const;
    void setIsBuiltIn(bool isBuiltIn);

    /** re-initialize the definition */
    void clear();

    /** generate a human readable buffer of the interface of the service */
    std::string toHumanReadableString() const;
private:
    std::string m_name;
    unsigned int m_majorVersion;    
    unsigned int m_minorVersion;    
    unsigned int m_microVersion;    
    std::string m_docString;
    std::list<Function> m_functions;
    // true for built in corelets, added using the
    // CoreletRegistry::registerCorelet() call
    bool m_builtIn;
};

/**
 * validate arguments given a bp::Map containing arguments and a
 * function description describing a function's interface.
 *
 * NOTE:  "arguments" may be modified, for example when converting double
 *                    arguments to integers (thanks, Safari).
 *
 * this function is used by both the daemon (for corelets) and the
 * plugin (for pluglets)
 *
 * returns non-empty string on failure, containing a verbose error message
 */
std::string validateArguments(const bp::service::Function & fdesc,
                              bp::Map* arguments);

std::string validateArguments(const bp::service::Function & desc,
                              bp::Map* arguments);
 
} }
    
#endif
