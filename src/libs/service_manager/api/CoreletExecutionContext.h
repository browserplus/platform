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
 * CoreletExecutionContext.h
 * 
 * Interface to allow corelets to interact with their execution context.  
 * Default implementation returns empty strings and always generates
 * UserDeny events. 
 *
 * Created by Gordon Durand on Fri Nov 16 2007.
 * Copyright (c) 2007 Yahoo!, Inc. All rights reserved.
 */

#ifndef __CORELETEXECTIONCONTEXT_H__
#define __CORELETEXECTIONCONTEXT_H__

#include <string>
#include "BPUtils/bptr1.h"
#include "BPUtils/bpfile.h"


namespace bp {
    class Map;
    class Object;
}


class ICoreletExecutionContextListener 
{
  public:
    virtual ~ICoreletExecutionContextListener() { }
    
    virtual void onUserResponse(unsigned int cookie,
                                const bp::Object & resp) = 0;
};

class CoreletExecutionContext
{
public:
    CoreletExecutionContext();
    virtual ~CoreletExecutionContext();
    
    /** attain the locale of the client session */
    virtual std::string locale();
    
    /** attain the URI of the client session */
    virtual std::string URI();
    
    /** attain the user agent of the client session */
    virtual std::string userAgent();
    
    /** attain the process ID of the client session */
    virtual long clientPid();
    
    /**
     * Interact with the user
     *
     * The results of the user interaction will be a passed in the
     * invocation of listener's
     * ICoreletExecutionContextListener::onUserResponse()
     * callback.
     *
     *  \param cookie - client state.  The client may pass in an integer
     *                  which will be set as the eventParameter of the
     *                  event which is raised which contains the results
     *                  of the user interaction.
     *  \param pathToHTMLDialog - a file system path to the HTML containing
     *                            the dialog to display
     *  \param jsonArgumentToDialog - The input argument to pass to the
     *                                HTML dialog, it's structure and
     *                                contents are defined by the dialog.
     */
    virtual void promptUser(
        std::tr1::weak_ptr<ICoreletExecutionContextListener> listener,
        unsigned int cookie,
        const bp::file::Path& pathToHTMLDialog,
        const bp::Object * arguments);  

    virtual void invokeCallback(unsigned int tid, const bp::Map* cbInfo);
};

#endif
