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
 * MessageTest.h
 * Unit tests for the simple IPC Message representation.
 *
 * Created by Lloyd Hilaiel on 7/30/08. (somewhere over the atlantic)
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "MessageTest.h"
#include "BPUtils/IPCMessage.h"


CPPUNIT_TEST_SUITE_REGISTRATION(MessageTest);

void
MessageTest::basicMessageTest()
{
    bp::ipc::Message m;

    // see if we can set the command
    m.setCommand("foo");
    CPPUNIT_ASSERT( !std::string("foo").compare(m.command()) );

    // see if we can set the payload
    {
        bp::List l;
        l.append(new bp::Integer(77));
        l.append(new bp::String("hi mom"));    
        l.append(new bp::Null);
        m.setPayload(l);
    }
    
    const bp::Object * payload = m.payload();
    CPPUNIT_ASSERT_EQUAL( (long long) (*payload)[(unsigned)0], (long long) 77 );
    CPPUNIT_ASSERT_EQUAL( (std::string) (*payload)[1], std::string("hi mom") );
    CPPUNIT_ASSERT_EQUAL( (*payload)[2].type(), BPTNull );
}

void
MessageTest::basicResponseTest()
{
    const static unsigned i = 123456;
    bp::ipc::Response r(i);
    CPPUNIT_ASSERT_EQUAL( i, r.responseTo() );
}

