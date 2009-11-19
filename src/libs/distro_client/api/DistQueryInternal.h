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
 * A virtual class internal to the DistributionClient library
 */

#ifndef __DISTQUERYINTERNAL_H__
#define __DISTQUERYINTERNAL_H__

#include "BPUtils/ServiceDescription.h"
#include "DistributionClient/DistQueryTypes.h"

class CoreletQuery;

class ICoreletQueryListener
{
  public:
    virtual ~ICoreletQueryListener();

    virtual void onTransactionFailed(const CoreletQuery * cq) = 0;
    virtual void gotAvailableServices(const CoreletQuery * cq,
                                      const AvailableCoreletList & list) = 0;    
    virtual void onServiceFound(const CoreletQuery * cq,
                                const AvailableCorelet & list) = 0;    
    virtual void onDownloadProgress(const CoreletQuery * cq,
                                    unsigned int pct) = 0;    
    virtual void onDownloadComplete(const CoreletQuery * cq,
                                    const std::vector<unsigned char> & buf) = 0;    
    virtual void gotServiceDetails(const CoreletQuery * cq,
                                   const bp::service::Description & desc) = 0;    
    virtual void onRequirementsSatisfied(const CoreletQuery * cq,
                                         const CoreletList & clist) = 0;
    virtual void onCacheUpdated(const CoreletQuery * cq,
                                const CoreletList & updates) = 0;
    virtual void gotServiceSynopsis(const CoreletQuery * cq,
                                    const ServiceSynopsisList & sslist) = 0;
    virtual void gotLatestPlatformVersion(const CoreletQuery * cq,
                                          const std::string & latest) = 0;
    virtual void onLatestPlatformDownloaded(
        const CoreletQuery * cq,
        const LatestPlatformPkgAndVersion & pkgAndVersion) = 0;
};

#endif
