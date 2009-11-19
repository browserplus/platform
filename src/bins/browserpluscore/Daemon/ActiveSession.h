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
 * ActiveSession.h
 * Command handlers for presence server
 *
 * Created by Lloyd Hilaiel on or around Wed May  9 17:39:04 MDT 2007
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 *
 * Owner: Lloyd Trevor Hilaiel (LtH) 
 */

#ifndef __ACTIVESESSION_H__
#define __ACTIVESESSION_H__

#include "BPUtils/BPLog.h"
#include "BPUtils/bpfile.h"
#include "CoreletManager/CoreletManager.h"
// for LocalizedCoreletDescriptionPtr, used when display component
// installation prompt.
#include "DistributionClient/DistributionClient.h"
#include "RequireRequest.h"
#include "Permissions/PermissionsManager.h"


class IActiveSessionListener
{
  public:
    virtual void onSessionEnd(bp::ipc::Channel * channel) = 0;
    virtual ~IActiveSessionListener() { };
};

class ActiveSession : public virtual RequireLock::ILockListener,
					  public virtual bp::ipc::IChannelListener,
                      public virtual CoreletExecutionContext,
                      public virtual IPermissionsManagerListener,
                      public virtual ICoreletInstanceListener,
                      public virtual RequireRequest::IRequireListener,
                      // this is only necc because ActiveSession does
                      // prompting.  We should be able to move ALL prompting
                      // to RequireRequest.
                      public virtual ICoreletExecutionContextListener,
                      public virtual ICoreletRegistryListener,
                      public std::tr1::enable_shared_from_this<ActiveSession>
{
  public:
    ActiveSession(bp::ipc::Channel * session,
                  std::tr1::shared_ptr<CoreletRegistry> registry,
                  const std::string & primaryDistroServer,
                  std::list<std::string> distroServers);
    virtual ~ActiveSession();

    // no assignment semantics, declared but not defined
    ActiveSession(const ActiveSession&);
    ActiveSession& operator=(const ActiveSession&);

    /** attain the locale of this session */
    virtual std::string locale();

    /** attain the URI of the client that initiated this session */
    virtual std::string URI();

    /** attain the user agent of the client that initiated this session */
    virtual std::string userAgent();
    
    /** attain the process id of the client that initiated this session */
    virtual long clientPid();
    
    /** attain the domain of the client that initiated this session */
    std::string domain();

    /** attain names and versions of corelet instances held by this
     *  session */
    std::vector< std::pair<std::string, std::string> > instances();
    
    /** get transient (non-persistent) permission */
    PermissionsManager::Permission transientPermission(const std::string& perm);
    void setTransientPermission(const std::string& perm, bool val);

    /** Overrides from CoreletExecutionContext */

    /** display an installation promt to the user.  This should instruct the client
     *  to display a modal dialog which causes the user to agree or
     *  disagree.
     *
     *  The results of the user interaction will be a posted event,
     *  either ActiveSession::UserConfirmEvent or
     *  ActiveSession::UserDenyEvent to the listener argument.
     *
     *  \param cookie - client state.  The client may pass in an integer
     *                  which will be set as the eventParameter of the
     *                  event which is raised which contains the results
     *                  of the user interaction.
     *  \param permissions - any domain/service permissions needed (localized)
     *  \param componentsToInstall - a localized description of all the
     *           components to install (see DistQueryTypes.h)
     */
    void displayInstallPrompt(
        std::tr1::weak_ptr<ICoreletExecutionContextListener> listener,
        unsigned int cookie,
        const std::vector<std::string>& permissions,
        const ServiceSynopsisList & platformUpdates,
        const ServiceSynopsisList & componentsToInstall);
                             
    virtual void promptUser(
        std::tr1::weak_ptr<ICoreletExecutionContextListener> listener,
        unsigned int cookie,
        const bp::file::Path& pathToHTMLDialog,
        const bp::Object * obj);
    
    virtual void invokeCallback(unsigned int tid,
                                const bp::Map* cbInfo);
    
    std::tr1::shared_ptr<CoreletRegistry> registry(); 

    void setListener(IActiveSessionListener * listener) {
        m_listener = listener;
    }
 
  private:
    // Messages may require domain permission validation.
    // A MessageContext keeps track of the request, and the
    // "doXXX()" handlers do the actual work after validation
    // is performed
    class MessageContext;
    typedef bool (ActiveSession::*tHandler)(MessageContext*);
    
    class MessageContext {
      public:
        MessageContext(tHandler h, bp::ipc::Channel * session,
                       const bp::ipc::Query & query,
                       const bp::ipc::Response& response) 
            : m_func(h), m_session(session),
              m_query(query), m_response(response), m_perms(), 
              m_cookie(0), m_isMessage(false)
        {
        }

        // copying/assignment supported via compiler generated fuctions.
        
        ~MessageContext() {
        }
        void sendResponse() {
            if (!m_session->sendResponse(m_response))
            {
                BPLOG_WARN_STRM("[" << m_session << "] " <<
                                "failed to send message");
            }
        }
            
        // "Messages" do not accept responses.  They are run through the
        // same logic that "Queries" are, for permission checks.
        // For messages, no errors are returned.
        tHandler m_func;
        bp::ipc::Channel * m_session;
        bp::ipc::Query m_query;
        bp::ipc::Response m_response;
        std::vector<std::string> m_perms;
        unsigned int m_cookie;
        bool m_isMessage;
    };
    
    // Get an smm message, check needed perms, and process
    bool dispatchMessage(tHandler func,
                         const std::vector<std::string>& perms,
                         bp::ipc::Channel * session,
                         const bp::ipc::Query & query,
                         bp::ipc::Response & response);

    // implementation of IChannelListener::onQuery(), first places where
    // incoming IPC queries arrive.  Performs type specific setup work,
    // then passes to dispatchMessage.
    virtual bool onQuery(bp::ipc::Channel *,
                         const bp::ipc::Query & query,
                         bp::ipc::Response & response);
    
    void doNextDispatch();
    
    bool doInvoke(MessageContext* ctx);
    bool doRequire(MessageContext* ctx);
    bool doDescribe(MessageContext* ctx);
    bool doHave(MessageContext* ctx);
    bool doActiveServices(MessageContext* ctx);
    bool doGetState(MessageContext* ctx);
	bool doSetState(MessageContext* ctx);

    // Messages which seem to require user prompting are queued and use the
    // RequireLock.  This ensures that only one user prompt is displayed at once
    std::list<MessageContext*> m_messages;
    
    // invoked when a response to a installation prompt is received
    // implemented from ICoreletExecutionContextListener to handle
    // prompt responses.
    void onUserResponse(unsigned int cookie, const bp::Object & resp);

    // invoked when a service function executes to completion
    void executionComplete(unsigned int tid,
                           const bp::Object & results);

    // invoked when a service function fails
    void executionFailure(
        unsigned int tid, const std::string & error,
        const std::string & verboseError);

    // implemented methods from IPermissionsManagerInterface
    void gotUpToDate();
    void cantGetUpToDate();

    // implemented from RequireLock::ILockListener 
    void gotRequireLock();

    // implemented from RequireRequest::IListener 
    void onComplete(unsigned int tid, const bp::List & satisfyingServices);
    void onFailure(unsigned int tid,
                   const std::string & error,
                   const std::string & verboseError);

    bp::ipc::Channel * m_session;
    std::tr1::shared_ptr<CoreletRegistry> m_registry;

    // a comparator for complex keys consisting of two
    // std::strings in a std::pair
    struct TwoStringCompare {
        bool operator()(std::pair<std::string, std::string> lhs,
                        std::pair<std::string, std::string> rhs) const;
    };

    // a map containing corelet instances associated with this session
    std::map<std::pair<std::string, std::string>,
        std::tr1::shared_ptr<CoreletInstance>,
        TwoStringCompare> m_instanceMap;

    // a map containing function invocations pending instance allocation
    struct PendingExecution
    {
        // the transaction id of the invocation
        unsigned int tid;
        // the name of the function to invoke
        std::string function;
        // dynamically allocated arguments
        bp::Object * args; 

        PendingExecution() : tid(0), args(NULL) { }
    };

    std::map<std::pair<std::string, std::string>,
        std::pair<unsigned int, std::vector<PendingExecution> > > 
        m_pendingExecutions;

    // invoked by CoreletRegistry upon successful service instance
    // allocation
    void gotInstance(unsigned int allocationId,
                     std::tr1::shared_ptr<CoreletInstance> instance);

    void doExecution(std::tr1::shared_ptr<CoreletInstance> instance,
                     unsigned int tid,
                     const std::string & function,
                     const bp::Object * args);

    // a function to grab an instance, we will look for it in the
    // map, adding it if it does not exist.  Instances spring into existance
    // upon first usage
    // primarily to abstract the confusing STL syntax here.
    std::tr1::shared_ptr<CoreletInstance> attainInstance(const std::string & corelet,
                                                      const std::string & version);

    // send a prompt user protocol message to the client.  This function
    // gaurantees that the listener will be signalled.  if the
    // message is malformed, this will be considered user refusal and
    // the error will be logged.
    unsigned int sendPromptUserMessage(
        std::tr1::weak_ptr<ICoreletExecutionContextListener> listener,
        unsigned int cookie, const bp::Object * o);

    // when a response is recieved to a prompt user message, this
    // function will process the response and send the results to
    // the appropriate corelet instance
    void handlePromptUserResponse(const bp::ipc::Response & resp);

    // a map holding outstanding eula / prompt user requests for this session.
    // the map is IPC Query ids - > listeners
    std::map<unsigned int,
             std::pair<std::tr1::weak_ptr<ICoreletExecutionContextListener>
                       , unsigned int> > m_promptRequests;
    
    // a list of outstanding require requests for this session
    std::list<std::tr1::shared_ptr<RequireRequest> > m_requireRequests;

    // uri is the URI of the client application, it is set at
    // createSession time 
    std::string m_URI;

    // locale is the locale of the client.  the locale to which end
    // user readable text should be localized.  It is set at
    // createSession time
    std::string m_locale;

    // useragent is a string identifying the client
    std::string m_userAgent;

    // processId of connected client
    long m_clientPid;
    
    std::map<std::string, bool> m_transientPermissions;
    
    // The "createSession" message may require that we check with
    // the distribution server.  Save the original message so that
    // we can respond.
    bp::ipc::Query * m_sessionMessage;

    // prevent the client from calling createSession twice
    bool m_createSessionCalled;
    
    // check permissions
    std::string checkPermissions();
    
    PermissionsManager::Permission
        checkDomainPermission(const std::string& permission);

    virtual void channelEnded(
        bp::ipc::Channel * c,
        bp::ipc::IConnectionListener::TerminationReason why,
        const char * errorString);

    virtual void onMessage(bp::ipc::Channel * c,
                           const bp::ipc::Message & m);

    virtual void onResponse(bp::ipc::Channel * c,
                            const bp::ipc::Response & response);

    IActiveSessionListener * m_listener;

    // a boost weak pointer populated in the ctor which is used
    // to ensure we clear out the RequireLock upon destruction
    std::tr1::weak_ptr<class ActiveSession> m_thisWeak;

    // Handle the fact that IE and FF report non-ascii urls differently.
    std::string normalizeClientURI( const std::string& uri);
    const std::string & m_primaryDistroServer;
    std::list<std::string> m_secondaryDistroServers;
};

#endif
