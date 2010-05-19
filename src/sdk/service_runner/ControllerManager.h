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

#ifndef __CONTROLLERMANAGER_H__
#define __CONTROLLERMANAGER_H__

#include "ServiceRunnerLib/ServiceRunnerLib.h"
#include "ConsoleLib/ConsoleLib.h"

class ControllerManager : public ServiceRunner::IControllerListener
{
  public:
    ControllerManager(class CommandExecutor * callback);
    ~ControllerManager();    

    // notify controller manager that an instance has been destroyed;
    void destroy(unsigned int id);
    unsigned int currentInstance();
    std::set<unsigned int> instances();
    bool select(unsigned int id);

    // view outstanding user prompts
    std::set<unsigned int> prompts();
    // respond to a user prompt
    bool responded(unsigned int promptId);
    
  private:
    void initialized(ServiceRunner::Controller *,
                     const std::string &,
                     const std::string &,
                     unsigned int) { }
    void onEnded(ServiceRunner::Controller *);
    void onDescribe(ServiceRunner::Controller * c,
                    const bp::service::Description & desc);
    void onAllocated(ServiceRunner::Controller * c, unsigned int aid,
                     unsigned int id);
    void onInvokeResults(ServiceRunner::Controller * c,
                         unsigned int instance,
                         unsigned int tid,
                         const bp::Object * results);
    void onInvokeError(ServiceRunner::Controller *,
                       unsigned int instance, unsigned int,
                       const std::string & error,
                       const std::string & verboseError);
    void onCallback(ServiceRunner::Controller *,
                    unsigned int instance,
                    unsigned int, long long int, const bp::Object *);
    void onPrompt(ServiceRunner::Controller * c, 
                  unsigned int instance,
                  unsigned int promptId,
                  const bp::file::Path & pathToDialog,
                  const bp::Object * arguments);

    class CommandExecutor * m_callback;

    unsigned int m_currentInstance;
    std::set<unsigned int> m_instances;

    std::set<unsigned int> m_prompts;
};

#endif
