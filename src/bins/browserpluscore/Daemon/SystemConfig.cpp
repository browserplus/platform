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

#include "SystemConfig.h"
#include "BPDaemon.h"
#include "I18n/FormatTime.h"
#include "I18n/idna.h"

#include <string.h>

using namespace std;
using namespace std::tr1;


typedef bp::Object * (* stateFunc)(const char * locale,
                                   const bp::Object * arg);

// forward declarations
static bp::Object * getHelpFunc(const char *,
                                const bp::Object * = NULL);
static bp::Object * requireDomainApprovalFunc(const char *,
                                              const bp::Object * = NULL);
static bp::Object * setHelpFunc(const char *,
                                const bp::Object * = NULL);
static bp::Object * activeSitesFunc(const char *,
                                    const bp::Object * = NULL);
static bp::Object * activeServicesFunc(const char * locale,
                                       const bp::Object * = NULL);
static bp::Object * permissionsFunc(const char * locale,
                                    const bp::Object * = NULL);
static bp::Object * alwaysAskFunc(const char *,
                                  const bp::Object * arg);
static bp::Object * setPermissionsFunc(const char *,
                                       const bp::Object * arg);
static bp::Object * deleteServiceFunc(const char *,
                                      const bp::Object * arg);

// lookup table of supported keys
typedef struct 
{
    const char * key;
    const char * doc;
    stateFunc sf;
} tKeyStruct;

static tKeyStruct s_supportedGetKeys[] = 
{
    {
        "help",
        "Provide a list of supported state keys with documentation.",
        getHelpFunc
    },
    {
        "requireDomainApproval",
        "Returns boolean indicating whether BrowserPlus requires user "
        "approval of unapproved domains.",
        requireDomainApprovalFunc
    },
    {
        "activeSites",
        "Returns all active sites connected to BrowserPlus, and the "
        "services that they are using.  Return value is a list of"
        "maps.  Each map contains keys 'site' (string value), "
        "'type' (string value), and 'services'.  'services' value is a list "
        "of maps containing 'service' and 'version' keys (both string values).",
        activeSitesFunc
    },
    {
        "activeServices",
        "return localized name, version, and descriptions of all services "
        "currently active.",
        activeServicesFunc
    },
    {
        "permissions",
        "Returns all persistent domain permissions.  Return value is "
        "a list of maps.  Each map contains keys 'site' (string), "
        "'when' (integer), 'whenStr' (string), "
        "'permissionKey' (string), and 'permission' (string).",
        permissionsFunc
    }
};

static tKeyStruct s_supportedSetKeys[] =
{
    {
        "help",
        "Provide a list of supported state keys with documentation.",
        setHelpFunc
    },
    {
        "requireDomainApproval",
        "Set whether unapproved domains require user approval to "
        "use BrowserPlus.  Argument is a single boolean.",
        alwaysAskFunc
    },
    {
        "permissions",
        "Set domain permissions.  Argument is a list of maps.  Each "
        "map contains keys site, permissionKey, and value.  All values "
        "are strings, 'value' is allow/deny/reset'.",
        setPermissionsFunc
    },
    {
        "deleteService",
        "Remove service.  Argument is a map containing keys "
        "service and version.  All values are strings.",
        deleteServiceFunc
    }
};

static bp::Object *
getHelpFunc(const char *, 
            const bp::Object *)
{
    bp::List * l = new bp::List;
    unsigned int i = 0;

    for (i=0; i<sizeof(s_supportedGetKeys)/sizeof(s_supportedGetKeys[0]) ; i++)
    {
        bp::List * kdesc = new bp::List;
        kdesc->append(new bp::String(s_supportedGetKeys[i].key));
        kdesc->append(new bp::String(s_supportedGetKeys[i].doc));
        l->append(kdesc);
    }
    
    return l;
}

static bp::Object *
setHelpFunc(const char *,
            const bp::Object *)
{
    bp::List * l = new bp::List;
    unsigned int i = 0;
    
    for (i=0; i<sizeof(s_supportedSetKeys)/sizeof(s_supportedSetKeys[0]) ; i++)
    {
        bp::List * kdesc = new bp::List;
        kdesc->append(new bp::String(s_supportedSetKeys[i].key));
        kdesc->append(new bp::String(s_supportedSetKeys[i].doc));
        l->append(kdesc);
    }
    
    return l;
}


static bp::Object *
requireDomainApprovalFunc(const char *,
                          const bp::Object *)
{
    PermissionsManager * pmgr = PermissionsManager::get();
    return new bp::Bool(pmgr->requireDomainApproval());
}


static bp::Object *
activeSitesFunc(const char *,
                const bp::Object *)
{
    bp::List * l = new bp::List;

    shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
    if (daemon != NULL) {
        std::vector<shared_ptr<ActiveSession> > sessions = 
            daemon->sessionManager()->currentSessions();

        for (unsigned int i = 0; i < sessions.size(); i++)
        {
            std::string site = "unknown";
            std::string type = "unknown";            

            bp::url::Url parser;
            if (parser.parse(sessions[i]->URI())) {
                if (parser.scheme() == "http" || parser.scheme() == "https") {
                    type = "website";
                    // This info will be displayed, so call idnaToUnicode().
                    site = bp::i18n::idnaToUnicode(parser.host());
                } else if (parser.scheme() == "file") {
                    type = "website";
                    site = "local file: ";
                    boost::filesystem::path p(parser.path());
                    site.append(p.filename().string());
                } else if (parser.scheme() == "bpclient") {
                    type = "program";
                    if (!parser.path().empty()) {
                        site = parser.path();
                        // hack of leading slash
                        if (site.length() > 0 && site[0] == '/') {
                            site = site.substr(1);
                        }
                    }
                }
            }

            // now services
            bp::List * services = new bp::List;
            {
                std::vector< std::pair<std::string, std::string> >
                    sVect = sessions[i]->instances();

                for (unsigned int j = 0; j < sVect.size(); j++)
                {
                    bp::Map * s = new bp::Map;
                    s->add("service", new bp::String(sVect[j].first));
                    s->add("version", new bp::String(sVect[j].second));
                    services->append(s);
                }
            }
            
            bp::Map * m = new bp::Map;

            m->add("services", services);
            m->add("type", new bp::String(type));
            m->add("site", new bp::String(site));
            
            l->append(m);
        }
    }
    
    BPLOG_DEBUG_STRM("return " << l->toJsonString());
    
    return l;
}

static bp::Object *
activeServicesFunc(const char * locale,
                   const bp::Object *)
{
    bp::List * l = new bp::List;

    shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
    if (daemon != NULL) {
        shared_ptr<ServiceRegistry> registry = daemon->registry();;
        if (registry) {
            std::list<bp::service::Summary> summaries =
                registry->availableServiceSummaries();

            std::list<bp::service::Summary>::iterator it;

            for (it = summaries.begin(); it != summaries.end(); it++) {
                std::string title, summary;
                
                if (it->localization(locale, title, summary)) {
                    bp::Map *m = new bp::Map;
                    // not localized
                    m->add("name", new bp::String(it->name()));
                    m->add("version", new bp::String(it->version()));

                    // localized
                    m->add("title", new bp::String(title));
                    m->add("desc", new bp::String(summary));

                    l->append(m);
                }
            }
        }
    }
    
    BPLOG_DEBUG_STRM("return " << l->toJsonString());
    
    return l;
}


static bp::Object *
permissionsFunc(const char * locale,
                const bp::Object *)
{
    using namespace std;
    
    PermissionsManager * pmgr = PermissionsManager::get();
    bp::List * rval = new bp::List;
    vector<PermissionsManager::PermissionDesc> allPerms =
        pmgr->queryAllPermissions();
    
    for (unsigned int i = 0; i < allPerms.size(); i++) 
    {
        if (allPerms[i].allowed == PermissionsManager::eUnknown) {
            // skip permissions which are unset
            continue;
        }
        string domain = allPerms[i].domain;
        string perm = allPerms[i].type;
        string localization;
        if (!perm.compare("SilentServiceUpdate")) {
            // Special handling for service silent updates.  We bundle it all
            // into one string.
            localization = pmgr->getLocalizedPermission(perm, locale);
            localization += " ";
            std::set<std::string>::const_iterator sit;
            bool first = true;
            for (sit = allPerms[i].extra.begin(); sit != allPerms[i].extra.end() ; sit++ )
            {
                if (!first) localization += ", ";
                first = false;
                localization += *sit;
            }
        } else {
            localization = pmgr->getLocalizedPermission(perm, locale);
        }
        
        BPTime permTime = allPerms[i].time;
        string whenStr = bp::i18n::formatRelativeTime(permTime,locale);

        bp::Map * m = new bp::Map;
        m->add("site", new bp::String(bp::i18n::idnaToUnicode(domain)));
        m->add("when", new bp::Integer(permTime.get()));
        m->add("whenStr", new bp::String(whenStr));
        m->add("permissionKey", new bp::String(perm));
        m->add("permission", new bp::String(localization));
        m->add("allow", new bp::Bool(allPerms[i].allowed == PermissionsManager::eAllowed));
        rval->append(m);
    }
    BPLOG_DEBUG_STRM("return " << rval->toJsonString());
    return rval;
}


static bp::Object *
alwaysAskFunc(const char *,
              const bp::Object * arg)
{
    bp::Object * rval = NULL;
    const bp::Bool * b = dynamic_cast<const bp::Bool *>(arg);
    if (b) {
        PermissionsManager::get()->setRequireDomainApproval(b->value());
        rval = new bp::Bool(true);
    } else {
        rval = new bp::Bool(false);
    }
    return rval;
}


static bp::Object *
setPermissionsFunc(const char *,
                   const bp::Object * arg)
{
    using namespace std;
    bp::Object * rval = NULL;
    try {
        PermissionsManager * pmgr = PermissionsManager::get();
        const bp::List * l = dynamic_cast<const bp::List *>(arg);
        if (!l) BP_THROW("arguments not a list");
        for (unsigned int i = 0; i < l->size(); i++) {
            const bp::Map * m = dynamic_cast<const bp::Map *>(l->value(i));
            if (!m) BP_THROW("argument not a map");
            const bp::String * s = dynamic_cast<const bp::String *>(m->get("site"));
            if (!s) BP_THROW("argument missing 'site'");
            string site(s->value());
            s = dynamic_cast<const bp::String *>(m->get("permissionKey"));
            if (!s) BP_THROW("argument missing 'permissionKey'");
            string permissionKey(s->value());
            s = dynamic_cast<const bp::String *>(m->get("value"));
            if (!s) BP_THROW("argument missing 'value'");
            string value(s->value());
            if (!value.compare("allow")) {
                pmgr->addDomainPermission(site, permissionKey);
            } else if (!value.compare("deny")) {
                pmgr->revokeDomainPermission(site, permissionKey);
            } else if (!value.compare("reset")) {
                const bp::Bool * b = dynamic_cast<const bp::Bool *>(m->get("allow"));
                if (!b) BP_THROW("argument missing 'allow'");
                PermissionsManager::Permission p = b->value() 
                    ? PermissionsManager::eAllowed : 
                      PermissionsManager::eNotAllowed;
                pmgr->resetDomainPermission(site, permissionKey, p);
            } else {
                BP_THROW("bad value of " + value + " for " 
                         + site + "/" + permissionKey);
            }
        }
        rval = new bp::Bool(true);
    } catch (const std::string& s) {
        BPLOG_ERROR_STRM(s);
    }
    
    if (!rval) rval = new bp::Bool(false);
    return rval;
}


static bp::Object *
deleteServiceFunc(const char *,
                  const bp::Object * arg)
{
    BPLOG_DEBUG_STRM("arg = " << arg->toJsonString());
    using namespace std;
    
    shared_ptr<BPDaemon> daemon = BPDaemon::getSharedDaemon();
    if (daemon == NULL || daemon->registry() == NULL) {
        return new bp::Bool(false);
    }
        
    bool rval = false;
    try {
        const bp::Map * m = dynamic_cast<const bp::Map *>(arg);
        if (!m) BP_THROW("arguments not a map");
        const bp::String * s = dynamic_cast<const bp::String *>(m->get("service"));
        if (!s) BP_THROW("argument missing 'service'");
        string service(s->value());
        s = dynamic_cast<const bp::String *>(m->get("version"));
        if (!s) BP_THROW("argument missing 'version'");
        string version(s->value());
        rval = daemon->registry()->purgeService(service, version);
    } catch (const std::string& s) {
        BPLOG_ERROR_STRM(s);
    }
    
    return new bp::Bool(rval);
}


bp::Object *
SystemConfig::getState(const char * locale,
                       const char * key)
{
    bp::Object * rv = NULL;
    unsigned int i;
    
    for (i=0; i<sizeof(s_supportedGetKeys)/sizeof(s_supportedGetKeys[0]) ; i++)
    {
        if (!strcmp(key, s_supportedGetKeys[i].key)) {
            rv = s_supportedGetKeys[i].sf(locale, NULL);
            break;
        }
    }

    if (rv == NULL) rv = new bp::Null;

    return rv;
}


bp::Object * 
SystemConfig::setState(const char * key,
                       const bp::Object * arg)
{
    bp::Object * rv = NULL;
    unsigned int i;
    
    for (i=0; i<sizeof(s_supportedSetKeys)/sizeof(s_supportedSetKeys[0]) ; i++)
    {
        if (!strcmp(key, s_supportedSetKeys[i].key)) {
            rv = s_supportedSetKeys[i].sf(NULL, arg);
            break;
        }
    }
    
    if (rv == NULL) rv = new bp::Bool(false);
    
    return rv;
}

