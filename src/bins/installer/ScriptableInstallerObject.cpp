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

#include "ScriptableInstallerObject.h"

#include <iostream>
#include <stdlib.h>
#include <assert.h>

using namespace std;

ScriptableInstallerObject::ScriptableInstallerObject()
    : m_listener(NULL), m_progress(0), m_state("started")
{
    // let's expose some functions
    m_so.mountFunction(this, "beginInstall");
    m_so.mountFunction(this, "cancelInstall");
    m_so.mountFunction(this, "state");
    m_so.mountFunction(this, "allDone");
}

ScriptableInstallerObject::~ScriptableInstallerObject()
{
}

bp::html::ScriptableObject *
ScriptableInstallerObject::getScriptableObject()
{
    return &m_so;
}

void
ScriptableInstallerObject::setStatus(const std::string & status)
{
    // never update state if we're in an error state
    if (0 != m_state.compare("error")) {
        m_desc = status;
    }
}

void
ScriptableInstallerObject::setProgress(int progress) 
{
    // never update state if we're in an error state
    if (0 != m_state.compare("error")) {
        if (progress < 100) {
            m_state = "installing";
        } else {
            m_state = "complete";            
        }
    }
    
    m_progress = progress;
}

void
ScriptableInstallerObject::setError(const std::string & localized,
                                    const std::string & details)
{
    m_state = std::string("error");
    m_desc = localized;
    m_errorDetails = details;
}

bp::Object *
ScriptableInstallerObject::invoke(const string & functionName,
                               unsigned int id,
                               vector<const bp::Object *> args)
{
    bp::Object * rv = NULL;

    if (!functionName.compare("state"))
    {
        bp::Map * m = new bp::Map;
        m->add("state", new bp::String(m_state));
        m->add("progress", new bp::Integer(m_progress));        
        if (!m_desc.empty()) m->add("desc", new bp::String(m_desc));        
        if (!m_errorDetails.empty()) {
            m->add("errorDetails", new bp::String(m_errorDetails));
        }
        rv = m;
    }
    else if (!functionName.compare("cancelInstall"))
    {
        if (m_listener) m_listener->cancelInstallation();
    }
    else if (!functionName.compare("beginInstall"))
    {
        if (m_listener) m_listener->beginInstall();
    }
    else if (!functionName.compare("allDone"))
    {
        if (m_listener) m_listener->shutdown();
    }

    return rv;
}


void
ScriptableInstallerObject::setListener(IInstallerSkinListener * listener)
{
    m_listener = listener;
}
