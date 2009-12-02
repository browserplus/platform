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
 * Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
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

    virtual void onTransactionFailed(unsigned int tid) = 0;
    virtual void gotAvailableServices(unsigned int tid,
                                      const CoreletList & list);
    virtual void onServiceFound(unsigned int tid,
                                const AvailableCorelet & list);
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
                                         const CoreletList & clist);
    virtual void onCacheUpdated(unsigned int tid,
                                const CoreletList & updates);
    virtual void gotServiceSynopsis(unsigned int tid,
                                    const ServiceSynopsisList & sslist);
    virtual void gotLatestPlatformVersion(unsigned int tid,
                                          const std::string & latest);
    virtual void onLatestPlatformDownloaded(
        unsigned int tid,
        const LatestPlatformPkgAndVersion & pkgAndVersion);
};

class DistQuery : virtual public ICoreletQueryListener
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
     * attain the list of available corelets from the distribution
     * server.  After the call, either AvailableCoreletsEvent or
     * AvailableCoreletsFailedEvent will be posted, depending on the
     * success of the call.  AvailableCoreletsEvent has a CoreletList
     * as payload
     *
     * platform must be one of 'win32' or 'osx'
     *
     * returns zero on failure (missing require parameter)
     */
    unsigned int availableServices(const std::string & platform);

    /**
     * Find a corelet which matches the supplied name, version, and
     * minversion.
     *
     * upon success, a CoreletNameAndVersion object is included in
     * the event payload.
     */
    unsigned int findService(const std::string & name,
                             const std::string & version,
                             const std::string & minversion,
                             const std::string & platform);

    /**
     * Download a corelet.
     */
    unsigned int downloadCorelet(const std::string & name,
                                 const std::string & version,
                                 const std::string & platform);

    /**
     * get a corelet description from the distribution server.
     * returns a ServiceDescriptionPtr in payload
     * (see DistQueryTypes.h)
     */
    unsigned int coreletDetails(const std::string & name,
                                const std::string & version,
                                const std::string & platform);

    /**
     * attain permissions, includes:
     *  corelet blacklist
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
                                 const std::string & sUA);
    
    /**
     * Given a list of require statments, Generate a topographically
     * sorted list of corelets that satisfy these requirements statements,
     * and all nested requirements expressed by dependent corelets.
     *
     * the event payload of SatisfyRequirementsEvent is a reverse
     * topologically sorted CoreletList (see DistQueryTypes.h)
     */
    unsigned int satisfyRequirements(
        const std::string & platform,
        const std::list<CoreletRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installedCorelets);

    /**
     * update the corelet cache, downloading any corelets that satisfy
     * the provided require statements and are newer than what is installed
     * or in cache.
     *
     * The event payload of CacheUpdatedEvent is a CoreletList
     * (see DistQueryTypes.h) with what corelets were installed into the
     * perding update cache.
     */
    unsigned int updateCache(
        const std::string & platform,
        const std::list<CoreletRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installedCorelets);

    /**
     * synchronously purge all corelets in the corelet cache
     */
    void purgeCache();

    /**
     * synchronously install a corelet from the corelet cache
     */
    bool installCoreletFromCache(const std::string & name,
                                 const std::string & version);

    /**
     * list what is available in the cache
     */
    CoreletList cachedCorelets();

    /**
     * check if a corelet is available in the cache.
     */
    bool isCached(const std::string & name,
                  const std::string & version);

    /**
     * Given a set of requirements, and a set of corelets that satisfy
     * those requirements, are there any newer corelets in cache that
     * _better_ satisfy the requirements.
     */
    CoreletList haveUpdates(
        const std::list<CoreletRequireStatement> &requirements,
        const std::list<bp::service::Summary> & installed);

    /**
     * Given a set of name/version pairs (either on distro server or in cache),
     * attain synopsis of the corelets with information suitable for
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
                                 const CoreletList & corelets);

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
            ListCorelets, CoreletDetails, Permissions,
            Download, FindCorelet, ReportPageUsage, SatisfyRequirements,
            UpdateCache, AttainServiceSynopses, DownloadLatestPlatform,
            LatestPlatformVersion, None
        } Type;
    
        DistQuery& m_owner;
        Type m_type;
        unsigned int m_tid;
        bp::time::Stopwatch m_stopWatch;
        // only used for permissions requests and page usage,
        // all others use CoreletQuery 
        std::tr1::shared_ptr<bp::http::client::Transaction> m_transaction;

        /* CoreletQuery object */
        std::tr1::shared_ptr<class CoreletQuery> m_coreletQuery;
      private:
        // no copy/assignment semantics
        TransactionContext(const TransactionContext&);
        TransactionContext& operator=(const TransactionContext&);
    };
    typedef std::tr1::shared_ptr<TransactionContext> TransactionContextPtr;

    // outstanding transactions
    std::list<TransactionContextPtr> m_transactions;

    TransactionContextPtr allocTransaction(TransactionContext::Type t);
    TransactionContextPtr findTransactionByCoreletQuery(const void * cq);

    void removeTransaction(unsigned int tid);

    // used to assign unique ids to each transaction
    unsigned int m_tid;

    const IServiceFilter * m_serviceFilter;

    virtual void onTransactionFailed(const CoreletQuery * cq);
    virtual void gotAvailableServices(const CoreletQuery * cq,
                                      const AvailableCoreletList & list);    
    virtual void onServiceFound(const CoreletQuery * cq,
                                const AvailableCorelet & list);    
    virtual void onDownloadProgress(const CoreletQuery * cq,
                                    unsigned int pct);    
    virtual void onDownloadComplete(const CoreletQuery * cq,
                                    const std::vector<unsigned char> & buf);    
    virtual void gotServiceDetails(const CoreletQuery * cq,
                                   const bp::service::Description & desc);    
    virtual void onRequirementsSatisfied(const CoreletQuery * cq,
                                         const CoreletList & clist);
    virtual void onCacheUpdated(const CoreletQuery * cq,
                                const CoreletList & updates);
    virtual void gotServiceSynopsis(const CoreletQuery * cq,
                                    const ServiceSynopsisList & sslist);
    virtual void gotLatestPlatformVersion(const CoreletQuery * cq,
                                          const std::string & latest);
    virtual void onLatestPlatformDownloaded(
        const CoreletQuery * cq,
        const LatestPlatformPkgAndVersion & pkgAndVersion);

    IDistQueryListener * m_listener;
};

#endif
