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

#include "JSFunctionWrapper.h"

unsigned int JSFunctionWrapper::s_globalId = 10000;

JSFunctionWrapper::JSFunctionWrapper(void * osContext, void * osCallback)
    : m_id(s_globalId++), m_osContext(osContext), m_osCallback(osCallback)
{
    retain(m_osCallback);
}

JSFunctionWrapper::JSFunctionWrapper()
    : m_id(0), m_osContext(NULL), m_osCallback(NULL)
{
}

JSFunctionWrapper::JSFunctionWrapper(const JSFunctionWrapper & o)
{
    m_id = o.m_id;
    m_osContext = o.m_osContext;
    m_osCallback = o.m_osCallback;
    retain(m_osCallback);
}

JSFunctionWrapper & 
JSFunctionWrapper::operator=(const JSFunctionWrapper & o)
{
    m_id = o.m_id;
    if (m_osCallback) release(m_osCallback);
    m_osCallback = o.m_osCallback;
    m_osContext = o.m_osContext;
    if (m_osCallback) retain(m_osCallback);
    return *this;
}

JSFunctionWrapper::~JSFunctionWrapper()
{
    if (m_osCallback) release(m_osCallback);
}

unsigned int
JSFunctionWrapper::id()
{
    return m_id;
}
