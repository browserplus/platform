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

// ScriptableConfigObject.  An object which encapsulates
// interaction with browserplus core, as well as various other
// functionality and can be mapped into a javascript execution
// environment.

#ifndef __SCRIPTABLECONFIGOBJECT_H__
#define __SCRIPTABLECONFIGOBJECT_H__

#include "HTMLRender/HTMLRender.h"
#include "BPProtocol/BPProtocol.h"

class ScriptableConfigObject : public bp::html::ScriptableFunctionHost
{
  public:
    ScriptableConfigObject();
    ~ScriptableConfigObject();    

    bp::html::ScriptableObject * getScriptableObject();
  private:
    bp::html::ScriptableObject m_so;

    std::string m_uri;
    BPProtoHand m_hand;
    bool m_dontReconnect;
    
    // Handle a user prompt request from the protocol library
    static void handleUserPrompt(void * cookie,
                                 const char * pathToHTMLDialog,
                                 const BPElement * arguments,
                                 unsigned int tid);
    
    // establishing a connection to BPCore
    static void connectCB(BPErrorCode ec, void * cookie,
                          const char * err, const char * verbErr);

    // after calling into the protocol library, if we receive an
    // error that suggests the connection has fall down, we'll initiate
    // a reconnect
    void connectIfNeeded(BPErrorCode ec);

    // landing place for a callback from the protocol library
    static void genericProtocolCallback(BPErrorCode ec, void * cookie,
                                        const BPElement * response);

    // dispatch function for when javascript calls into us
    virtual bp::Object * invoke(const std::string & functionName,
                                unsigned int id,
                                std::vector<const bp::Object *> args); 
    
    bool isInstalled();
    bool isEnabled();
};

#endif
