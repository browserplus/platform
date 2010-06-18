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
 * DistQuery - A class which abstracts client code from the web services
 *             protocol to communicate with the BrowserPlus distribution
 *             server.
 */

#ifndef __DISTQUERY_H__
#define __DISTQUERY_H__

#include <map>
#include <vector>
#include <string>

#include "BPUtils/bpstopwatch.h"
#include "BPUtils/HttpListener.h"
#include "BPUtils/HttpTransaction.h"
#include "BPUtils/ServiceSummary.h"

#include "DistributionClient/DistQueryTypes.h"
#include "DistributionClient/DistQueryInternal.h"

// a service filter is a class that will filter the set of results
// from the distribution server.  This is useful for things like
// blacklists
class IServiceFilter
{
  public:
    virtual bool serviceMayRun(const std::string& name,
                               const std::string& version) const = 0;
    virtual ~IServiceFilter() { }
};


class IDistQueryListener
{
  public:
    virtual ~IDistQueryListener() { }

    virtual void onTransactionFailed(unsigned int tid,
                                     const std::string& msg) = 0;
    virtual void gotAvailableServices(unsigned int tid,
                                      const ServiceList & list);
    virtual void onServiceFound(unsigned int tid,
                                const AvailableService & list);
    virtual void onDownloadProgress(unsigned int tid,
                                    unsigned int pct);
    virtual void onDownloadComplete(unsigned int tid,
                                    const std::vector<unsigned char> & buf);
    virtual void gotServiceDetails(unsigned int tid,
                                   const bp::service::Description & desc);
    virtual void gotPermissions(unsigned int tid,
                                std::vector<unsigned char> permBundle);
    virtual void onPageUsageReported(unsigned int tid);
    virtual void onRequirementsSatisfied(unsigned int tid,
                                         const ServiceList & clist);
    virtual void onCacheUpdated(unsigned int tid,
                                const ServiceList & updates);
    virtual void gotServiceSynopsis(unsigned int tid,
                                    const ServiceSynopsisList & sslist);
    virtual void gotLatestPlatformVersion(unsigned int tid,
                                          const std::string & latest);
    virtual void onLatestPlatformDownloaded(
        unsigned int tid,
        const LatestPlatformPkgAndVersion & pkgAndVersion);
};

class DistQuery : virtual public IServiceQueryListener
{
  public:
    /**
     * allocate a DistQuery object.
     * \param baseURL the baseURL to the distribution server web services
     *                api
     */ 
    DistQuery(std::list<std::string> distributionServers,
              const IServiceFilter * serviceFilter);

    void setListener(IDistQueryListener * listener);
    
    virtual ~DistQuery();

    /**
     * attain the list of available services from the distribution
     * server.  After the call, either AvailableServicesEvent or
     * AvailableServicesFailedEvent will be posted, depending on the
     * success of the call.  AvailableServicesEvent has a ServiceList
     * as payload
     *
     * platform must be one of 'win32' or 'osx'
     *
     * returns zero on failure (missing require parameter)
     */
    unsigned int availableServices(const std::string & platform);

    /**
     * Find a service which matches the supplied name, version, and
     * minversion.
     *
     * upon success, a ServiceNameAndVersion object is included in
     * the event payload.
     */
    unsigned int findService(const std::string & name,
                             const std::string & version,
                             const std::string & minversion,
                             const std::string & platform);

    /**
     * Download a service.
     */
    unsigned int downloadService(const std::string & name,
                                 const std::string & version,
                                 const std::string & platform);

    /**
     * get a service description from the distribution server.
     * returns a ServiceDescriptionPtr in payload
     * (see DistQueryTypes.h)
     */
    unsigned int serviceDetails(const std::string & name,
                                const std::string & version,
                                const std::string & platform);

    /**
     * attain permissions, includes:
     *  service blacklist
     *  platform blacklist
     */
    unsigned int permissions();
    
    /**
     * report page usage
     * return payload is currently just a string "ok"
     **/
    unsigned int reportPageUsage(const std::string & sOSVersion,
                                 const std::string & sBPPlatform,
                                 const std::string & sURL,
                                 const std::string & sID,
                                 const std::string & sUA,
                                 const std::string & sServices);
    
    /**
     * report new installs
     * return payload is currently just a string "ok"
     **/
    unsigned int reportInstall(const std::string & sOSVersion,
                               const std::string & sBPPlatform,
                               const std::string & sID);

    /**
     * Given a list of require statments, Generate a topographically
     * sorted list of services that satisfy these requirements statements,
     * and all nested requirements expressed by dependent services.
     *
     * the event payload of SatisfyRequirementsEvent is a reverse
     * topologically sorted ServiceList (see DistQueryTypes.h)
     */
    unsigned int satisfyRequirements(
        const std::string & platform,
        const std::list<ServiceRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installedServices);

    /**
     * update the service cache, downloading any services that satisfy
     * the provided require statements and are newer than what is installed
     * or in cache.
     *
     * The event payload of CacheUpdatedEvent is a ServiceList
     * (see DistQueryTypes.h) with what services were installed into the
     * perding update cache.
     */
    unsigned int updateCache(
        const std::string & platform,
        const std::list<ServiceRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installedServices);

    /**
     * synchronously purge all services in the service cache
     */
    void purgeCache();

    /**
     * synchronously install a service from the service cache
     */
    bool installServiceFromCache(const std::string & name,
                                 const std::string & version);

    /**
     * list what is available in the cache
     */
    ServiceList cachedServices();

    /**
     * check if a service is available in the cache.
     */
    bool isCached(const std::string & name,
                  const std::string & version);

    /**
     * Given a set of requirements, and a set of services that satisfy
     * those requirements, are there any newer services in cache that
     * _better_ satisfy the requirements.
     */
    ServiceList haveUpdates(
        const std::list<ServiceRequireStatement> &requirements,
        const std::list<bp::service::Summary> & installed);

    /**
     * Given a set of name/version pairs (either on distro server or in cache),
     * attain synopsis of the services with information suitable for
     * display to end user.
     *
     * A synopsis is represented as a ServiceSynopsis (declared in
     * DistQueryTypes.h).
     * 
     * Once complete, a SynopsesAvailableEvent will be raised
     * containing ServiceSynopsesList (see DistQueryTypes.h)
     */
    unsigned int serviceSynopses(const std::string & platform,
                                 const std::string & locale,
                                 const ServiceList & services);

    /**
     * Get the latest available platform version.
     *
     * LatestPlatformVersionEvent contains a LatestPlatformVersionPtr
     * (see DistQueryTypes.h)
     */
    unsigned int latestPlatformVersion(const std::string & platform);

    /**
     * Download an bpkg of the latest BrowserPlus platform version
     * available on the distribution server.
     *
     * onDownloadProgress() will be invoked to report download progress
     * with the correct tid.
     *
     * PlatformDownloadedEvent contains a LatestPlatformPkgAndVersion
     * contains the content (bpkg) and platform version actually downloaded.
     * (see DistQueryTypes.h)
     */
    unsigned int downloadLatestPlatform(const std::string & platform);

  private:
    std::list<std::string> m_distroServers;

    class TransactionContext : public bp::http::client::Listener {
      public:
        TransactionContext(DistQuery& owner);
        ~TransactionContext();        
       
        // overrides from Listener 
        virtual void onResponseStatus(const bp::http::Status& status,
                                      const bp::http::Headers& headers);
        virtual void onClosed();
        virtual void onTimeout();
        virtual void onCancel();
        virtual void onError(const std::string& msg);

        void logTransactionCompletion(bool success);

        typedef enum {
            ListServices, ServiceDetails, Permissions,
            Download, FindService, ReportPageUsage, SatisfyRequirements,
            UpdateCache, AttainServiceSynopses, DownloadLatestPlatform,
            LatestPlatformVersion, None
        } Type;
    
        DistQuery& m_owner;
        Type m_type;
        unsigned int m_tid;
        bp::time::Stopwatch m_stopWatch;
        // only used for permissions requests and page usage,
        // all others use ServiceQuery 
        std::tr1::shared_ptr<bp::http::client::Transaction> m_transaction;

        /* ServiceQuery object */
        std::tr1::shared_ptr<class ServiceQuery> m_serviceQuery;
      private:
        // no copy/assignment semantics
        TransactionContext(const TransactionContext&);
        TransactionContext& operator=(const TransactionContext&);
    };
    typedef std::tr1::shared_ptr<TransactionContext> TransactionContextPtr;

    // outstanding transactions
    std::list<TransactionContextPtr> m_transactions;

    TransactionContextPtr allocTransaction(TransactionContext::Type t);
    TransactionContextPtr findTransactionByServiceQuery(const void * cq);

    void removeTransaction(unsigned int tid);

    // used to assign unique ids to each transaction
    unsigned int m_tid;

    const IServiceFilter * m_serviceFilter;

    virtual void onTransactionFailed(const ServiceQuery * cq,
                                     const std::string& msg);
    virtual void gotAvailableServices(const ServiceQuery * cq,
                                      const AvailableServiceList & list);    
    virtual void onServiceFound(const ServiceQuery * cq,
                                const AvailableService & list);    
    virtual void onDownloadProgress(const ServiceQuery * cq,
                                    unsigned int pct);    
    virtual void onDownloadComplete(const ServiceQuery * cq,
                                    const std::vector<unsigned char> & buf);    
    virtual void gotServiceDetails(const ServiceQuery * cq,
                                   const bp::service::Description & desc);    
    virtual void onRequirementsSatisfied(const ServiceQuery * cq,
                                         const ServiceList & clist);
    virtual void onCacheUpdated(const ServiceQuery * cq,
                                const ServiceList & updates);
    virtual void gotServiceSynopsis(const ServiceQuery * cq,
                                    const ServiceSynopsisList & sslist);
    virtual void gotLatestPlatformVersion(const ServiceQuery * cq,
                                          const std::string & latest);
    virtual void onLatestPlatformDownloaded(
        const ServiceQuery * cq,
        const LatestPlatformPkgAndVersion & pkgAndVersion);

    IDistQueryListener * m_listener;
};

#endif
