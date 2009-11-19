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

#include "api/IPCConnection.h"

// maximum allowed message is 4mb
// this is for control channels, right? (for now)
const unsigned int
bp::ipc::Connection::MaxMessageLength = 4194304;

std::string
bp::ipc::IConnectionListener::terminationReasonToString(TerminationReason tr)
{
    std::string reason;
    
    if (tr == ProtocolError) reason.append("protocol error encountered");
    else if (tr == DisconnectCalled) reason.append("disconnect() was called");
    else if (tr == PeerClosed) reason.append("peer terminated connection");
    else if (tr == InternalError) reason.append("internal error");
    else reason.append("unknown reasons");

    return reason;
}
