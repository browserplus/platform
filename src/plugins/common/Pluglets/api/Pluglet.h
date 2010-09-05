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
 * A pure virtual pluglet base class
 */

#ifndef __PLUGLET_H__
#define __PLUGLET_H__

#include "BPUtils/bptypeutil.h"
#include "platform_utils/ServiceDescription.h"
#include "ServiceAPI/bpdefinition.h"


typedef void (*plugletExecutionSuccessCB)(void * cookie,
                                          unsigned int tid,
                                          const bp::Object * returnValue);

typedef void (*plugletInvokeCallbackCB)(void * cookie,
                                        unsigned int tid,
                                        BPCallBack cbHandle,
                                        const bp::Object * returnValue);


typedef void (*plugletExecutionFailureCB)(void * cookie,
                                          unsigned int tid,
                                          const char * error,
                                          const char * verboseError);

/**
 * A abstract base class for a Pluglet.
 */
class Pluglet
{
  public:
    /** allocate a pluglet */
    Pluglet(class BPPlugin * plugin,
            const bp::service::Description& desc);

    /** destroy a pluglet */
    virtual ~Pluglet();

    /** execute a function on a pluglet.  The pluglet must call
     *  either successCB or failureCB exactly once upon completion
     *  of execution. */
    virtual void execute(unsigned int tid,
                         const char * function,
                         const bp::Object * arguments,
                         bool syncInvocation,
                         plugletExecutionSuccessCB successCB,
                         plugletExecutionFailureCB failureCB,
                         plugletInvokeCallbackCB   callbackCB,
                         void * callbackArgument) = 0;

    /**
     * Return the pluglet interface
     */
    virtual const bp::service::Description * describe();

    /** 
     * Get/set locale().  Default is "en-US"
     */
    virtual std::string locale();
    virtual void setLocale(const std::string& locale);

  protected:
    class BPPlugin * m_plugin;
    std::string m_locale;
    bp::service::Description m_desc;
};

#endif
