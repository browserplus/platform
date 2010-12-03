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
 * Summary
 *
 * A Class capable of determining if a directory represents a valid service,
 * and if so the type of service and metadata contained in the service
 * manifest.
 */

#include "ServiceSummary.h"
#include <sstream>
#include <string.h>
#include "bplocalization.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bptime.h"
#include "BPUtils/bptypeutil.h"


using namespace bp;

static const char * s_standaloneType = "standalone";
static const char * s_dependentType = "dependent";
static const char * s_providerType = "provider";

static const char * s_manifestFileName = "manifest.json";

static const char * s_serviceLibraryKey = "ServiceLibrary";
static const char * s_deprecatedServiceLibraryKey = "CoreletLibrary";
static const char * s_typeKey = "type";
static const char * s_usesKey = "uses";

static const char * s_shutdownDelayKey = "shutdownDelaySecs";
static const char * s_serviceKey = "service";
static const char * s_deprecatedServiceKey = "corelet";
static const char * s_versionKey = "version";
static const char * s_minversionKey = "minversion";

static const char * s_argumentsKey = "arguments";

// localization of end user facing description of service.
static const char * s_stringsKey = "strings";
static const char * s_titleKey = "title";
static const char * s_summaryKey = "summary";
static const char * s_englishKey = "en";

// list of requires permissions
static const char * s_permissionsKey = "permissions";

service::Summary::Summary()
    : m_type(None), m_modDate(), m_shutdownDelaySecs(-1)
{
}

service::Summary::~Summary()
{
}

std::string
service::Summary::name() const
{
    return m_name;
}

std::string 
service::Summary::version() const
{
    return m_version;
}
    
const boost::filesystem::path&
service::Summary::path() const
{
    return m_path;
}

bool
service::Summary::detectService(const boost::filesystem::path &dirName,
                                std::string &error)
{    
    clear();

    // determine path to the manifest file
    boost::filesystem::path manifestPath = dirName / s_manifestFileName;

    std::string manifestContents;

    // read file
    if (!bp::strutil::loadFromFile(manifestPath, manifestContents)) {
        error.append("Couldn't read '");
        error.append(manifestPath.string());
        error.append("'");
        return false;
    }

    // now parse JSON in manifestContents
    std::string jsonParseError;
    bp::Object * o =
        bp::Object::fromPlainJsonString(manifestContents, &jsonParseError);
    if (o == NULL) {
        error.append("Syntax error in manifest: ");
        if (!jsonParseError.empty()) error.append(jsonParseError);
        return false;
    }
    
    // determine the Service type
    if (!o->has(s_typeKey, BPTString))
    {
        error.append("Manifest missing required '");
        error.append(s_typeKey);
        error.append("' key");
        delete o;
        return false;
    }
    std::string type;
    type.append(*(o->get(s_typeKey)));

    // parse the optional shutdown delay
    if (o->has(s_shutdownDelayKey, BPTInteger))
    {
        m_shutdownDelaySecs = (int) (long long) *(o->get(s_shutdownDelayKey));
    }

    // all services must be localized to at least english
    std::map<std::string, std::pair<std::string, std::string> > localizations;

    if (!o->has(s_stringsKey, BPTMap))
    {
        error.append("Manifest missing required '");
        error.append(s_stringsKey);
        error.append("' key (localization)");
        delete o;
        return false;
    }

    // internalize localization from manifest.json
    {
        const bp::Object * m = o->get(s_stringsKey);
  
        if (!m->has(s_englishKey, BPTMap)) {
            error.append("Manifest missing required english localization.");
            delete o;
            return false;
        }

        std::map<std::string, const bp::Object *> mMap = *m;
        std::map<std::string, const bp::Object *>::const_iterator it;

        for (it = mMap.begin(); it != mMap.end(); it++) {
            if (it->second->type() != BPTMap) {
                error.append("'");
                error.append(it->first);
                error.append("' is not a map in localization table ");
                error.append("in manifest file.");
                delete o;
                return false;
            }

            const bp::Object * l = it->second;
            if (!l->has(s_titleKey, BPTString) ||
                !l->has(s_summaryKey, BPTString))
            {            
                error.append("'");
                error.append(it->first);
                error.append("' locale requires both '");
                error.append(s_titleKey);
                error.append("' and '");
                error.append(s_summaryKey);
                error.append("' in manifest file.");
                delete o;
                return false;
            }

            localizations[it->first] =
                std::pair<std::string, std::string>(
                    *(l->get(s_titleKey)), *(l->get(s_summaryKey)));
        }
    }
    
    if (o->has(s_permissionsKey)) {
        const bp::Object * perms = o->get(s_permissionsKey);
        if (perms->type() != BPTList) {
            std::stringstream ss;
            ss << "'" << s_permissionsKey << "' when present, must be a list";
            error.append(ss.str());
            delete o;
            return false;
        }

        std::vector<const bp::Object *> v = *perms;
        for (unsigned int i = 0; i < v.size(); i++) {
            if (v[i]->type() != BPTString) {
                std::stringstream ss;
                ss << "'" << s_permissionsKey
                   << "' when present, must be a list of STRINGS";
                error.append(ss.str());
                delete o;
                return false;
                
            }
            m_permissions.insert(*(v[i]));
        }
    }

    // based on the type we'll do further validation
    if (!type.compare(s_standaloneType))
    {
        std::string serviceLibrary;
        
        // extract the path to the service library
        if (o->has(s_serviceLibraryKey, BPTString))
        {
            serviceLibrary = (std::string) *(o->get(s_serviceLibraryKey));
        }
        else if (o->has(s_deprecatedServiceLibraryKey, BPTString))
        {
            serviceLibrary = (std::string) *(o->get(s_deprecatedServiceLibraryKey));
        }
        else
        {
            std::stringstream ss;
            ss << "'" << s_standaloneType << "' services require a '"
               << s_serviceLibraryKey << "' key";
            error.append(ss.str());
            delete o;
            return false;
        }
        
        // got it, allocate summary
        m_type = Standalone;
		m_serviceLibraryPath /= serviceLibrary;
    }
    else if (!type.compare(s_providerType))
    {
        std::string serviceLibrary;

        // extract the path to the service library
        if (o->has(s_serviceLibraryKey, BPTString))
        {
            serviceLibrary = (std::string) *(o->get(s_serviceLibraryKey));
        }
        else if (o->has(s_deprecatedServiceLibraryKey, BPTString))
        {
            serviceLibrary = (std::string) *(o->get(s_deprecatedServiceLibraryKey));
        }
        else
        {
            std::stringstream ss;
            ss << "'" << s_providerType << "' services require a '"
               << s_serviceLibraryKey << "' key";
            error.append(ss.str());
            delete o;
            return false;
        }

        m_type = Provider;
		m_serviceLibraryPath /= serviceLibrary;
    }
    else if (!strcmp(s_dependentType, type.c_str()))
    {
        // require a "uses" element
        if (!o->has(s_usesKey, BPTMap))
        {
            std::stringstream ss;
            ss << "'" << s_dependentType << "' services require a '"
               << s_usesKey << "' key";
            error.append(ss.str());
            delete o;
            return false;
        }

        const bp::Object * uses = o->get(s_usesKey);
        BPASSERT(uses != NULL);
        
        // "uses" map requires at least service name
        if (uses->has(s_serviceKey, BPTString))
        {
            m_usesService.append(*(uses->get(s_serviceKey)));
        }
        else if (uses->has(s_deprecatedServiceKey, BPTString))
        {
            m_usesService.append(*(uses->get(s_deprecatedServiceKey)));
        }
        else
        {
            std::stringstream ss;
            ss << "'" << s_usesKey << "' map requires a '" << s_serviceKey
               << "' key.";
            error.append(ss.str());
            delete o;
            return false;
        }

        // now extract the versions and validate them if present 
        if (uses->has(s_versionKey, BPTString)) {
            std::string v(*(uses->get(s_versionKey)));
            
            // try to parse
            if (!m_usesVersion.parse(v)) {
                error.append("malformed version string: '" + v + "'");
                delete o;
                return false;
            }
        }

        if (uses->has(s_minversionKey, BPTString)) {
            std::string v = *(uses->get(s_minversionKey));
            // try to parse
            if (!m_usesMinversion.parse(v)) {
                error.append("malformed version string: '" + v + "'");
                delete o;
                return false;
            }
        }

        // now let's parse up the "Arguments" map
        if (o->has(s_argumentsKey, BPTMap)) {
            const bp::Map * args = (bp::Map *) o->get(s_argumentsKey);
            BPASSERT(args != NULL);

            bp::Map::Iterator i(*args);
            const char * key = NULL;
            while (NULL != (key = i.nextKey())) {
                if (args->get(key)->type() != BPTString) {
                    error.append("only strings allowed in 'arguments' map, '"
                                 + std::string(key) +
                                 "' is not a string.");
                    delete o;
                    return false;
                }
                m_arguments[key] = (std::string) *(args->get(key));
            }
        }
            
        m_type = Dependent;

        // sheesh, that was tiring.
    }
    else
    {
        error.append("Invalid service type '" + type + "'");
        delete o;
        return false;
    }

    m_path = dirName;
        
    // now set the modtime
    try {
        m_modDate.set(boost::filesystem::last_write_time(manifestPath));
    } catch (const boost::filesystem::filesystem_error&) {
        m_modDate.set(0);
    }

    // and set the localization table
    m_localizations = localizations;
    
    delete o;

    // (lth) ServiceSummaries are thrown around everywhere because
    // they're lightweight.  Building one requires parsing JSON, but
    // doesn't require loading a shared library.  There are many cases
    // where we'd like to know the service name and version *without*
    // loading the library, and we approximate that by relying on the
    // convention that services usually reside in a directory
    // hierarchy where the parent is the version, and the grandparent
    // is the service name.  This code works by testing if the parent
    // is a parseable version, if it isn't then we assume that this
    // service isn't in a <name>/<version> dir, and leave these two
    // members empty.
    std::string versionCandidate = dirName.filename().string();
    bp::SemanticVersion sv;

    if (!versionCandidate.empty() && sv.parse(versionCandidate)) {
        m_version = versionCandidate;     
        m_name = dirName.parent_path().filename().string();
    } else {
        m_version.clear();
        m_name.clear();
    }
    
    return true;
}

BPTime
service::Summary::modDate() const
{
    return m_modDate;
}

bool
service::Summary::outOfDate() const
{
    boost::filesystem::path manifestPath = m_path / s_manifestFileName;
    BPTime t((long)0);
    try {
        t.set(boost::filesystem::last_write_time(manifestPath));
    } catch (const boost::filesystem::filesystem_error&) {
        t.set(0);
    }
    return (0 != m_modDate.compare(t));
}

int
service::Summary::shutdownDelaySecs() const
{
    return m_shutdownDelaySecs;
}

std::string
service::Summary::typeAsString() const
{
    switch(type()) {
        case Standalone: return "standalone";
        case Dependent: return "dependent";
        case Provider: return "provider";
        case BuiltIn: return "builtin";
        case None: return "invalid";
    }
    return "unknown";
}

std::list<std::string>
service::Summary::localizations() const
{
    std::list<std::string> locales;
    std::map<std::string, std::pair<std::string, std::string> >::const_iterator it;
    for (it = m_localizations.begin(); it != m_localizations.end(); it++)
    {
        locales.push_back(it->first);
    }
    return locales;
}


/** get the localization for a specific locale */
bool
service::Summary::localization(const std::string &locale,
                               std::string &title, std::string &summary) const
{
    std::vector<std::string> locales = 
        bp::localization::getLocaleCandidates(locale);
    std::map<std::string, std::pair<std::string, std::string> >::const_iterator it;
    for (unsigned int i = 0; i < locales.size(); i++) {
        it = m_localizations.find(locales[i]);
        if (it != m_localizations.end()) {
            title = it->second.first;
            summary = it->second.second; 
            return true;
        }
    }
    return false;
}

void
service::Summary::setLocalizations(std::map<std::string,
                                   std::pair<std::string, std::string> > l)
{
    m_localizations = l;
}

boost::filesystem::path
service::Summary::serviceLibraryPath() const
{
    return m_serviceLibraryPath;
}
std::string
service::Summary::usesService() const
{
    return m_usesService;
}

bp::SemanticVersion
service::Summary::usesVersion() const
{
    return m_usesVersion;
}

bp::SemanticVersion
service::Summary::usesMinversion() const
{
    return m_usesMinversion;
}

service::Summary::ServiceType
service::Summary::type() const
{
    return m_type;
}

std::map<std::string, std::string>
service::Summary::arguments() const
{
    return m_arguments;
}

void
service::Summary::clear()
{
    m_type = None;

    m_shutdownDelaySecs = -1;
    m_name.clear();
    m_version.clear();
    m_path.clear();
    m_localizations.clear();
    m_permissions.clear();
    // TODO: reset m_modDate;

    m_serviceLibraryPath.clear();    
    m_usesService.clear();
    m_usesVersion.parse("");
    m_usesMinversion.parse("");
    m_arguments.clear();
}

bool
service::Summary::isInitialized() const
{
    // path is a good indicator
    return (!m_path.empty());
}

std::set<std::string>
service::Summary::permissions() const
{
    return m_permissions;
}

bool
bp::service::operator<(const service::Summary & lhs,
                       const service::Summary & rhs)
{
    // sort dependent services last
    if (lhs.type() == service::Summary::Dependent && 
        rhs.type() != lhs.type())
    {
        return false;
    }
    else if (rhs.type() == service::Summary::Dependent && 
             rhs.type() != lhs.type())
    {
        return true;
    }
    
    return (lhs.path().string().compare(rhs.path().string()) < 0);
}

bp::service::BuiltInSummary::BuiltInSummary(const std::string & name,
                                            const std::string & version)
{
    m_type = BuiltIn;
    m_name = name;
    m_version = version;
}

std::string
bp::service::Summary::toHumanReadableString() const
{
    std::stringstream ss;
    ss << "Summary for service: " << path() << std::endl;
    if (!m_name.empty()) {
        ss << "  " << m_name;
        if (!m_version.empty()) ss << "/" << m_version;
        ss << std::endl;
    }
    ss << "type: " << typeAsString() << std::endl;
    if (m_type == Provider || m_type == Standalone) {
        ss << "library: " << m_serviceLibraryPath << std::endl;
    } else if (m_type == Dependent) {
        ss << "depends on: " << std::endl;
        ss << "   service: " << m_usesService << std::endl;
        if (m_usesVersion.majorVer() >= 0) {
            ss << "   version: " << m_usesVersion.asString() << std::endl;
        }
        if (m_usesMinversion.majorVer() >= 0) {
            ss << "   minversion: " << m_usesMinversion.asString() << std::endl;
        }
        ss << "arguments to provider service: " << std::endl;        
        bp::Map m;
        std::map<std::string, std::string>::const_iterator i;
        for (i = m_arguments.begin(); i != m_arguments.end(); i++) {
            m.add(i->first, new bp::String(i->second));
        }
        ss << m.toPlainJsonString(true);
    }
    if (m_shutdownDelaySecs >= 0) {
        ss << "Shutdown delay: " << m_shutdownDelaySecs << "s" << std::endl;
    }

    ss << "localized to: ";
    {
        std::map<std::string, std::pair<std::string, std::string> >
            ::const_iterator i;
        bool firstRun = true;
        for (i = m_localizations.begin(); i != m_localizations.end(); i++) {
            if (!firstRun) ss << ", ";
            ss << i->first;
            firstRun = false;
        }
    }
    ss << std::endl;

    ss << "required permissions: " << std::endl;
    {
        std::set<std::string>::const_iterator i;
        for (i = m_permissions.begin(); i != m_permissions.end(); i++) {
            ss << "    " << *i << std::endl;
        }
    }

    return ss.str();
}

