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
 * A virtual class internal to the DistributionClient library
 */

#ifndef __DISTQUERYINTERNAL_H__
#define __DISTQUERYINTERNAL_H__

#include "BPUtils/ServiceDescription.h"
#include "DistributionClient/DistQueryTypes.h"

class ServiceQuery;

class IServiceQueryListener
{
  public:
    virtual ~IServiceQueryListener();

    virtual void onTransactionFailed(const ServiceQuery * cq,
                                     const std::string& msg) = 0;
    virtual void gotAvailableServices(const ServiceQuery * cq,
                                      const AvailableServiceList & list) = 0;    
    virtual void onServiceFound(const ServiceQuery * cq,
                                const AvailableService & list) = 0;    
    virtual void onDownloadProgress(const ServiceQuery * cq,
                                    unsigned int pct) = 0;    
    virtual void onDownloadComplete(const ServiceQuery * cq,
                                    const std::vector<unsigned char> & buf) = 0;    
    virtual void gotServiceDetails(const ServiceQuery * cq,
                                   const bp::service::Description & desc) = 0;    
    virtual void onRequirementsSatisfied(const ServiceQuery * cq,
                                         const ServiceList & clist) = 0;
    virtual void onCacheUpdated(const ServiceQuery * cq,
                                const ServiceList & updates) = 0;
    virtual void gotServiceSynopsis(const ServiceQuery * cq,
                                    const ServiceSynopsisList & sslist) = 0;
    virtual void gotLatestPlatformVersion(const ServiceQuery * cq,
                                          const std::string & latest) = 0;
    virtual void onLatestPlatformDownloaded(
        const ServiceQuery * cq,
        const LatestPlatformPkgAndVersion & pkgAndVersion) = 0;
};

#endif
