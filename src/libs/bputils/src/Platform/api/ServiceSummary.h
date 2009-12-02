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
 * ServiceSummary.h
 *
 * A class capable of providing high level information about a service on
 * disk.  Essentially ServiceSummary exposes all of the information available
 * in the manifest.json file of a corelet.
 *
 * ServiceSummary also contains the name and version of the corelet.  This
 * can be determined from path (<name>/<version> or can be set explicitly).
 *
 * ServiceSummary does _not_ load nor unload dynamic libraries.
 */

#ifndef __SERVICESUMMARY_H__
#define __SERVICESUMMARY_H__

#include "BPUtils/bptime.h"
#include "BPUtils/bpserviceversion.h"
#include "BPUtils/bpfile.h"

#include <set>
#include <string>
#include <list>
#include <map>

namespace bp { namespace service {

/**
 * A class for representing service metadata and populating said metadata
 * from disk
 */ 
class Summary
{
public:
    // default copy constructor and assignment operator sufficient
    
    Summary();
    ~Summary();  
    
    /**
     *  parse a directory to determine whether a valid corelet resides
     *  beneath.
     */
    bool detectCorelet(const bp::file::Path &path, std::string &error);
    
    typedef enum {
        None,
        Standalone,
        Dependent,
        Provider,
        BuiltIn
    } CoreletType;

    std::string name() const;
    std::string version() const;
    
    CoreletType type() const;
    std::string typeAsString() const;

    /** Path to the corelet directory */
    const bp::file::Path& path() const;

    /** last modification of the corelet */
    BPTime modDate() const;

    /** check if the on disk manifest is newer than when the summary
     *  was loaded */
    bool outOfDate() const;

    /** fetch the optional shutdown delay parameter that may be
     *  specified in the manifest.json of a service.  If -1 is returned,
     *  no such option was specified */
    int shutdownDelaySecs() const;
    
    /** get the locales for which this corelet is localized */
    std::list<std::string> localizations() const;

    /** get the localization for a specific locale */
    bool localization(const std::string &locale,
                      std::string &title, std::string &summary) const;

    /** set the localizations for the service */ 
    void setLocalizations(std::map<std::string,
                                   std::pair<std::string, std::string> > l);

    /** for a provider or standalone corelet, get the path to the
     *  dynamic library that implements the corelet */
    bp::file::Path coreletLibraryPath() const;

    /** for a dependent corelet, who does the corelet depend on? */
    std::string usesCorelet() const;
    /** for a dependent corelet, what version does the corelet depend on? */
    bp::ServiceVersion usesVersion() const;
    /** for a dependent corelet, what minVersion does the corelet depend on? */
    bp::ServiceVersion usesMinversion() const;

    /** for a dependent corelet, ??? */
    std::map<std::string, std::string> arguments() const;

    /** has this summary been initialized? */
    bool isInitialized() const;

    /** reset to uninitialized state */
    void clear();

    /** get the permission keys that are required by this service.  This
     *  information is packed into the manifests.json file. */
    std::set<std::string> permissions() const;

    std::string toHumanReadableString() const;

  protected:
    CoreletType m_type;

    std::string m_name;
    std::string m_version;
    bp::file::Path m_path;
    std::map<std::string, std::pair<std::string, std::string> >
        m_localizations;
    BPTime m_modDate;
    int m_shutdownDelaySecs;

    // specific to standalone or provider corelets
    bp::file::Path m_coreletLibraryPath;    

    // specific to dependent corelets
    std::string m_usesCorelet;
    bp::ServiceVersion m_usesVersion;
    bp::ServiceVersion m_usesMinversion;
    std::map<std::string, std::string> m_arguments;

    std::set<std::string> m_permissions;
};

/**
 * Test if lhs is bitwise less than rhs.
 * this function allows CoreletSummaries to be used as keys in STL data
 * structures.  Comparison is based on path.
 */
bool operator<(const bp::service::Summary &lhs,
               const bp::service::Summary &rhs);

/**
 * A class used by the CoreletManager library to generate summaries for
 * built-in corelets
 */
class BuiltInSummary : public Summary
{
  public:
    BuiltInSummary(const std::string & name, const std::string & version);
};

} }
 
#endif
