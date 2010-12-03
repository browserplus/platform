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

#include <string>
#include "DistributionClient/DistributionClient.h"
#include "ServiceManager/ServiceManager.h"
#include "BPUtils/bpfile.h"

namespace bp {
namespace install {

// Asynchronously interact with DistQuery to download platforms and services

class IFetcherListener
{ 
 public:
    virtual ~IFetcherListener() {}
    virtual void onTransactionFailed(unsigned int tid,
                                     const std::string& msg = "") = 0;
    virtual void onDownloadProgress(unsigned int tid,
                                    const std::string& item,
                                    unsigned int pct) = 0;
    virtual void onServicesDownloaded(unsigned int tid) = 0;
    virtual void onPlatformDownloaded(unsigned int tid) = 0;
    virtual void onPlatformVersionAndSize(unsigned int tid,
                                          const std::string& version,
                                          size_t size) = 0;
};


class Fetcher : virtual public IDistQueryListener
{ 
 public:
    class MyServiceFilter : public virtual IServiceFilter {
      public:
        virtual bool serviceMayRun(const std::string&,
                                   const std::string&) const { return true; }
        virtual ~MyServiceFilter() {}
    };

    Fetcher(const boost::filesystem::path& keyPath,
		    const std::list<std::string>& distroServers,
            const boost::filesystem::path& destDir);

    virtual ~Fetcher();
    
    virtual void setListener(std::tr1::weak_ptr<IFetcherListener> listener) {
        m_listener = listener;
    }
    
    // Get latest platform version and size
    virtual unsigned int getPlatformVersionAndSize();

    // Get and "install" platform to destination dir.
    virtual unsigned int getPlatform();

    // Get and "install" services to destination dir.
    virtual unsigned int getServices(const std::list<ServiceRequireStatement>& services);
    
    virtual std::string platformVersion() const {
        return m_platformVersion;
    }

    virtual size_t platformSize() const {
        return m_platformSize;
    }

    // IDistQueryListener interface
    //
    virtual void onTransactionFailed(unsigned int tid,
                                     const std::string& msg);
    virtual void onServiceFound(unsigned int tid,
                                const AvailableService& list);
    virtual void onDownloadProgress(unsigned int tid,
                                    unsigned int pct);
    virtual void onDownloadComplete(unsigned int tid,
                                    const std::vector<unsigned char>& buf);
    virtual void onRequirementsSatisfied(unsigned int tid,
                                         const ServiceList& clist);
    virtual void gotLatestPlatformVersion(unsigned int tid,
                                          const std::string& latest);
    virtual void onLatestPlatformDownloaded(unsigned int tid,
                                            const LatestPlatformPkgAndVersion& info);
    
 protected:
    unsigned int fetchNextService();
    
    boost::filesystem::path m_keyPath;
    std::list<std::string> m_distroServers;
    boost::filesystem::path m_destDir;
    std::string m_platformVersion;
    size_t m_platformSize;
    std::list<ServiceRequireStatement> m_requires;
    ServiceList m_neededServices;
    boost::scoped_ptr<MyServiceFilter> m_filter;
    boost::scoped_ptr<DistQuery> m_distQuery;
    std::tr1::weak_ptr<IFetcherListener> m_listener;
    enum {
        eIdle,
        eFetchingPlatformInfo,
        eFetchingPlatform,
        eFetchingRequirements,
        eFetchingService
    } m_state;
    unsigned int m_tid;
    // state used for rough progress scaling in eFetchingService case
    unsigned int m_totalServices;
    unsigned int m_completedServices;
};

}}
