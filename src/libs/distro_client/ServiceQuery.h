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
 * ServiceQuery - A class capable of querying multiple distribution servers
 *                to find and attain services.
 */

#ifndef __SERVICEQUERY_H__
#define __SERVICEQUERY_H__

#include "DistQueryInternal.h"
#include "DistQueryTypes.h"
#include "platform_utils/ServiceSummary.h"
#include "QueryCache.h"


class ServiceQuery : public bp::http::client::Listener,
                     public IQueryCacheListener,
                     public std::tr1::enable_shared_from_this<ServiceQuery>,
                     public bp::thread::HoppingClass
{
  public:
    ServiceQuery(std::list<std::string> serverURLs,
                 const IServiceFilter * serviceFilter);
    ~ServiceQuery();

    void setListener(IServiceQueryListener * listener);

    /** override from bp::http::client::Listener
    */
    virtual void onResponseStatus(const bp::http::Status& status,
                                  const bp::http::Headers& headers);
    virtual void onResponseBodyBytes(const unsigned char* pBytes, 
                                     unsigned int size);
    virtual void onClosed();
    virtual void onTimeout();
    virtual void onCancel();
    virtual void onError(const std::string& msg);

    /** query available services, optionally for a specific platform,
     *  event data is an AvailableServiceList object */
    void availableServices(std::string platform);

    void findService(std::string name, std::string version,
                     std::string minversion, std::string platform);

    void downloadService(std::string name, std::string version,
                         std::string platform);

    void serviceDetails(std::string name, std::string version,
                        std::string platform);

    void satisfyRequirements(
        std::string platform,
        const std::list<ServiceRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installed);

    void updateCache(
        std::string platform,
        const std::list<ServiceRequireStatement> & requirements,
        const std::list<bp::service::Summary> & installedServices);

    void serviceSynopses(
        const std::string & platform,
        const std::string & locale,
        const ServiceList & services);

    void latestPlatformVersion(std::string platform);

    void downloadLatestPlatform(std::string platform);

  private:
    QueryCache m_qc;
    enum {
        None,
        AvailableServices,
        ServiceDetails,
        Download,
        FindService,
        SatisfyRequirements,
        UpdateCache,
        AttainServiceSynopses,
        LatestPlatformVersion,
        DownloadLatestPlatform
    } m_type;

    std::string m_locale;
    std::string m_name;
    std::string m_version;
    std::string m_minversion;
    std::string m_platform;
    std::list<ServiceRequireStatement> m_requirements;
    std::list<bp::service::Summary> m_installed;
    bool m_wantNewest;

    // updates to install, only pertinent in UpdateCache
    AvailableServiceList m_updates;
    // current update to install
    std::list<AvailableService>::iterator m_currentUpdate;

    // pertinent for for LocalizeDescriptions, we move items off the
    // m_serviceList and onto the m_locDescs list.
    std::list<std::pair<std::string, std::string> > m_serviceList;
    std::list<ServiceSynopsis> m_locDescs;
    // < used to figure out which server to query for localized descriptions
    AvailableServiceList m_services; 

    void fetchLocalization(const AvailableService & acp);
    void fetchDetails(const AvailableService & acp);
    void startDownload(const AvailableService & acp);
    void getNextLocalization();
    void parseLocalization(const unsigned char* buf, size_t len);

    bp::http::client::TransactionPtr m_httpTransaction;

    // used for downloads
    std::vector<unsigned char> m_cletBuf;
    unsigned int m_dlSize;
    unsigned int m_lastPct;
    bool m_zeroPctSent;

    // implementation of IQueryCacheListener
    void onServiceList(const AvailableServiceList & list);
    void onServiceListFailure();    
    void onLatestPlatform(const LatestPlatformServerAndVersion & latest);
    void onLatestPlatformFailure();    

    const IServiceFilter * m_serviceFilter;

    IServiceQueryListener * m_listener;

    // invoke listener failure callback if listener is defined
    void transactionFailed(const std::string& msg);

    // used for localizations to gaurantee we don't invoke client's callback
    // before function return.
    void onHop(void *);
};

#endif
