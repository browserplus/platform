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

#include "api/Fetcher.h"
#include "BPUtils/OS.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "platform_utils/ProductPaths.h"

using namespace std;
using namespace std::tr1;
namespace bpf = bp::file;

namespace bp {
    namespace install {

        //------------------------------------------

        Fetcher::Fetcher(const bpf::Path& keyPath,
                         const list<string>& distroServers,
                         const bpf::Path& destDir)
            : m_keyPath(keyPath), m_distroServers(distroServers), m_destDir(destDir),
              m_platformVersion(), m_platformSize(0), m_state(eIdle),
              m_totalServices(0), m_completedServices(0)
        {
            m_filter.reset(new MyServiceFilter);
            m_distQuery.reset(new DistQuery(m_distroServers, m_filter.get()));
            m_distQuery->setListener(this);
            // TODO: propogate log level and file to spawned processes
        }


        Fetcher::~Fetcher()
        {
        }


        unsigned int
        Fetcher::getPlatform()
        {
            m_state = eFetchingPlatform;
            m_tid = m_distQuery->downloadLatestPlatform(bp::os::PlatformAsString());
            return m_tid;
        }


        unsigned int
        Fetcher::getPlatformVersionAndSize()
        {
            m_state = eFetchingPlatformInfo;
            // TODO: should be "AndSize" when it's available
            m_tid = m_distQuery->latestPlatformVersion(bp::os::PlatformAsString());
            return m_tid;
        }

        unsigned int
        Fetcher::getServices(const list<ServiceRequireStatement>& services)
        {
            m_state = eFetchingRequirements;
    
            // See what we need.  Already installed services will be filtered
            // out in onRequirementsSatisfied()
            list<bp::service::Summary> installed;
            m_tid = m_distQuery->satisfyRequirements(bp::os::PlatformAsString(),
                                                     services, installed);
            return m_tid;
        }


        //  IDistQueryListener interface 

        void
        Fetcher::onTransactionFailed(unsigned int tid,
                                     const std::string& msg)
        {
            shared_ptr<IFetcherListener> l = m_listener.lock();
            if (l) {
                l->onTransactionFailed(m_tid, msg);
            }
            m_state = eIdle;
        }


        void
        Fetcher::onServiceFound(unsigned int tid,
                                const AvailableService& list)
        {
            // empty
        }


        void 
        Fetcher::onDownloadProgress(unsigned int tid,
                                    unsigned int pct) 
        {
            shared_ptr<IFetcherListener> l = m_listener.lock();
            if (l) {
                if (m_state == eFetchingPlatform) {
                    l->onDownloadProgress(m_tid, m_platformVersion, pct);
                } else if (m_state == eFetchingService) {
                    // rough scaling of pct to ensure progression from 1 to 100
                    // when multiple services are being fetched
                    double pctPerService = (100.0 / m_totalServices);
                    pct = (unsigned int)
                        (((pct / 100.0) * pctPerService) +
                         (m_completedServices * pctPerService));
                    pair<string, string> p = m_neededServices.front();
                    l->onDownloadProgress(m_tid, p.first, pct);
                }
            }
        }


        void 
        Fetcher::onDownloadComplete(unsigned int tid,
                                    const vector<unsigned char>& buf) 
        {
            shared_ptr<IFetcherListener> l = m_listener.lock();
            if (m_state == eFetchingService) {
                try {
                    pair<string, string> p = m_neededServices.front();
                    bpf::Path destDir = m_destDir / "services" / p.first / p.second;
                    string errMsg;
                    ServiceUnpacker unpacker(buf, m_keyPath);
                    BPLOG_DEBUG_STRM("unpack service to " << destDir);
                    if (!unpacker.unpackTo(destDir, errMsg)) {
                        BP_THROW(errMsg);
                    }
                    m_neededServices.pop_front();
                    m_completedServices++;
                    fetchNextService();
                } catch (const bp::error::Exception& e) {
                    if (l) {
                        l->onTransactionFailed(m_tid, e.what());
                    }
                } 
            }
        }


        void 
        Fetcher::onRequirementsSatisfied(unsigned int tid,
                                         const ServiceList& clist) 
        {
            // Run thru clist, only adding services that we don't
            // have to m_neededServices.  We check for the existence
            // of a service's manifest.json since if a running service
            // is removed, the directory and dll will still exist,
            // but the manifest file won't (since windows can't remove
            // open files or their containing dirs).
            bpf::Path serviceDir = bp::paths::getServiceDirectory();
            ServiceList::const_iterator it;
            for (it = clist.begin(); it != clist.end(); ++it) {
                bpf::Path path = serviceDir / it->first / it->second / "manifest.json";
                if (!bpf::exists(path)) {
                    m_neededServices.push_back(*it);
                }
            }
            m_totalServices = m_neededServices.size();
            fetchNextService();
        }


        void 
        Fetcher::gotLatestPlatformVersion(unsigned int tid,
                                          const string& latest) 
        {
            m_state = eIdle;
            m_platformVersion = latest;
            shared_ptr<IFetcherListener> l = m_listener.lock();
            if (l) {
                // TODO: should be in gotLatestPlatformVersionAndSize() when
                // TODO: distquery has that api
                l->onPlatformVersionAndSize(m_tid, m_platformVersion,
                                            m_platformSize);
            }
        }


        void 
        Fetcher::onLatestPlatformDownloaded(unsigned int tid,
                                            const LatestPlatformPkgAndVersion& info)
        {
            m_state = eIdle;
    
            shared_ptr<IFetcherListener> l = m_listener.lock();
            try {
                if (!m_platformVersion.empty() 
                    && m_platformVersion.compare(info.m_version)) {
                    BP_THROW("version changed between calls!");
                }
                PlatformUnpacker unpacker(info.m_pkg, m_destDir,
                                          info.m_version, m_keyPath);
                string errMsg;
                bool rval = (unpacker.unpack(errMsg) && unpacker.install(errMsg));
                if (l) {
                    if (rval) {
                        l->onPlatformDownloaded(m_tid);
                    } else {
                        BP_THROW(errMsg);
                    }
                }
            } catch (const bp::error::Exception& e) {
                if (l) {
                    l->onTransactionFailed(m_tid, e.what());
                }
            }
        }


        unsigned int 
        Fetcher::fetchNextService()
        {
            m_state = eFetchingService;
    
            if (!m_neededServices.empty()) {
                pair<string, string> p = m_neededServices.front();
                return m_distQuery->downloadService(p.first, p.second,
                                                    bp::os::PlatformAsString());
            } else {
                m_state = eIdle;
                shared_ptr<IFetcherListener> l = m_listener.lock();
                if (l) {
                    l->onServicesDownloaded(m_tid);
                }
            }
            return 0;
        }
    }
}



