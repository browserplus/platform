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
 * ServiceQueryUtil - logic to perform service matching, and determine
 *                    hot to satisfy requirements.
 */

#ifndef __SERVICEQUERYUTIL_H__
#define __SERVICEQUERYUTIL_H__

#include "ServiceQuery.h"
#include "DistQueryTypes.h"
//#include "BPUtils/BPUtils.h"

namespace ServiceQueryUtil {
    bool findBestMatch(std::string name,
                       std::string versionStr,
                       std::string minversionStr,
                       const AvailableServiceList & list,
                       AvailableService & oMatch);

    // get a (reverse) topologically sorted list of services that
    // satisfy the given requirements
    bool findSatisfyingServices(
        const std::list<ServiceRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installed,
        const AvailableServiceList & available, bool wantNewest,
        AvailableServiceList & oNeed);

    // given the set of requirements, satisfied by the provided list of
    // installed services, are there services on the 'updates' list that
    // are newer and would still satisfy these requirements?
    bool haveUpdates(
        const std::list<ServiceRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installed,
        const std::list<bp::service::Summary> & updates,
        AvailableServiceList & oBest);

    // reformat the internal representation of a available service list
    // to something we send out to the client
    ServiceList reformatAvailableServiceList(
        const AvailableServiceList & list);

    // reformat a service summary pointer into an ACLPtr 
    AvailableServiceList
        serviceSummaryToACL(const std::list<bp::service::Summary> & cs);
};

#endif
