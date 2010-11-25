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

#include "ServiceInstaller.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/OS.h"
#include "ServiceManager/ServiceManager.h"
#include "DistributionClient/DistributionClient.h"
#include "Permissions/Permissions.h"

using namespace std;
using namespace std::tr1;


#define TMPDIR_PREFIX "BrowserPlus"

// the class that does all the asynchronous work of installing a service
class SingleServiceInstaller : virtual public IDistQueryListener
{
public:
    // Constructor for installing from dist server
    SingleServiceInstaller(
        std::list<std::string> distroServers,
        const std::string & name,
        const std::string & version,
        unsigned int iid,
        weak_ptr<ServiceInstaller::IListener> listener);
    
    // constructor for installing from bpkg buffer
    SingleServiceInstaller(
        const std::string & name,
        const std::string & version,
        const std::vector<unsigned char> & buffer,
        unsigned int iid,
        weak_ptr<ServiceInstaller::IListener> listener);

    void start();

    ~SingleServiceInstaller();

    // add a listener to be notified when this installation completes,
    // and return the installation id
    unsigned int addListener(
        weak_ptr<ServiceInstaller::IListener> listener);

    std::string m_name;
    std::string m_version; 
    
private:
    // implementation of methods from IDistQueryListener
    virtual void onTransactionFailed(unsigned int tid,
                                     const std::string& msg);
    virtual void onDownloadProgress(unsigned int tid, unsigned int pct);
    virtual void onDownloadComplete(unsigned int tid,
                                    const std::vector<unsigned char> & buf);

    void progressUpdateToAllListeners(unsigned int progressPct);
    void postToAllListeners(bool success);
    bool installService(const std::vector<unsigned char> buf);
    void removeSelfFromQueue();
    std::list<weak_ptr<ServiceInstaller::IListener> > m_listeners;
    DistQuery * m_distQuery;
    unsigned int m_iid;
    std::vector<unsigned char> m_pkgBuffer;
};

typedef struct {
    std::list<shared_ptr<SingleServiceInstaller> > m_installQueue;
    shared_ptr<SingleServiceInstaller> m_currentInstallation;
    std::list<std::string> m_distroServers;
    shared_ptr<ServiceRegistry> m_registry;
    unsigned int m_currentTransaction;
} InstallerContext;

static InstallerContext * s_context = NULL;

static bool
preflight(const std::string & name,
          const std::string & version)
{
    // don't bother with a blacklisted service
    PermissionsManager* pmgr = PermissionsManager::get();
    if (pmgr->serviceMayRun(name, version) == false) {
        BPLOG_WARN_STRM("Blacklisted service "
                        << name << "/"
                        << version << " not installed");
        // don't log to serviceinstalllog since we didn't actuall
        // install anything and it would unnecessarily grow the log
        return false;
    }

    // check if this service is already installed
    if (s_context->m_registry->haveService(name, version, std::string()))
    {
        BPLOG_WARN_STRM("service " << name << "/" 
                        << version << " already installed");
        return false;
    }
    return true;
}


void
ServiceInstaller::startup(std::list<std::string> distroServers,
                          shared_ptr<ServiceRegistry> registry)
{
    if (s_context != NULL) return;
    s_context = new InstallerContext;
    s_context->m_distroServers = distroServers;
    s_context->m_registry = registry;
    s_context->m_currentTransaction = 1000;
}


void
ServiceInstaller::shutdown()
{
    if (s_context == NULL) return;
    delete s_context;
    s_context = NULL;
}


unsigned int
ServiceInstaller::installService(
    const std::string & name,
    const std::string & version,
    weak_ptr<ServiceInstaller::IListener> listener)
{
    if (!preflight(name, version)) {
        return 0;
    }
    if (s_context == NULL) return 0;
    unsigned int iid = 0;

    // next, lets check if this service is already queued to be
    // installed.
    std::list<shared_ptr<SingleServiceInstaller> >::iterator it;

    for (it = s_context->m_installQueue.begin();
         it != s_context->m_installQueue.end();
         it++)
    {
        if (!(*it)->m_name.compare(name) &&
            !(*it)->m_version.compare(version))
        {
            BPLOG_INFO_STRM("service " << name
                            << "/" << version
                            << " is already queued for  installation, "
                            << "attaching to existing download.");
            return (*it)->addListener(listener);
        }
    }

    // cool! we can enqueue it

    // generate unique id
    iid = s_context->m_currentTransaction++;
    
    shared_ptr<SingleServiceInstaller> installer(
        new SingleServiceInstaller(s_context->m_distroServers,
                                   name, version, iid, listener));

    s_context->m_installQueue.push_back(installer);
    
    BPLOG_INFO_STRM("enqueued " << name
                    << "/" << version 
                    << " for installation, "
                    << s_context->m_installQueue.size() << " on queue");
    
    // and if we're not doing an install right now, we should start it
    // up.
    if (s_context->m_installQueue.size() == 1) {
        BPLOG_INFO("starting installer");
        installer->start();
    }

    return iid;
}


unsigned int
ServiceInstaller::installService(
    const std::string & name,
    const std::string & version,
    const std::vector<unsigned char> & buffer,
    weak_ptr<ServiceInstaller::IListener> listener)
{
    if (!preflight(name, version)) {
        return 0;
    }
               
    // if not, install it
    unsigned int iid = s_context->m_currentTransaction++;
    shared_ptr<SingleServiceInstaller> installer(
        new SingleServiceInstaller(name, version,
                                   buffer, iid, listener));
    s_context->m_installQueue.push_back(installer);
        
    BPLOG_INFO_STRM("enqueued prefetched " << name
                    << "/" << version 
                    << " for installation "
                    << s_context->m_installQueue.size() << " on queue");
    
    // and if we're not doing an install right now, we should start it
    // up.
    if (s_context->m_installQueue.size() == 1) {
        BPLOG_INFO("starting installer");
        installer->start();
    }
    
    return iid;
}


bool
ServiceInstaller::isBusy()
{
    return s_context && s_context->m_installQueue.size() > 0;
}


//////////////////////////////////////////////////////////////////////
// begin installer logic
////////////////////////////////////////////////////////////////////// 

SingleServiceInstaller::SingleServiceInstaller(
    std::list<std::string> distroServers,
    const std::string & name,
    const std::string & version,
    unsigned int iid,
    weak_ptr<ServiceInstaller::IListener> listener)
    : m_name(name), m_version(version), m_distQuery(NULL),
      m_iid(0), m_pkgBuffer()
{
    m_distQuery = new DistQuery(distroServers, PermissionsManager::get());
    m_distQuery->setListener(this);
    m_listeners.push_back(listener);
    m_iid = iid;
}


SingleServiceInstaller::SingleServiceInstaller(
    const std::string & name,
    const std::string & version,
    const std::vector<unsigned char> & buffer,
    unsigned int iid,
    weak_ptr<ServiceInstaller::IListener> listener)
    :  m_name(name), m_version(version), m_listeners(),
       m_distQuery(NULL), m_iid(iid), m_pkgBuffer(buffer)
{
    m_listeners.push_back(listener);
}


SingleServiceInstaller::~SingleServiceInstaller()
{
}


void
SingleServiceInstaller::postToAllListeners(bool success)
{
    std::list<weak_ptr<ServiceInstaller::IListener> >::iterator it;

    if (success) {
        for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
            shared_ptr<ServiceInstaller::IListener> p = (*it).lock();
            if (p) p->installed(m_iid, m_name, m_version);
        }
        BPLOG_INFO_STRM(m_name << ", ver " << m_version
                        << " installed successfully");
    } else {
        for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
            shared_ptr<ServiceInstaller::IListener> p = (*it).lock();
            if (p) p->installationFailed(m_iid);
        }
        BPLOG_WARN_STRM(m_name << ", ver " << m_version
                        << " failed to install");
    }
}

void
SingleServiceInstaller::progressUpdateToAllListeners(unsigned int progressPct)
{
    std::list<weak_ptr<ServiceInstaller::IListener> >::iterator it;
    for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
        shared_ptr<ServiceInstaller::IListener> p = (*it).lock();
        if (p) p->installStatus(m_iid, m_name, m_version, progressPct);
    }
}

bool
SingleServiceInstaller::installService(const std::vector<unsigned char> buf)
{
    // log timing output here
    bp::time::Stopwatch sw;
    sw.start();

    BPLOG_INFO_STRM("("<< sw.elapsedSec() <<"s) Installing service");

    // unpack and install
    ServiceUnpacker unpacker(buf);
    string errMsg;
    bool rval = unpacker.unpack(errMsg);

    BPLOG_INFO_STRM("("<< sw.elapsedSec() <<"s) unpacked");

    if (rval) {
        std::string errMsg;
        rval = unpacker.install(errMsg);
        BPLOG_INFO_STRM("("<< sw.elapsedSec() <<"s) installed");
        if (!rval) {
            BPLOG_WARN_STRM("failed to install, error: " << errMsg);
        }
    } else {
        BPLOG_WARN_STRM("failed to unpack, error: " << errMsg);
    }
    BPLOG_INFO_STRM("("<< sw.elapsedSec() <<"s) cleaned up");
    return rval;
}

void
SingleServiceInstaller::onTransactionFailed(unsigned int,
                                            const std::string& msg)
{
    BPLOG_WARN_STRM("service install failed: " << msg);
    postToAllListeners(false);
    removeSelfFromQueue();
}

void
SingleServiceInstaller::onDownloadProgress(unsigned int, unsigned int pct)
{
    progressUpdateToAllListeners(pct);
}

void
SingleServiceInstaller::onDownloadComplete(unsigned int,
                   const std::vector<unsigned char> & buf)
{
    BPLOG_INFO_STRM("downloaded " << m_name
                    << " ver " << m_version
                    << ", " << buf.size() << " bytes");

    // now we've got a buffer with the zipfile, the service name and
    // and version, so we're ready to try to install!
    bool ok = installService(buf);
    // regardless of wether the service installed correctly, we'll force
    // a disk rescan
    if (s_context != NULL) {
        s_context->m_registry->forceRescan();
    }
    postToAllListeners(ok);
    removeSelfFromQueue();
}

void
SingleServiceInstaller::removeSelfFromQueue()
{
    BPASSERT(s_context != NULL);
    BPASSERT(s_context->m_installQueue.front().get() == this);    

    // causes deletion of this object.  careful
    s_context->m_installQueue.pop_front();
    
    // now begin installation of the next item on the queue
    if (s_context->m_installQueue.size() >= 1) {
        s_context->m_installQueue.front()->start();
    }
}


void
SingleServiceInstaller::start()
{
    if (!m_pkgBuffer.empty()) {
        // serviceupdate has already downloaded, now "install"
        bool ok = installService(m_pkgBuffer);
        // regardless of wether the service installed correctly, we'll force
        // a disk rescan
        if (s_context != NULL) {
            s_context->m_registry->forceRescan();
        }
        postToAllListeners(ok);
        removeSelfFromQueue();
    } else {
        std::string platform = bp::os::PlatformAsString();

        BPLOG_INFO_STRM("now I should download and install "
                        << m_name << " - "
                        << m_version);
        
        if (!m_distQuery->downloadService(
                m_name,
                m_version,
                platform))
        {
            postToAllListeners(false);
            removeSelfFromQueue();
        }
    }
}


unsigned int
SingleServiceInstaller::addListener(
    weak_ptr<ServiceInstaller::IListener> listener)
{
    m_listeners.push_back(listener);
    return m_iid;
}
