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


#include <iostream>
#include <sstream>
#include "BPUtils/IPCConnection.h"

class MyIPCClient : public bp::ipc::IConnectionListener
{
public:
    MyIPCClient() { }
        
    void gotMessage(const bp::ipc::Connection *,
                    const unsigned char * message,
                    unsigned int len)
    {
        std::cout.write((const char *) message, len);
        std::cout << std::endl;
    }

    void connectionEnded(bp::ipc::Connection *,
                         TerminationReason why,
                         const char * errorString)
    {
        std::stringstream ss;
        ss << "Connection ended. ";
        if (why == DisconnectCalled) ss << "We terminated";
        else if (why == PeerClosed) ss << "Peer terminated";
        else ss << " ERROR";
        if (errorString) ss << ": " << errorString;
        else ss << ".";
        std::cerr << ss.str() << std::endl;
    }
    ~MyIPCClient() { }
};

#ifdef WIN32
#define sleep Sleep
#endif

int
main(int argc, char ** argv)
{
#ifndef WIN32
    (void) signal(SIGPIPE, SIG_IGN);
#endif

    // process command line arguments
    if (argc != 2) {
        printf("USAGE: %s <ipc location>\n", argv[0]);
        exit(1);
    }

    unsigned long long msgs = 0;

    // our listener, must have a lifetime exceeding the connection, or we
    // must explicitly call disconnect at the end
    MyIPCClient client;
    {
        // let's instantiate our ipc connection
        bp::ipc::Connection conn;

        conn.setListener(&client);
        std::string error;
        if (!conn.connect(argv[1], &error)) {
            std::cout << "couldn't connect";
            if (!error.empty()) std::cout << ": " << error.c_str();
            std::cout << std::endl;
            exit(1);
        }
    
        // now let's process keyboard input, and shovel it to the server
        // until we hit EOF
        char buf[256];
        while(NULL != fgets(buf, sizeof(buf), stdin)) {
            if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\n') {
                buf[strlen(buf) - 1] = 0;
            }
            if (strlen(buf) > 0) {
                if (!conn.sendMessage(buf)) {
                    std::cout << "failed to send message" << std::endl;
                } else {
                    msgs++;
                }
            }
        }
    }

    std::cerr << "exiting, " << msgs << " messages successfully sent"
              << std::endl;
    
    return 0;
}
