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
 * Written by Lloyd Hilaiel, on or around Fri May 18 17:06:54 MDT 2007 
 *
 * A service is defined by a set if in-memory structures.  These
 * structures are hierarchical and can define a service containing
 * any number of functions, which themselves accept any number of
 * arguments.
 */

#ifndef __BPDEFINITION_V4_H__
#define __BPDEFINITION_V4_H__

#include "v4_bptypes.h"
#include "v4_bperror.h"

namespace sapi_v4 {

/** The definition of an argument */
typedef struct {
    /** The name of the agument, all arguments in BrowserPlus are named */
    BPString name;
    /** A human readable english documentation string */
    BPString docString;
    /** The expected type of the argument */
    BPType   type;
    /** whether the argument is required or not */
    BPBool   required;
} BPArgumentDefinition;

/** The definition of a function */
typedef struct {
    /** the name of the function */
    BPString functionName;
    /** A human readable english documentation string. */
    BPString docString;
    /** The number of arguements in the arguments array */
    unsigned int numArguments;
    /** An array of argument definitions */
    BPArgumentDefinition * arguments;
} BPFunctionDefinition;

/** The definition of a service */
typedef struct BPCoreletDefinition_t {
    /** The name of the service */
    BPString coreletName;
    /** The major version of the service */
    unsigned int majorVersion;
    /** The minor version of the service */
    unsigned int minorVersion;
    /** The micro version (build number) of the service */
    unsigned int microVersion;
    /** A human readable english documentation string. */
    BPString docString;
    /** The number of functions in the functions array */
    unsigned int numFunctions;
    /** An array of function definitions */
    BPFunctionDefinition * functions;
} BPCoreletDefinition;

};

#endif
