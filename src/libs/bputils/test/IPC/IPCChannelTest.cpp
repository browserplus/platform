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
 * IPCChannelTest.h
 * Unit tests for the IPC channel abstraction
 *
 * Created by Lloyd Hilaiel on 7/30/08 (somewhere over the atlantic)
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "IPCTestServer.h"
#include "IPCChannelTest.h"


CPPUNIT_TEST_SUITE_REGISTRATION(IPCChannelTest);

// a simple channel listener which calls stop on a runloop when the
// channel it's listening to falls down
class MyChannelListener : public bp::ipc::IChannelListener
{
public:
    MyChannelListener(bp::runloop::RunLoop * rl)
        : m_why(bp::ipc::IConnectionListener::InternalError), m_rl(rl) { }
    
    void channelEnded(bp::ipc::Channel *,
                      bp::ipc::IConnectionListener::TerminationReason why,
                      const char *)
    {
        m_why = why;
        m_rl->stop();
    }
    
    virtual void onMessage(bp::ipc::Channel *,
                           const bp::ipc::Message &)
    {
    }
    
    virtual bool onQuery(bp::ipc::Channel *,
                         const bp::ipc::Query &,
                         bp::ipc::Response &)
    {
        return false;
    }
                               
    virtual void onResponse(bp::ipc::Channel * c,
                            const bp::ipc::Response & r)
    {
        if (!r.command().compare("echo")) {
            if (r.payload()) {
                if (r.payload()->type() == BPTString) {
                    if (!((std::string) *r.payload()).compare("quit"))
                        m_rl->stop();
                } else if (r.payload()->type() == BPTInteger) {
                    long long val = (long long) *(r.payload());
                    if (val < 500) {
                        bp::ipc::Query q;
                        q.setCommand("echo");
                        q.setPayload(bp::Integer(val + 1));
                        CPPUNIT_ASSERT( c->sendQuery(q) );
                    } else {
                        m_rl->stop();                        
                    }
                }
            } 
        }
    }

    // accessible
    bp::ipc::IConnectionListener::TerminationReason m_why;
private:
    bp::runloop::RunLoop * m_rl;
};

void
IPCChannelTest::stopTest()
{
    using namespace bp;

    // allocate a runloop
    runloop::RunLoop rl;

    rl.init();

    // allocate a server
    IPCTestServer server;

    // allocate a client
    MyChannelListener listener(&rl);
    {
        ipc::Channel c;
        c.setListener(&listener);

        // now connect the client
        CPPUNIT_ASSERT( c.connect(server.location()) );

        // now let's send a "stop" string to be echo'd back to us.
        // when we get the response, we'll stop the runloop
        bp::ipc::Query q;
        q.setCommand("echo");
        q.setPayload(bp::String("quit"));
        CPPUNIT_ASSERT( c.sendQuery(q) );

        rl.run();
    }
}

void
IPCChannelTest::fiveHundredTest()
{
    using namespace bp;

    // allocate a runloop
    runloop::RunLoop rl;

    rl.init();

    // allocate a server
    IPCTestServer server;

    // allocate a client
    MyChannelListener listener(&rl);
    {
        ipc::Channel c;
        c.setListener(&listener);

        // now connect the client
        CPPUNIT_ASSERT( c.connect(server.location()) );

        // now let's send an integer to be echo'd back to us.
        // when we get the response, we'll increment the count and resend
        // the message.  When the count gets to 500 we'll call it quits
        bp::ipc::Query q;
        q.setCommand("echo");
        q.setPayload(bp::Integer(0));
        CPPUNIT_ASSERT( c.sendQuery(q) );

        rl.run();
    }
}

void
IPCChannelTest::peerTerminatedTest()
{
    using namespace bp;

    // allocate a runloop
    runloop::RunLoop rl;

    rl.init();

    // allocate a client listener (which will stop the runloop when
    // it detects disconnect)
    MyChannelListener listener(&rl);
    {
        ipc::Channel c;
        c.setListener(&listener);

        {
            // allocate and start a server
            IPCTestServer server;
            // now connect the client
            CPPUNIT_ASSERT( c.connect(server.location()) );
        }
        // server is now destroyed, ensure our listener kicks us out of
        // the runloop
        rl.run();

        // and that the termination reason is appropriate
        CPPUNIT_ASSERT_EQUAL( listener.m_why,
                              bp::ipc::IConnectionListener::PeerClosed );
    }
}

// a simple channel listener which calls stop on a runloop when the
// channel it's listening to falls down
class MyChannelListener2 : public bp::ipc::IChannelListener
{
public:
    MyChannelListener2(bp::runloop::RunLoop * rl) : m_rl(rl), sum(0) { }
    
    void addChannel(bp::ipc::Channel * c) 
    {
        m_msgCount[c] = 0;
    }

    void channelEnded(bp::ipc::Channel * c,
                      bp::ipc::IConnectionListener::TerminationReason why,
                      const char * str)
    { 
		std::cout << "Channel ended: " << c << ": " << why << " " << (str ? str : "unknown") << std::endl;
	}
    
    virtual void onMessage(bp::ipc::Channel *,
                           const bp::ipc::Message &)
    { }
    
    virtual bool onQuery(bp::ipc::Channel *,
                         const bp::ipc::Query &,
                         bp::ipc::Response &)
    { return false; }
                               
    virtual void onResponse(bp::ipc::Channel * c,
                            const bp::ipc::Response & r)
    {
        std::map<bp::ipc::Channel *, int>::iterator it;
        it = m_msgCount.find(c);
        CPPUNIT_ASSERT( it != m_msgCount.end() );        
        it->second++;

        CPPUNIT_ASSERT( !r.command().compare("echo") );
        CPPUNIT_ASSERT( r.payload() != NULL );        
        CPPUNIT_ASSERT( r.payload()->type() == BPTInteger );        

        sum++;
        
        long long val = (long long) *(r.payload());

        // verify the count is equal to the number of responses received
        CPPUNIT_ASSERT_EQUAL(it->second, (int) val);
        
        if (val < 400) {
            bp::ipc::Query q;
            q.setCommand("echo");
            q.setPayload(bp::Integer(val + 1));
            CPPUNIT_ASSERT( c->sendQuery(q) );
        }
        if (sum >= 4000) m_rl->stop();                        
    }

private:
    // validation.  a collection of channels and the number of responses
    // we've received on each channel
    std::map<bp::ipc::Channel *, int> m_msgCount;

    bp::runloop::RunLoop * m_rl;
    int sum;
};

void
IPCChannelTest::tenTimesFourHundredTest()
{
    using namespace bp;

    // allocate a runloop
    runloop::RunLoop rl;

    rl.init();

    // allocate a server
    IPCTestServer server;

    // allocate a client
    MyChannelListener2 listener(&rl);
    {
        ipc::Channel c[10];
        // now connect the clients
        for (int i = 0; i < 10; i++)  {
			listener.addChannel(c+i);
            c[i].setListener(&listener);
            std::string errBuf;
            bool connected = c[i].connect(server.location(), &errBuf);
            CPPUNIT_ASSERT_MESSAGE( errBuf, connected );
            // lth: hack!  The way windows named pipes are implemented,
            //      it doesn't seem possible to avoid a temporary period
            //      after connecting a pipe where a 231 error would
            //      be returned "all named pipe instances are busy",
            //      A 20ms sleep allows the named pipe server thread to
            //      allocate and "connect" a new listening pipe.  very
            //      silly.
#ifdef WIN32
            Sleep(20);
#endif            
            bp::ipc::Query q;
            q.setCommand("echo");
            q.setPayload(bp::Integer(1));
            CPPUNIT_ASSERT( c[i].sendQuery(q) );
        }
        
        rl.run();
    }
}
