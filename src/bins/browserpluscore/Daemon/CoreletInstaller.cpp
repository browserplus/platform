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

#include "CoreletInstaller.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/OS.h"
#include "CoreletManager/CoreletManager.h"
#include "DistributionClient/DistributionClient.h"
#include "Permissions/Permissions.h"

using namespace std;
using namespace std::tr1;


#define TMPDIR_PREFIX "BrowserPlus"

// the class that does all the asynchronous work of installing a corelet
class SingleCoreletInstaller : virtual public IDistQueryListener
{
public:
    // Constructor for installing from dist server
    SingleCoreletInstaller(
        std::list<std::string> distroServers,
        const std::string & name,
        const std::string & version,
        const bp::file::Path & dir,
        unsigned int iid,
        weak_ptr<CoreletInstaller::IListener> listener);
    
    // constructor for installing from bpkg buffer
    SingleCoreletInstaller(
        const std::string & name,
        const std::string & version,
        const std::vector<unsigned char> & buffer,
        const bp::file::Path & dir,
        unsigned int iid,
        weak_ptr<CoreletInstaller::IListener> listener);

    void start();

    ~SingleCoreletInstaller();

    // add a listener to be notified when this installation completes,
    // and return the installation id
    unsigned int addListener(
        weak_ptr<CoreletInstaller::IListener> listener);

    std::string m_name;
    std::string m_version; 
    
private:
    // implementation of methods from IDistQueryListener
    virtual void onTransactionFailed(unsigned int tid);
    virtual void onDownloadProgress(unsigned int tid, unsigned int pct);
    virtual void onDownloadComplete(unsigned int tid,
                                    const std::vector<unsigned char> & buf);

    void progressUpdateToAllListeners(unsigned int progressPct);
    void postToAllListeners(bool success);
    bool installCorelet(const std::vector<unsigned char> buf);
    void removeSelfFromQueue();
    std::list<weak_ptr<CoreletInstaller::IListener> > m_listeners;
    DistQuery * m_distQuery;
    bp::file::Path m_dir;
    unsigned int m_iid;
    std::vector<unsigned char> m_pkgBuffer;
};

typedef struct {
    std::list<shared_ptr<SingleCoreletInstaller> > m_installQueue;
    shared_ptr<SingleCoreletInstaller> m_currentInstallation;
    std::list<std::string> m_distroServers;
    shared_ptr<CoreletRegistry> m_registry;
    unsigned int m_currentTransaction;
} InstallerContext;

static InstallerContext * s_context = NULL;

static bool
preflight(const std::string & name,
          const std::string & version,
          const bp::file::Path & dir)
{
    if (dir.empty()) {
        BPLOG_WARN_STRM("Bogus install directory, corelet "
                        << name << "/"
                        << version << " not installed");
        return false;
    }
    
    // don't bother with a blacklisted corelet
    PermissionsManager* pmgr = PermissionsManager::get();
    if (pmgr->serviceMayRun(name, version) == false) {
        BPLOG_WARN_STRM("Blacklisted corelet "
                        << name << "/"
                        << version << " not installed");
        // don't log to coreletinstalllog since we didn't actuall
        // install anything and it would unnecessarily grow the log
        return false;
    }

    // check if this corelet is already installed
    if (s_context->m_registry->haveCorelet(name, version, std::string()))
    {
        BPLOG_WARN_STRM("corelet " << name << "/" 
                        << version << " already installed");
        return false;
    }
    return true;
}


void
CoreletInstaller::startup(std::list<std::string> distroServers,
                          shared_ptr<CoreletRegistry> registry)
{
    if (s_context != NULL) return;
    s_context = new InstallerContext;
    s_context->m_distroServers = distroServers;
    s_context->m_registry = registry;
    s_context->m_currentTransaction = 1000;
}


void
CoreletInstaller::shutdown()
{
    if (s_context == NULL) return;
    delete s_context;
    s_context = NULL;
}


unsigned int
CoreletInstaller::installCorelet(
    const std::string & name,
    const std::string & version,
    const bp::file::Path & dir,
    weak_ptr<CoreletInstaller::IListener> listener)
{
    if (!preflight(name, version, dir)) {
        return 0;
    }
    if (s_context == NULL) return 0;
    unsigned int iid = 0;

    // next, lets check if this corelet is already queued to be
    // installed.
    std::list<shared_ptr<SingleCoreletInstaller> >::iterator it;

    for (it = s_context->m_installQueue.begin();
         it != s_context->m_installQueue.end();
         it++)
    {
        if (!(*it)->m_name.compare(name) &&
            !(*it)->m_version.compare(version))
        {
            BPLOG_INFO_STRM("corelet " << name
                            << "/" << version
                            << " is already queued for  installation, "
                            << "attaching to existing download.");
            return (*it)->addListener(listener);
        }
    }

    // cool! we can enqueue it

    // generate unique id
    iid = s_context->m_currentTransaction++;
    
    shared_ptr<SingleCoreletInstaller> installer(
        new SingleCoreletInstaller(s_context->m_distroServers,
                                   name, version, dir,
                                   iid,
                                   listener));

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
CoreletInstaller::installCorelet(
    const std::string & name,
    const std::string & version,
    const std::vector<unsigned char> & buffer,
    const bp::file::Path & dir,
    weak_ptr<CoreletInstaller::IListener> listener)
{
    if (!preflight(name, version, dir)) {
        return 0;
    }
               
    // if not, install it
    unsigned int iid = s_context->m_currentTransaction++;
    shared_ptr<SingleCoreletInstaller> installer(
        new SingleCoreletInstaller(name, version,
                                   buffer, dir, iid, listener));
    s_context->m_installQueue.push_back(installer);
        
    BPLOG_INFO_STRM("enqueued prefetched " << name
                    << "/" << version 
                    << " for installation onto "
                    << dir << ", "
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
CoreletInstaller::isBusy()
{
    return s_context && s_context->m_installQueue.size() > 0;
}


//////////////////////////////////////////////////////////////////////
// begin installer logic
////////////////////////////////////////////////////////////////////// 

SingleCoreletInstaller::SingleCoreletInstaller(
    std::list<std::string> distroServers,
    const std::string & name,
    const std::string & version,
    const bp::file::Path & dir,
    unsigned int iid,
    weak_ptr<CoreletInstaller::IListener> listener)
    : m_name(name), m_version(version), m_distQuery(NULL), m_dir(),
      m_iid(0), m_pkgBuffer()
{
    m_distQuery = new DistQuery(distroServers, PermissionsManager::get());
    m_distQuery->setListener(this);
    m_listeners.push_back(listener);
    m_iid = iid;
    m_dir = dir;;
}


SingleCoreletInstaller::SingleCoreletInstaller(
    const std::string & name,
    const std::string & version,
    const std::vector<unsigned char> & buffer,
    const bp::file::Path & dir,
    unsigned int iid,
    weak_ptr<CoreletInstaller::IListener> listener)
    :  m_name(name), m_version(version), m_listeners(),
       m_distQuery(NULL), m_dir(dir), m_iid(iid), m_pkgBuffer(buffer)
{
    m_listeners.push_back(listener);
}


SingleCoreletInstaller::~SingleCoreletInstaller()
{
}


void
SingleCoreletInstaller::postToAllListeners(bool success)
{
    std::list<weak_ptr<CoreletInstaller::IListener> >::iterator it;

    if (success) {
        for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
            shared_ptr<CoreletInstaller::IListener> p = (*it).lock();
            if (p) p->installed(m_iid, m_name, m_version);
        }
        BPLOG_INFO_STRM(m_name << ", ver " << m_version
                        << " installed successfully");
    } else {
        for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
            shared_ptr<CoreletInstaller::IListener> p = (*it).lock();
            if (p) p->installationFailed(m_iid);
        }
        BPLOG_WARN_STRM(m_name << ", ver " << m_version
                        << " failed to install");
    }
}

void
SingleCoreletInstaller::progressUpdateToAllListeners(unsigned int progressPct)
{
    std::list<weak_ptr<CoreletInstaller::IListener> >::iterator it;
    for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
        shared_ptr<CoreletInstaller::IListener> p = (*it).lock();
        if (p) p->installStatus(m_iid, m_name, m_version, progressPct);
    }
}

bool
SingleCoreletInstaller::installCorelet(const std::vector<unsigned char> buf)
{
    // log timing output here
    bp::time::Stopwatch sw;
    sw.start();

    BPLOG_INFO_STRM("("<< sw.elapsedSec() <<"s) Installing service");

    // unpack and install
    CoreletUnpacker unpacker(buf, m_dir, m_name, m_version);
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
SingleCoreletInstaller::onTransactionFailed(unsigned int)
{
    postToAllListeners(false);
    removeSelfFromQueue();
}

void
SingleCoreletInstaller::onDownloadProgress(unsigned int, unsigned int pct)
{
    progressUpdateToAllListeners(pct);
}

void
SingleCoreletInstaller::onDownloadComplete(unsigned int,
                   const std::vector<unsigned char> & buf)
{
    BPLOG_INFO_STRM("downloaded " << m_name
                    << " ver " << m_version
                    << ", " << buf.size() << " bytes");

    // now we've got a buffer with the zipfile, the corelet name and
    // and version, so we're ready to try to install!
    bool ok = installCorelet(buf);
    // regardless of wether the corelet installed correctly, we'll force
    // a disk rescan
    if (s_context != NULL) {
        s_context->m_registry->forceRescan();
    }
    postToAllListeners(ok);
    removeSelfFromQueue();
}

void
SingleCoreletInstaller::removeSelfFromQueue()
{
    assert(s_context != NULL);
    assert(s_context->m_installQueue.front().get() == this);    

    // causes deletion of this object.  careful
    s_context->m_installQueue.pop_front();
    
    // now begin installation of the next item on the queue
    if (s_context->m_installQueue.size() >= 1) {
        s_context->m_installQueue.front()->start();
    }
}


void
SingleCoreletInstaller::start()
{
    if (!m_pkgBuffer.empty()) {
        // coreletupdate has already downloaded, now "install"
        bool ok = installCorelet(m_pkgBuffer);
        // regardless of wether the corelet installed correctly, we'll force
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
        
        if (!m_distQuery->downloadCorelet(
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
SingleCoreletInstaller::addListener(
    weak_ptr<CoreletInstaller::IListener> listener)
{
    m_listeners.push_back(listener);
    return m_iid;
}
