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
#include <set>
#include <sstream>
#include "BPUtils/IPCServer.h"


class MyServerListener : public bp::ipc::IServerListener,
                         public bp::ipc::IConnectionListener
{
public:
    MyServerListener(bool verbose) : m_verbose(verbose) { }
    ~MyServerListener() { }

    void gotConnection(bp::ipc::Connection * c)
    {
        printf("%p: new connection established\n", c);
        c->setListener(this);
    }

    void gotMessage(const bp::ipc::Connection * c,
                    const unsigned char * message,
                    unsigned int len)
    {
        if (m_verbose) {
            std::cout << c << ": message: ";
            std::cout.write((const char *) message, len);
            std::cout << std::endl;
        }
        
        // echo
        c->sendMessage(message, len);
    }

    void connectionEnded(bp::ipc::Connection * c,
                         bp::ipc::IConnectionListener::TerminationReason why,
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
        delete c;
    }

    void serverEnded(bp::ipc::IServerListener::TerminationReason why,
                     const char * errorString) const
    {
        std::cout << "Server exited ";
        std::cout << ((why == StopCalled) ? "normally" : "abnormally");
        if (errorString) std::cout << ": " << errorString;
        std::cout << std::endl;
    }
private:
    bool m_verbose;
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

    bool verbose = false;
    const char * path = NULL;
    
    // process command line arguments
    if (argc == 3 && (0 != strcmp("-v", argv[1]))) {
        verbose = true;
        path = argv[2];
    } else if (argc == 2) {
        path = argv[1];
    } else {
        printf("USAGE: %s [-v] <ipc location>\n", argv[0]);
        exit(1);
    }

    // allocate a server
    bp::ipc::Server s;

    // allocate a listener
    MyServerListener list(verbose);

    s.setListener(&list);

    // start up the server
    std::string errBuf;
    if (!s.start(path, &errBuf)) {
        std::cerr << "Couldn't start IPC server";
        if (!errBuf.empty()) std::cerr << ": " << errBuf.c_str();
        std::cerr << std::endl;
        exit(1);
    }
    

    // now just wait until we're killed
    for (;;) sleep((unsigned int) -1);
}
