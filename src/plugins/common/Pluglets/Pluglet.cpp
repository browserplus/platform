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

#include "Pluglet.h"
#include "BPPlugin.h"

Pluglet::Pluglet(BPPlugin * plugin,
                 const bp::service::Description& desc)
    : m_plugin(plugin), m_desc(desc)
{
}

Pluglet::~Pluglet()
{
}

std::string 
Pluglet::locale()
{
    if (m_locale.empty()) {
        setLocale("en-US");
    }
    return m_locale;
}

void 
Pluglet::setLocale(const std::string& locale)
{
    m_locale = locale;
}


const bp::service::Description *
Pluglet::describe()
{
    return &m_desc;
}


