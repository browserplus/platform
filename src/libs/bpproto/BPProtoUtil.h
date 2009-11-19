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
 * BPProtoUtil.h -- custom little tools used by the corelet library.
 */

#ifndef __BPPROTOUTIL_H__
#define __BPPROTOUTIL_H__

#include "BPProtocolInterface.h"
#include <BPUtils/bpprocess.h>
#include <BPUtils/bptypeutil.h>
#include <ServiceAPI/bptypes.h>
#include <ServiceAPI/bpdefinition.h>



// free a pointer returned from drvToBPElement
void freeBPElement(BPElement * elem);

// map the results from describe into a bpdescription.
// Minimal memory copying is performed, so the resultant definition has
// pointers to data stored inside the bp::Object structure.  This means
// the BPDescription should not be used once the bp::Object is freed.
BPCoreletDefinition * objectToDefinition(const bp::Object * obj);

// free a pointer returned from drvToBPDescription
void freeDefinition(BPCoreletDefinition * definition);

// Attempt to start the daemon
bool startupDaemon(bp::process::spawnStatus& spawnStatus);

// map a protocol response into an error/verbose error pair
// the client must provide a reference to an output map which will
// be used for "extended errors"
void mapResponseToErrorCode(const bp::Object * obj,
                            BPErrorCode & ec,
                            std::string & errorString,
                            std::string & verboseError);

// log the contents of an extended error map.
void logExtendedErrorInfo(const std::string& sContext,
                          const bp::Map& bpMap);

// given a bp object, attempt to extract the error and verbose error
// into string buffers
void
extractExtendedError(const bp::Object * obj,
                     std::string &error, std::string &verboseError);


#endif
