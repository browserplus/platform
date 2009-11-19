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
 * CoreletInstance
 *
 * An instantiated corelet instance upon which functions may be
 * invoked.
 */

#ifndef __CORELETINSTANCE_H__
#define __CORELETINSTANCE_H__

#include "BPUtils/bpthreadhopper.h"
#include "BPUtils/bpfile.h"
#include "CoreletExecutionContext.h"


class ICoreletInstanceListener
{
  public:
    virtual ~ICoreletInstanceListener() { }

    virtual void executionComplete(unsigned int tid,
                                   const bp::Object & results) = 0;

    virtual void executionFailure(
        unsigned int tid, const std::string & error,
        const std::string & verboseError) = 0;
};

class CoreletInstance :
    virtual public ICoreletExecutionContextListener,
    virtual public bp::thread::HoppingClass,
    public std::tr1::enable_shared_from_this<CoreletInstance>
{
public:
    CoreletInstance(std::tr1::weak_ptr<CoreletExecutionContext> context);
    virtual ~CoreletInstance();    

    void setListener(std::tr1::weak_ptr<ICoreletInstanceListener> listener);

    /**
     * execute a function on this corelet instance
     * \param tid A transaction id that should be included in the event
     *            raised once the execution is complete
     * \param function The name of the function to call
     * \param args key/value arguments
     */
    virtual void execute(unsigned int tid,
                         const std::string & function,
                         const bp::Object & args) = 0;

  protected:
    // TODO: are these *always* references to the the same underlying
    //       object?
    std::tr1::weak_ptr<ICoreletInstanceListener> m_listener;
    std::tr1::weak_ptr<CoreletExecutionContext> m_context;

    // utilities for derived classes to send failure and success results.
    // These utility routines will hop to ensure delivery post function
    // return and will correctly promot weak pointers into strong.
    // these routines may be called from any thread.
    void sendComplete(unsigned int tid,
                      const bp::Object & results);
    void sendFailure(unsigned int tid, const std::string & error,
                     const std::string & verboseError);
    // in the invokeCallback case the results contain the integer callback
    // handle being invoked.
    void invokeCallback(unsigned int tid,
                        const bp::Object & results);
    void sendUserPrompt(
        unsigned int cookie,
        const bp::file::Path & pathToHTMLDialog,
        const bp::Object * arguments);  

    void onHop(void * ctx);

    // derived class may override
    virtual void onUserResponse(unsigned int, const bp::Object&) { }
    
};
    
#endif
