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
 * CoreletQueryUtil - logic to perform corelet matching, and determine
 *                    hot to satisfy requirements.
 */

#ifndef __CORELETQUERYUTIL_H__
#define __CORELETQUERYUTIL_H__

#include "CoreletQuery.h"
#include "DistQueryTypes.h"
//#include "BPUtils/BPUtils.h"

namespace CoreletQueryUtil {
    bool findBestMatch(std::string name,
                       std::string versionStr,
                       std::string minversionStr,
                       const AvailableCoreletList & list,
                       AvailableCorelet & oMatch);

    // get a (reverse) topologically sorted list of corelets that
    // satisfy the given requirements
    bool findSatisfyingCorelets(
        const std::list<CoreletRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installed,
        const AvailableCoreletList & available, bool wantNewest,
        AvailableCoreletList & oNeed);

    // given the set of requirements, satisfied by the provided list of
    // installed corelets, are there corelets on the 'updates' list that
    // are newer and would still satisfy these requirements?
    bool haveUpdates(
        const std::list<CoreletRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installed,
        const std::list<bp::service::Summary> & updates,
        AvailableCoreletList & oBest);

    // reformat the internal representation of a available corelet list
    // to something we send out to the client
    CoreletList reformatAvailableCoreletList(
        const AvailableCoreletList & list);

    // reformat a corelet summary pointer into an ACLPtr 
    AvailableCoreletList
        coreletSummaryToACL(const std::list<bp::service::Summary> & cs);
};

#endif
