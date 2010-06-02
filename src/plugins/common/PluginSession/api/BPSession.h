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

/*
 *  BPSession.h
 *
 *  Created by Lloyd Hilaiel
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __BPSESSION_H__
#define __BPSESSION_H__

#include <string>
#include <list>
#include "BPUtils/bpthreadhopper.h"
#include "BPProtocol/BPProtocol.h"
#include "PluginCommonLib/BPPlugin.h"
#include "PluginCommonLib/BPTransaction.h"
#include "PluginCommonLib/PluginObject.h"
#include "PluginCommonLib/PluginVariant.h"
#include "PluginCommonLib/PlugletRegistry.h"

/**
 * The BPSession object is where all the heavy lifting occurs in the
 * browser plugins.  There is one BPSession per plugin instance (browser js
 * context using browserplus).  
 */
class BPSession
{
  public:
    BPSession(BPPlugin* pPlugin);
    ~BPSession();

    // initialize the plugin, check permissions, etc.
    bool initialize(const plugin::Variant* arguments,
                    const plugin::Object* callback,
                    plugin::Variant* result);

    // invoke a method on a service
    bool executeMethod(const std::string& service,
                       const std::string& version,
                       const std::string& method,
                       const plugin::Variant* arguments,
                       const plugin::Object* callback, 
                       plugin::Variant* result);

    // enumerate the available services, both pluglets and services.
    bool enumerateServices(const plugin::Object* callback,
                           plugin::Variant* result);

    // load services, returning the description of those services to
    // javascript
    bool requireServices(const plugin::Variant* args,
                         const plugin::Object* callback,
                         plugin::Variant* result);

    // attain the description of an active (but perhaps not loaded)
    // service
    bool describeService(const plugin::Variant* args,
                         const plugin::Object* callback,
                         plugin::Variant* result);


    // The next 3 methods currently have to be public due to non-member
    // func generateResult in BPSessionInvoke.cpp.
    bool generateErrorReturn( const char* cszError,
                              const char* cszVerboseError,
                              plugin::Variant* pvtRet ) const;

    bool generateSuccessReturn( const bp::Object& value,
                                plugin::Variant* pvtRet ) const;

    BPPlugin & plugin() const;

    PlugletRegistry * getPlugletRegistry();
    
  private:
    // the first thing that the webpage must call is "Initialize",
    // this function will generate the correct return value
    bool notInitialized(const plugin::Object* callback,
                        plugin::Variant* result);

    // invoke a javascript callback with an error
    bool invokeCallbackWithError(const char * error,
                                 const char * verboseError,
                                 const plugin::Object* callback);
      
    // functions which implement ExecuteMethod
    static void pletFailureCB(void * cookie, unsigned int tid,
                              const char * error, const char * verboseError);
    static void pletSuccessCB(void * cookie, unsigned int tid,
                              const bp::Object * returnValue);
    static void pletCallbackCB(void * cookie, unsigned int tid,
                               BPCallBack methodID,
                               const bp::Object * args);

    static void executeMethodInvokeCallbackRelayCB(void * cookie);
    static void executeMethodInvokeCallbackCB(void * cookie,
                                              unsigned int tid,
                                              long long int cbHand,
                                              const BPElement * params);
    static void executeMethodRelayResultsCB(void * cookie);
    static void executeMethodResultsCB(void * cookie,
                                       unsigned int tid,
                                       BPErrorCode ec,
                                       const BPElement * results);

    // Callback invoked by protocol library when enumerate is complete
    static void enumerateServicesCallback(BPErrorCode ec,
                                          void * cookie,
                                          const BPElement * services);

    // functions which implement RequireServices
    static void requireReturn(void * cookie);
    static void requireServicesCallback(BPErrorCode ec,
                                        void * cookie,
                                        const BPServiceDefinition ** defs,
                                        unsigned int numDefs,
                                        const char * error,
                                        const char * verboseError);

    // functions which implement describeService
    static void describeReturn(void * cookie);
    static void describeServiceCallback(BPErrorCode ec,
                                        void * cookie,
                                        const BPServiceDefinition * def,
                                        const char * error,
                                        const char * verboseError);

    // Callback invoked by protocol library when connect is complete
    static void connectCallback(BPErrorCode ec, void * cookie,
                                const char * error,
                                const char * verboseError);
    // A proxy function used to invoke js callback after function
    // returns in the case where we're already connected
    static void alreadyConnected(void * cookie);

    
    // search the list to see if a service is already loaded
    bool findLoadedService(const std::string & service,
                           const std::string & version,
                           const std::string & minversion,
                           bp::service::Description & oDescription);

    // add a transaction.  BPSession now takes ownership of this
    // memory, the transaction will be deleteed when it's removed, or
    // when BPSession is destructed, whichever comes first.
    void addTransaction(BPTransaction * t);
    void removeTransaction(BPTransaction * t);
    BPTransaction * findTransactionByProtoID(unsigned int id);    
    BPTransaction * findTransactionByTID(unsigned int id);    
    
    // traverse an argument from javascript, perform callback caching and
    // handle de-obfuscation, return a bp::Object representation.
    // callbacks will be stored on the transaction object, and will be
    // replaced with unique integer handles.
    bool variantToBPObject( const plugin::Variant* pvtInput,
                            BPTransaction* pTransaction,
                            bp::Object*& rpoOut ) const;

    // Handle a user prompt request from the protocol library, relaying
    // to the correct thread
    static void handleUserPrompt(void * cookie,
                                 const char * pathToHTMLDialog,
                                 const BPElement * arguments,
                                 unsigned int tid);

    // Execute a user prompt request displaying native UI to the end user
    static void executeUserPrompt(void * cookie);
    
    // handle the end user response to prompt, relay results back to
    // protocol library
    static void deliverUserPromptResults(void * cookie,
                                         const bp::Object * response);

    /////////////////////////
    // Attributes
    std::string m_name;
    
    BPPlugin * m_plugin;

    bool m_initialized;

    PlugletRegistry * m_plugletRegistry;

    bp::thread::Hopper m_threadHopper;

    BPProtoHand m_protoHand;

    // a list of the descriptions of loaded ("required") services
    // should only be accessed from the main UI thread (not from protocol
    // callbacks)
    std::list<bp::service::Description> m_loadedServices;

    // Currently running transactions 
    std::list<BPTransaction *> m_activeTransactions;
    
    // Queue of user prompts
    std::list<void *> m_prompts;
};


inline BPPlugin & BPSession::plugin() const
{
    // TODO: check/log could happen here
    return *m_plugin;
}



#endif
