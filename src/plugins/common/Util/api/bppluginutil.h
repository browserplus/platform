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

/************************************************************************
* bppluginutil.h
*
* Written by Gordon Durand, Thu 20 Sept 2007
* (c) 2007, Yahoo! Inc, all rights reserved.
*/

#ifndef __BPPLUGINUTIL_H__
#define __BPPLUGINUTIL_H__

#include <set>
#include <vector>
#include <string>

#include "BPProtocol/BPProtocol.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpfile.h"



namespace bp {
namespace pluginutil {

    /**
     * append the corelet name/version pairs returned from BPEnumerate
     * onto a list
     * \returns false if the list is malformed.  true on success.
     */
    bool appendEnumerateResultsToList(const BPElement * corelets,
                                      bp::List &serviceList);

    // traverse a return value from BPCore.  perform handle obfuscation
    bool toBrowserSafeRep( const bp::Object* input, bp::Object*& output );
    
    
    // flags for applyFilters, OR them together as needed
    static const unsigned int kRecurse = 0x01;
    static const unsigned int kIncludeGestureInfo = 0x02;
    
    // apply filtering and recursion to a selection, returning the 
    // approprate bp::Object.  Caller assumes ownership.
    bp::Object* applyFilters(const std::vector<bp::file::Path>& selection,
                             const std::set<std::string>& mimeTypes,
                             unsigned int flags,
                             unsigned int limit);    


    // get the build type string from BrowserPlus.config
    std::string getBuildType();
}
}


#endif
