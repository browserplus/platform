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

#include "PlugletRegistry.h"
#include "BPUtils/bperrorutil.h"
// XXX
#include "BPUtils/BPLog.h"

PlugletRegistry::PlugletRegistry()
{
}

PlugletRegistry::~PlugletRegistry()
{
    std::list<Pluglet *>::iterator it;
    for (it = m_pluglets.begin(); it != m_pluglets.end(); it++) 
    {
        delete *it;
    }
}

bool
PlugletRegistry::registerPluglets(std::list<Pluglet *> pluglets)
{
    if (pluglets.empty())
    {
        return false;
    }
        
    std::list<Pluglet*>::iterator it;
    for (it = pluglets.begin(); it != pluglets.end(); ++it) 
    {
        const bp::service::Description* d = (*it)->describe();
        std::string n = d->name();
        std::string v = d->versionString();
        BPLOG_INFO_STRM("register pluglet " << n << " / " << v);
        m_pluglets.push_back(*it);
    }
    return true;
}

Pluglet *
PlugletRegistry::find(std::string name, std::string wantverStr,
                      std::string wantminverStr)
{
    Pluglet * pluglet = NULL;

    // the version pattern we want
    bp::ServiceVersion wantver;
    // the minimum version we want
    bp::ServiceVersion wantminver;
    // the version we've found
    bp::ServiceVersion got;

    if (!wantver.parse(wantverStr.c_str()) ||
        !wantminver.parse(wantminverStr.c_str()))
    {
        return NULL;
    }

    std::list<Pluglet *>::iterator it;
    for (it = m_pluglets.begin(); it != m_pluglets.end(); it++) 
    {
        const bp::service::Description * desc = (*it)->describe();
        BPASSERT(desc != NULL);
        
        if (name.compare(desc->name())) continue;
        
        bp::ServiceVersion current;            
        current.setMajor(desc->majorVersion());
        current.setMinor(desc->minorVersion());
        current.setMicro(desc->microVersion());

        // is this a newer match than what we've already got?
        if (!bp::ServiceVersion::isNewerMatch(current, got,
                                              wantver, wantminver))
        {
            continue;
        }

        // passed our tests! 
        pluglet = *it;
        got = current;
    }

    return pluglet;
}

    
std::list<Pluglet *>
PlugletRegistry::availablePluglets()
{
    return m_pluglets;
}
