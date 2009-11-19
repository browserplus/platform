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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * A transaction is all required context associated with a single method
 * invocation.
 */

#include "BPTransaction.h"
#include "BPUtils/BPLog.h"


unsigned int BPTransaction::s_curTid = 777;

BPTransaction::BPTransaction()
    : m_bpProtoTransactionID(0),
      m_tid(s_curTid++),
      m_curCid(10000)
{
    BPLOG_DEBUG_STRM("transaction " << m_tid << " allocated");
}
    
BPTransaction::~BPTransaction()
{
    BPLOG_DEBUG_STRM("transaction " << m_tid << " freed");

    // release all callbacks
    CallbackMap::iterator it;
    
    for (it = m_callbacks.begin(); it != m_callbacks.end(); it++) 
    {
        delete it->second;
    }
}

unsigned int
BPTransaction::tid()
{
    return m_tid;
}
    
unsigned int
BPTransaction::addCallback(const plugin::Object * callback)
{
    if (callback == NULL) return 0;
    
    unsigned int cid = m_curCid++;
    
    m_callbacks[cid] = callback->clone();

    return cid;
}

plugin::Object*
BPTransaction::findCallback(long long int callbackHandle)
{
    CallbackMap::iterator it;
    
    it = m_callbacks.find(callbackHandle);

    if (it == m_callbacks.end()) return NULL;
    
    return it->second;
}
