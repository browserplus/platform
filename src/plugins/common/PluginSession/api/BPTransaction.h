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
 * A transaction is all required context associated with a single method
 * invocation.
 */

#ifndef __BPTRANSACTION_H__
#define __BPTRANSACTION_H__

#include <map>
#include "PluginObject.h"

class BPTransaction
{
  public:
    typedef long long int CallbackId;
        
    BPTransaction();
    ~BPTransaction();    

    unsigned int tid();
    
    // in the case of a transaction using BPProtocol (corelet, not pluglet)
    // we'll keep the protocol transaction id here, for future cancelation.
    unsigned int m_bpProtoTransactionID;

    // add a callback to the transaction.  It will be retained, and a
    // unique (transaction scoped) integer handle will be returned.
    // The callback will be released when the BPTransaction is deleted.
    unsigned int addCallback(const plugin::Object * callback);

    plugin::Object* findCallback(CallbackId callbackHandle);
    
  private:
    unsigned int m_tid;

    // all callbacks recieve unique IDs.
    unsigned int m_curCid;    
    
    static unsigned int s_curTid;

    typedef std::map<CallbackId, plugin::Object *> CallbackMap;
    CallbackMap m_callbacks;
};

#endif
