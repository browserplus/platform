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
 * PermissionsManager.h - A singleton responsible for maintaining up to
 *                        date black lists and domain permissions and for
 *                        checking permissions against those lists.
 *
 * Created by Lloyd Hilaiel on Fri July 29th 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __PERMISSIONSMANAGER_H__
#define __PERMISSIONSMANAGER_H__

#include <map>
#include <string>
#include <vector>
#include "BPUtils/bpthreadhopper.h"
#include "DistributionClient/DistributionClient.h"


class IPermissionsManagerListener 
{
  public:
    virtual ~IPermissionsManagerListener() { }
    virtual void gotUpToDate() = 0;
    virtual void cantGetUpToDate() = 0;
};

class PermissionsManager : virtual public IServiceFilter,
                           virtual public IDistQueryListener,
                           virtual public bp::thread::HoppingClass 
{
public:
    /**
     * State of a permanent permission.
     */
    typedef enum {
        eAllowed, eNotAllowed, eUnknown
    } Permission;
    
    /** 
     * Each domain permission granted/denied is represented 
     * by a PermissionInfo
     */
    class PermissionInfo {
     public:
        PermissionInfo(bool b) : m_allowed(b), m_time(BPTime()) {}
        PermissionInfo(bool b, const std::string& s) : m_allowed(b), m_time(s) {}
        bool m_allowed;
        BPTime m_time;
    };
    
    /**
     * Baked in domain permission key
     */
    static const char* kAllowDomain;

    /**
     * AutoUpdate info for a domain is represented by a AutoUpdateInfo.
     * Tells us if silent platform updates are allowed, and what services
     * may be silently updated.
     */
    class AutoUpdateInfo {
     public:
        AutoUpdateInfo() : m_platform(eUnknown), m_services(),
            m_time(BPTime()) {}
        std::string toString() const {
            std::stringstream ss;
            ss << "platformUpdate: " << m_platform;
            std::map<std::string, Permission>::const_iterator it;
            for (it = m_services.begin(); it != m_services.end(); ++it) {
                ss << ", " << it->first << ": " << it->second;
            }
            return ss.str();
        }
        Permission m_platform;
        std::map<std::string, Permission> m_services;
        BPTime m_time;
    };
    
    /**
     * Baked in key for autoUpdate
     */
    static const char* kAutoUpdate;
    
    /**
     * Get singleton, possibly creating.
     */
    static PermissionsManager* get(int major = -1,
                                   int minor = -1,
                                   int micro = -1); // throw bp::error::FatalException

    /** 
     * Is the permissions database sufficiently up to date that
     * we may run?  If so, returns true.  Otherwise, returns false
     * and initiates an update of permissions, posting status
     * events to listener.
     * \param listener - listener who will receive status events
     */
    bool upToDateCheck(IPermissionsManagerListener * listener);

    /**
     * Remove a listener canceling the check.  This function
     * will cause no callbacks to be called on the specified
     * listener after its return.
     *
     * \return true if the listener had called upToDateCheck
     *              and false was returned, and the listener
     *              has yet to recieve a response.  
     */
    bool cancelCheck(IPermissionsManagerListener * listener);
    
    /**
     * Return time of last check.
     *
     * \param t [out] on success, time of last check
     * \return true if a time was retrieved, else false
     */
    bool lastCheck(BPTime& t) const;

    /**
     * May we run at all (e.g. is platform version blacklisted)?
     */
    bool mayRun();
    
    /**
     * Are we currently updating permissions?
     * Should delay daemon shutdown.
     */
    bool isBusy() const;
    
    /**
     * Has a fatal error been encountered?
     * Will cause daemon shutdown.
     */
    bool error() const;
    
    /**
     * Do unapproved domains require user approval?
     */
    bool requireDomainApproval() const;
    
    /**
     * Set whether unapproved domains require user approval.  Defaults to true.
     */
    void setRequireDomainApproval(bool b);
    
    /** 
     * May a specified domain use browserplus?
     */
    Permission domainMayUseBrowserPlus(const std::string& domain) const;
    
    /**
     * Give a domain the virtual permission to use browserplus
     */
    void allowDomain(const std::string& domain);
    
    /**
     * Remove a domain's virtual permission to use browserplus
     */
    void disallowDomain(const std::string& domain);
    
    /** 
     * May the BrowserPlus Daemon use a specified service?
     */
    bool serviceMayRun(const std::string& name, 
                       const std::string& version) const;
    
    /**
     * Add a permission to a domain
     */
    void addDomainPermission(const std::string& domain,
                             const std::string& permission);
    
    /**
     * Revoke a permission from a domain.
     */
    void revokeDomainPermission(const std::string& domain,
                                const std::string& permission);
    
    /**
     * Reset a domain permission to eUnknown
     * The currentState arg is only used if 'permission' == kSilentServiceUpdate.
     * It is interpreted as follows:
     *  eAllowed:     only those permissions whose current
     *                value is eAllowed will be modified
     *  eNotAllowed:  only those permissions whose current
     *                value is eNotAllowed will be modified
     *  eUnknown:     all permissions will be modified
     */
    void resetDomainPermission(const std::string& domain,
                               const std::string& permission,
                               Permission currentState = eUnknown);
    
    /**
     * Revoke all permissions from a domain.
     */
    void revokeAllDomainPermissions(const std::string& domain);
    
    /**
     * Reset all permissions for a domain.
     */
    void resetAllDomainPermissions(const std::string& domain);
    
    /**
     * Revoke a permission from all domains
     */
    void revokePermission(const std::string& permission);
    
    /**
     * Reset a permission for all domains
     */
    void resetPermission(const std::string& permission);
    
    /** 
     * What permissions does a domain have?
     */
    std::map<std::string, PermissionInfo> 
        queryDomainPermissions(const std::string& domain)  const;
    
    /**
     * Does domain have a specific permission?
     */
    Permission queryDomainPermission(const std::string& domain,
                                     const std::string& permission) const;
    
    /**
     * What domains have a permission?
     */
    std::set<std::string> queryPermissionDomains(const std::string& permission) const;
    
    /**
     * Get a map of all domain permissions, key is domain, value is set of permissions.
     */
    std::map<std::string, std::map<std::string, PermissionInfo> > 
        queryAllDomainPermissions() const;

    /**
     * A generic structure capable of representing all different types of permissions
     * currently in the system
     */
    struct PermissionDesc {
        /** May be 'ServiceSilentUpdate', 'PlatformSilentUpdate', or any of the
         *  other permissions in the system */
        std::string type;
        /** the domain pattern for which the permission applies */
        std::string domain;
        /** certain permissions may be *negative*, that is disallowed in a sticky
         *  fashion (i.e. never let this site use my webcam) */
        Permission allowed;
        /** the time when the permission was set */
        BPTime time;        
        /** certain permissions may contain extra information/qualifiers.  The only
         *  current example is 'ServiceSilentUpdate' in which case 'extra' is set
         *  to the name of the services */
        std::set<std::string> extra;

        /* unneccesary details */
        PermissionDesc() : allowed(eUnknown) { };
    };
    
    /** get descriptions of all the permissions in the system.  A vector of
     *  PermissionDescs is return, contents described inline above. */
    std::vector<PermissionDesc> queryAllPermissions() const;
    
    /**
     * Get localized string for a permission.  If no localization found,
     * original key is returned.
     */
    std::string getLocalizedPermission(const std::string& permission,
                                       const std::string& locale) const;
    
    /** 
     * Does a domain have autoupdate permission for the platform?
     */
    Permission queryAutoUpdatePlatform(const std::string& domain) const;
     
    /** 
     * Does a domain have autoupdate permission for a service?
     */
    Permission queryAutoUpdateService(const std::string& domain,
                                      const std::string& service) const;
    
    /** 
     * Set platform autoupdate for a domain.
     */
    void setAutoUpdatePlatform(const std::string& domain,
                               Permission perm);
    
    /** 
     * Set specific service autoupdate for a domain.
     */
    void setAutoUpdateService(const std::string& domain,
                              const std::string& service,
                              Permission perm);
    
    /**
     * Get raw autoupdate map, key is domain, value is AutoUpdateInfo
     */
    std::map<std::string, AutoUpdateInfo> queryAutoUpdate() const;
    
    /**
     * Normalize a domain name by trying to resolved ip addresses
     */               
    std::string normalizeDomain(const std::string& domain) const;
    
private:
    PermissionsManager(const std::string& baseURL);
    virtual ~PermissionsManager();
    void load();
    void saveDomainPermissions(); // throw bp::error::FatalException
    bool domainPatternValid(const std::string& pattern) const;

    // implementation of ITimerListener interface
    void timesUp(class Timer * t);
    
    static PermissionsManager* m_singleton;
    
    double m_checkDays;
    std::string m_url;
    DistQuery * m_distQuery;
    bool m_error;
    
    // map of outstanding transaction ids to listeners
    std::map<unsigned int, IPermissionsManagerListener *> m_listeners;
    
    // if permissions on disk are missing or malformed,
    // we must fetch from server
    bool m_badPermissionsOnDisk;
    
    // key is service name, value is vector of
    // blacklisted versions x.y.z.  pattern
    // matching of version via bp::ServiceVersion::match() 
    std::map<std::string, std::vector<std::string> > m_blacklist; 
    
    // blacklisted platforms
    std::vector<std::string> m_platformBlacklist;
    
    // require user approval for unapproved domains?
    bool m_requireDomainApproval;
    
    // map of domain permissions, key is domain, value is map of 
    // permissions and their PermissionInfo
    std::map<std::string, std::map<std::string, PermissionInfo> > m_domainPermissions;
    
    // map of autoupdate permissions, key is domain, value is AutoUpdateInfo
    std::map<std::string, AutoUpdateInfo> m_autoUpdatePermissions;
    
    // permission localizations.  key is permission, value is map of 
    // key/localizations
    std::map<std::string, std::map<std::string, std::string> > m_permLocalizations;
    
    virtual void onTransactionFailed(unsigned int tid);
    virtual void gotPermissions(unsigned int tid,
                                std::vector<unsigned char> permBundle);

    // only used when we encounter setup failures to inform listener
    // that the upToDateCheck failed
    void onHop(void * context);
};

#endif
