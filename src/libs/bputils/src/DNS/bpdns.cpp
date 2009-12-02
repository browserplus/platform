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

/*
 *  bpdns.h
 *
 *  Created by Lloyd Hilaiel on 1/7/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>

#include "bpdns.h"

std::vector<std::string>
bp::dns::ipToNames(const std::string & ip)
{
    std::vector<std::string> names;
    struct hostent *he = NULL;
    struct in_addr addr;

#ifdef WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return names;
    }

    addr.s_addr = inet_addr(ip.c_str());
    if (addr.s_addr == INADDR_NONE) {
        return names;
    } else {
        he = gethostbyaddr((char *) &addr, 4, AF_INET);
    }
#else
    if (inet_pton(AF_INET, ip.c_str(), &addr) == 1) {
        he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    } else if (inet_pton(AF_INET6, ip.c_str(), &addr) == 1) {
        he = gethostbyaddr(&addr, sizeof(addr), AF_INET6);
    }

#endif

    if (he != NULL) {
        if (he->h_name) names.push_back(he->h_name);
        
        char ** p = he->h_aliases;
        while (p != NULL && *p != NULL) {
            names.push_back(*p);
            p++;
        }
    } 

    return names;
}
