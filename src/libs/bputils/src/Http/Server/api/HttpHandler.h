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
 * HttpHandler.h  - A base class from which clients can derive to
 *                  implement HTTP handlers.
 */

#ifndef __HTTP_HANDLER_H__
#define __HTTP_HANDLER_H__


#include "BPUtils/HttpRequest.h"
#include "BPUtils/HttpResponse.h"


namespace bp { namespace http { namespace server {
  
/**
 * the IHandler interface may be implemented by classes wishing
 * to handle incoming HTTP requests.  all response data for the
 * request must be provided synchronously in IHandler::processRequest.
 *
 * WARNING: IHandler::processRequest will be invoked on a thread
 *          spawned by the bp::httpserver::Server, so if there's any access
 *          to shared data structures, the client is responsible for
 *          handling sync.
 *
 * IHandlers are mounted on specific paths, or globbing expressions,
 * using the bp::httpserver::Server::mount() method.
 */
class IHandler
{
  public:
    /** process the incoming request, populating the outgoing
     *  response.  Note async streaming is not supported.
     *
     *  \returns if false is returned, the server will send a
     *           Internal server error.  For more robust error handlering
     *           the response code of the response object should be
     *           set appropriately.     
     */
    virtual bool processRequest(const Request & request,
                                Response & response) = 0;

    /**
     * offer the handler an oppty to "shape" the response packet.
     * the handler can specify a new packet size and/or an amount of
     * time to delay transmission of the packet.
     * 
     * \returns true if handler wishes packet to be shaped, false otherwise.
     */
    virtual bool shapePacket(unsigned int packetsSent,
                             unsigned int bytesSent,
                             double elapsedSec,
                             unsigned int& packetBytes,
                             double& packetDelaySec) {
        return false;
    }
            
    /** noop virtual dtor to establish a vtable */
    virtual ~IHandler() { }
  private:
};

} } }


#endif    
