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

#include "stresstest.h"
#include <iostream>
#include "BPProtocol/BPProtocol.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bptime.h"
#include "BPUtils/bptimer.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/bperrorutil.h"


bp::runloop::RunLoop s_rl;

static unsigned int s_started = 0;
static unsigned int s_complete = 0;
static unsigned int s_failures = 0;    

static void startConnection(BPProtoHand * pph);

static void requireCallback(BPErrorCode ec,
                            void * cookie,
                            const BPCoreletDefinition ** ,
                            unsigned int ,
                            const char *, const char *)
{
    BPProtoHand * pph = (BPProtoHand *) cookie;

    // done
    if (ec != BP_EC_OK) s_failures++;
    else s_complete++;
    
    BPFree(*pph);
    *pph = NULL;
    startConnection(pph);                    
}

static void connectCallback(BPErrorCode ec, void * cookie,
                            const char *, const char *)
{
    BPProtoHand * pph = (BPProtoHand *) cookie;

    if (ec == BP_EC_OK) {
        // he's connected, let's kick off a require
        bp::Object * args = bp::Object::fromPlainJsonString(
            "{\"services\":[{\"service\":\"InactiveServices\"}]}");

        // now require the InactiveServices service on this connection
        BPErrorCode ec2;
        ec2 = BPRequire(*pph, args->elemPtr(), requireCallback,
                        (void *) pph, NULL, NULL, NULL);
        delete args;

        if (ec2 != BP_EC_OK) s_failures++;
    } else {
        s_failures++;
    }
}

static void
promptCB(void * handptr, const char * path, const BPElement *, unsigned int tid)
{
    BPProtoHand hand = (BPProtoHand) handptr;
    std::cout << "Received request to prompt user: " << path << std::endl;
    bp::String s("AlwaysAllow");
    BPDeliverUserResponse(hand, tid, s.elemPtr());
}

static void startConnection(BPProtoHand * pph)
{
    BPErrorCode ec;

    *pph = BPAlloc();
    BPSetUserPromptCallback(*pph, promptCB, *pph);
    BPASSERT(*pph != NULL);

    s_started++;
    ec = BPConnect(*pph, "bpclient://9F802D4B-1F23-42A4-9490-8FC8EE2BCCDD",
                   "en", "BrowserPlus stress tester", 
                   connectCallback, (void *) pph);

    BPLOG_INFO_STRM("BPConnect returns with: " <<  BPErrorCodeToString(ec));

    if (ec != BP_EC_OK) {
        // for a failure at this point we will not retry
        s_failures++;
    }
}

class StresserStopper : public bp::time::ITimerListener 
{
public:
    void timesUp(bp::time::Timer *) { s_rl.stop(); }
};

void
runTest(unsigned int simulConns, long duration)
{
    BPTime startTime;

    s_rl.init();

    unsigned int i;
    
    std::cout <<  "running with " << simulConns << " simultaneous "
              << "connections for " << duration << "s." << std::endl;

    // setup a timer so we can stop the dance as configured
    StresserStopper ss;
    bp::time::Timer timer;
    timer.setListener(&ss);
    timer.setMsec(duration * 1000);
    
    // first allocate an array of connection handles
    BPProtoHand * s_phs = (BPProtoHand*)
        calloc(simulConns, sizeof(BPProtoHand));
  
    // get all of the stress contexts connecting
    for (i=0; i<simulConns; i++) startConnection(s_phs + i);

    s_rl.run();
    
    // now clean up and report
    for (i=0; i<simulConns; i++) BPFree(s_phs[i]);

    free(s_phs);
    s_phs = NULL;

    std::cout << "Test complete, " << s_started
              << " connections started, "
              << s_failures << " failures, "
              << s_complete << " completed. "
              << "Spent " << BPTime().diffInSeconds(startTime)
              << "s." << std::endl;
}
