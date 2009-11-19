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
 * SessionCreator.h -- An abstraction resposible for establishing a
 *                     connection with the core daemon.  This involves
 *                     spawning the daemon if not running, connecting,
 *                     and executing a CreateSession protocol message
 *                     which relays the URI to the daemon and gives
 *                     a chance for the daemon to pass errors.
 */

#ifndef __SESSIONCREATOR_H__
#define __SESSIONCREATOR_H__

#include <string>

#include "BPProtocolInterface.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bptimer.h"
#include "BPUtils/IPCChannel.h"

class ISessionCreatorListener 
{
  public:
    virtual void sessionCreated() = 0;
    virtual void sessionCreationFailed(BPErrorCode e,
                                       const std::string & errorString,
                                       const std::string & verboseError) = 0;
    virtual ~ISessionCreatorListener() { }
};

class SessionCreator : virtual public bp::time::ITimerListener,
                       virtual public bp::ipc::IChannelListener
{
public:
    // allocate a session creator given a specific channel.  the lifetime
    // of the channel must exceed that of SessionCreator and
    // the client retains ownership of the channel
    SessionCreator(bp::ipc::Channel * channel);
    ~SessionCreator();

    // specify the class to notify when events occur in session creation
    void setListener(ISessionCreatorListener * listener);

    // begin the session creation process
    void createSession(const char * uri, const char * locale,
                       const char * userAgent);

    // cancel session creation, WILL NOT disconnect the channel
    void cancel();

private:
    /** The initial poll period */
    static const double c_initialPollPeriodS;
    /** The maximum poll period */
    static const double c_maxPollPeriodS;
    /** The poll period growth rate */
    static const double c_pollPeriodGrowthRate;
    /** The maximum amout of time we'll spend trying to connect */
    static const double c_maxWaitS;    
    static const char * c_createSessionCommand;

    // errorString may be supplied when e == BP_EC_EXTENDED error to
    // return extended error information
    void reportError(BPErrorCode e, const std::string & verboseError,
                     const char * errorString = NULL);

    bp::ipc::Channel * m_channel;

    // a bit of state:  If we cannot directly attempt to the daemon, we
    // assume it is not running and spawn it.  This prevents us from
    // trying to spawn twice.
    bool m_spawnedProcess;

    // another bit of state:  When timer expires we can either be waiting
    // to retry connection OR for BrowserPlusCore to respond to our
    // CreateSession message
    bool m_sentCreateSession;

    bp::time::Stopwatch m_elapsedTime;
    bp::time::Timer m_timer;
    bp::process::spawnStatus m_spawnStatus;
    std::string m_uri;
    std::string m_locale;
    std::string m_userAgent;

    /* the current poll period */
    double m_curPollS;

    ISessionCreatorListener * m_listener;

    /* get the poll period and handle scaling it */
    double getPollPeriodSec();

    /* Attempt a connection */
    void tryConnect();

    // invoked when our timer expires, this means either it's time to
    // try to reconnect, or give up if too much time has elapsed
    void timesUp(bp::time::Timer * t);
    
    /* channel events from IChannelListener */ 
    // we care about if the channel falls down in the middle of the process
    virtual void channelEnded(
        bp::ipc::Channel * c,
        bp::ipc::IConnectionListener::TerminationReason why,
        const char * errorString);

    // invoked when a message is recieved from the channel,
    // this is a protocol error, we expect no messages
    // during initial connection phase.
    virtual void onMessage(bp::ipc::Channel * c,
                           const bp::ipc::Message & m);

    // proto error, we expect no queries during intial connection phase.
    virtual bool onQuery(bp::ipc::Channel * c,
                         const bp::ipc::Query & query,
                         bp::ipc::Response & response);

    // invoked when we recieve our response to the initial CreateSession
    // message which relays the URI and locale to the daemon.
    virtual void onResponse(bp::ipc::Channel * c,
                            const bp::ipc::Response & response);
};

#endif
