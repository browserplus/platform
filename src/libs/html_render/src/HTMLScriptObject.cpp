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

/*
 * HTMLWindow.h - an abstraction around running a window which displays
 *                HTML and exposes scriptable functions into the
 *                running javascript context. 
 */

#include "api/HTMLScriptObject.h"
#include "HTMLTransactionContext.h"

using namespace bp::html;

unsigned int ScriptableFunctionHost::s_uniqueId = 10;

ScriptableObject::ScriptableObject()
    : m_osSpecific(NULL)
{
}

bool
ScriptableObject::mountFunction(
    ScriptableFunctionHost * host,
    const std::string & functionName)
{
    if (host == NULL || functionName.empty()) return false;
    if (m_functions.find(functionName) != m_functions.end()) return false;
    m_functions[functionName] = host;
    return true;
}

ScriptableFunctionHost *
ScriptableObject::findFunctionHost(const std::string& functionName) const
{
    std::map<std::string, ScriptableFunctionHost *>::const_iterator it;
    it = m_functions.find( functionName );

    return it == m_functions.end() ? NULL : it->second;
}

const std::map<std::string, ScriptableFunctionHost *>&
ScriptableObject::functionMap() const
{
    return m_functions;
}

bp::Object *
ScriptableFunctionHost::invokeCallback(unsigned int id,
                                       bp::CallBack cb,
                                       std::vector<const bp::Object *> args)
{
    // first let's find the transaction
    std::map<unsigned int, bp::html::BPHTMLTransactionContext *>::iterator it;
    it = m_transactions.find(id);
    if (it != m_transactions.end()) {
        // now let's see if this callback exists
        std::map<unsigned int, JSFunctionWrapper>::iterator cit;
        cit = it->second->m_callbacks.find((unsigned int)((long long) cb));
        if (cit != it->second->m_callbacks.end()) {        
            return cit->second.invoke(args);
        }
    }

    return NULL;
}

void
ScriptableFunctionHost::retain(unsigned int id)
{
    std::map<unsigned int, bp::html::BPHTMLTransactionContext *>::iterator it;
    it = m_transactions.find(id);
    if (it != m_transactions.end()) it->second->refCount++;
}

void
ScriptableFunctionHost::release(unsigned int id)
{
    std::map<unsigned int, bp::html::BPHTMLTransactionContext *>::iterator it;
    it = m_transactions.find(id);
    if (it != m_transactions.end()) {
        if (!(--it->second->refCount)) {
            delete it->second;
            m_transactions.erase(it);
        }
    }
}

unsigned int
ScriptableFunctionHost::addTransaction(bp::html::BPHTMLTransactionContext * t)
{
    unsigned int tid = s_uniqueId++;
    t->refCount = 1;
    m_transactions[tid] = t;
    return tid;
}
