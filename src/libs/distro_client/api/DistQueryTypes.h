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
 * DistQueryTypes - Various data structures which are returned by the
 *                  DistQuery library
 */

#ifndef __DISTQUERYTYPES_H__
#define __DISTQUERYTYPES_H__

#include <list>
#include <map>
#include <vector>
#include <set>
#include <string>

#include "BPUtils/bpsemanticversion.h"

/**
 * a smart pointer container for a list of service name, version pairs.
 * used as event payload
 */
typedef std::list<std::pair<std::string, std::string> > ServiceList;

/**
 * a service name and version.
 */
class AvailableService
{
  public:
    AvailableService() : sizeBytes(0), serviceAPIVersion(0), dependentService(false) { }

    std::string name;
    bp::SemanticVersion version;
    std::string serverURL;    
    unsigned int sizeBytes;
    unsigned int serviceAPIVersion;

    bool dependentService;
    std::string providerName;
    std::string providerMinversion;
    std::string providerVersion;
};

/** operator< for strict weak ordering, stl support.  */
bool operator<(const AvailableService& lhs, const AvailableService& rhs);

typedef std::list<AvailableService> AvailableServiceList;

/**
 * a memory representation of a require statement
 */
class ServiceRequireStatement
{
  public:
    std::string m_name;
    std::string m_version;
    std::string m_minversion;
};

/**
 * representation of a service description along with localized
 * title and description 
 */
class ServiceSynopsis
{
  public:
    ServiceSynopsis()
        : m_sizeInBytes(0), m_isUpdate(false), m_permissions() { }
  
    std::string m_name;
    std::string m_version;
    std::string m_title;
    std::string m_summary;

    /* The size, in bytes, of data that will need to be downloaded for
     * this service to be installed.  This will be zero for cached updates */
    unsigned int m_sizeInBytes;

    bool m_isUpdate;

    // the permissions required by this service.
    std::set<std::string> m_permissions;
};

/**
 * A list of ServiceSynopsis
 */
typedef std::list<ServiceSynopsis> ServiceSynopsisList;

/**
 * A platform version
 */
class LatestPlatformPkgAndVersion
{
  public:
    std::string m_version;
    std::vector<unsigned char> m_pkg;
};

#endif
