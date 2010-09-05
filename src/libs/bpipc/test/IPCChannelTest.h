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

/**
 * IPCChannelTest.h
 * Unit tests for the IPC channel abstraction
 *
 * Created by Lloyd Hilaiel on 7/30/08 (somewhere over the atlantic)
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __IPCCHANNELTEST_H__
#define __IPCCHANNELTEST_H__

#include "TestingFramework/TestingFramework.h"

class IPCChannelTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(IPCChannelTest);
    CPPUNIT_TEST(stopTest);
    CPPUNIT_TEST(fiveHundredTest);
    CPPUNIT_TEST(peerTerminatedTest);
    CPPUNIT_TEST(tenTimesFourHundredTest);
    CPPUNIT_TEST_SUITE_END();
    
protected:
    // a basic test which exchanges two messages and then verifies that 
    // we can shutdown cleanly
    void stopTest();
    // A test which exchanges 500 query/responses and then quits
    void fiveHundredTest();
    // connect to a server, immediately shut down the server, and verify that
    // our connection is properly terminated
    void peerTerminatedTest();
    // A test which exchanges 500 query/responses over 5 connections and then
    // quits
    void tenTimesFourHundredTest();
};

#endif
