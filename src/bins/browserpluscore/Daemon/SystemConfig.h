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

#ifndef __SYSTEMCONFIG_H__
#define __SYSTEMCONFIG_H__

#include "BPUtils/bptypeutil.h"

namespace SystemConfig 
{
    // get an object representing system state or configuration given a
    // string key.  The "help" key returns a list of supported keys.
    // new keys are implemented inside SystemConfig.cpp
    bp::Object * getState(const char * locale,
                          const char * key);
    
    // set system state or configuration given a key and an optional argument.
    // Return value is a bp::Boolean indicating success/failure.
    // The "help" key returns a list of supported keys.
    // new keys are implemented inside SystemConfig.cpp
    bp::Object * setState(const char * key,
                          const bp::Object * arg);
};

#endif
