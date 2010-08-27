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
* RequireRequest.h
 *
 * A class which fulfills a service require.
 */

#ifndef __REQUIREREQUEST_H__
#define __REQUIREREQUEST_H__

#include "ServiceManager/ServiceManager.h"
#include "DistributionClient/DistributionClient.h"
#include "RequireLock.h"
#include "ServiceInstaller.h"


// forward declaration, as RequireRequest calls back into ActiveSession,
// behaving much like a service might (invoking callbacks)
class ActiveSession;

class RequireRequest : virtual public IDistQueryListener,
                       virtual public IServiceExecutionContextListener,
                       virtual public RequireLock::ILockListener,
                       virtual public ServiceInstaller::IListener,
                       public std::tr1::enable_shared_from_this<RequireRequest>
{
public:
    // key into localized strings for platform description
    static const char* kPlatformDescriptionKey;
    
	// a key into the bp::phash for the require's that have been done
	static const char* kRequireStatementsKey;
    
    static const char* kVersionSeparator;
	
    RequireRequest(const std::list<ServiceRequireStatement>& requires,
                   BPCallBack progressCallback,
                   std::tr1::weak_ptr<ActiveSession> activeSession,
                   unsigned int tid,
                   const std::string & primaryDistroServer,
                   const std::list<std::string>& secondaryDistroServers);

    virtual ~RequireRequest();    
    
    BPCallBack progressCallback() { return m_progressCB; }
    unsigned int tid() { return m_smmTid; }
    virtual void run();

    // Listener for RequireRequest events
    class IRequireListener {
    public:
        // Event raised when require completes, returns the tid as
        // well as the set of services that satisfy the requirements.
        virtual void onComplete(unsigned int tid,
                                const bp::List & satisfyingServices) = 0;

        // Event raised when require fails
        virtual void onFailure(unsigned int tid,
                               const std::string & error,
                               const std::string & verboseError) = 0;

        virtual ~IRequireListener() { }
    };
    
    void setListener(std::tr1::weak_ptr<IRequireListener> listener);
    
private:
    // implementation of methods from IDistQueryListener interface 
    void onTransactionFailed(unsigned int tid,
                             const std::string& msg);
    void onRequirementsSatisfied(unsigned int tid, const ServiceList & clist);
    void gotServiceSynopsis(unsigned int tid,
                            const ServiceSynopsisList & sslist);

    // implementation of methods from RequireLock::ILockListener interface 
    void gotRequireLock();

    // implementation of methods from ServiceInstaller::IListener interface 

    // invoked to deliver download percentage
    void installStatus(unsigned int installId,
                       const std::string & name,
                       const std::string & version,
                       unsigned int pct);

    // invoked when installation is complete
    void installed(unsigned int installId,
                   const std::string & name,
                   const std::string & version);

    // invoked when installation fails
    void installationFailed(unsigned int installId);

    void doRun();
    void onUserResponse(unsigned int cookie, const bp::Object & resp);

    void doNextRequire();    
    void installNextService();
    void checkPlatformUpdates();
    void updateRequireHistory(
        const std::list<ServiceRequireStatement> & requires);
    void promptUser();
    void postProgress(const std::string & name,
                      const std::string & version,
                      int localPercent);
    void postSuccess();
    void postFailure(const std::string& error,
                     const std::string& verboseError);
    
    ServiceSynopsis getDescription(const std::string & name,
                                   const std::string & version);
    bool checkDomainPermission(const std::string& permission);
    
    // Try to do a silent platform/service update.  Returns
    // true if silent update attempted.
    bool silentPlatformUpdate(const std::string& version);
    bool silentServiceUpdate(const std::string& service,
                             const std::string& version);
                
    // the original require statements and what must 
    // be installed/updated
    std::list<ServiceRequireStatement> m_requires;
    ServiceList m_toInstall;
    bool m_updatesOnly;
    
    // any pending platform updates
    std::vector<bp::SemanticVersion> m_platformUpdates;
    
    ServiceSynopsisList m_platformUpdateDescriptions;
    ServiceSynopsisList m_descriptions;
    ServiceSynopsis m_currentInstall;
    
    // any needed permissions
    std::set<std::string> m_permissions;
    
    unsigned int m_installSize;
    unsigned int m_installedSize;
    
    unsigned int m_smmTid;
    unsigned int m_distTid;
    unsigned int m_installTid;
    unsigned int m_promptCookie;
    
    bool m_zeroTotalProgressPosted;
    bp::time::Stopwatch m_lastPostTimer;
    
    BPCallBack m_progressCB;
    std::tr1::weak_ptr<ActiveSession> m_activeSession;
    
    DistQuery * m_distQuery;
    std::tr1::shared_ptr<ServiceRegistry> m_registry;
    std::string m_locale;

    std::tr1::weak_ptr<IRequireListener> m_listener;

    // a boost weak pointer populated in the ctor which is used
    // to ensure we clear out the RequireLock upon destruction
    std::tr1::weak_ptr<class RequireRequest> m_thisWeak;
};

#endif

