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
 * ServiceVersion
 *
 * A Class for representing, comparing and parsing service versions
 */

#ifndef __BPSERVICEVERSION_H__
#define __BPSERVICEVERSION_H__

#include <string>

namespace bp {

/**
 * A Class for representing, comparing and parsing service versions
 */ 
class ServiceVersion
{
public:
    ServiceVersion();
    virtual ~ServiceVersion();    

    ServiceVersion& operator=(const ServiceVersion & in);

    bool parse(const char * version);
    bool parse(const std::string & version);

    int majorVer(void) const;
    void setMajor(int majorVersion);

    int minorVer(void) const;
    void setMinor(int minorVersion);

    int microVer(void) const;
    void setMicro(int microVersion);

    /** -1 if this version is less than other version
     *  0 if this version is equal to other version
     *  1 if this version is greater than other version
     */
    int compare(const ServiceVersion & otherVersion) const;

    /** do the two versions match, considering negative version numbers
     *  to be wildcards */
    bool match(const ServiceVersion & otherVersion) const;

    std::string asString(void) const;


    /**
     * A very specific, but common test which occurs all over.
     * When scanning a list of available services, how do we tell if
     * the 'current' version both matches the wanted 'version' and
     * 'minversion', AND is newer than the match we've already 'got'
     */
    static bool isNewerMatch(const ServiceVersion &current,
                             const ServiceVersion &got,
                             const ServiceVersion &wantver,
                             const ServiceVersion &wantminver);
private:
    int m_major;
    int m_minor;
    int m_micro;
};

};

#endif
