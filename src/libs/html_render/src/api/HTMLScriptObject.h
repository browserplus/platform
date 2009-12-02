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
 * HTMLScriptObject.h -
 * 
 */

#ifndef _HTMLSCRIPTOBJECT_H_
#define _HTMLSCRIPTOBJECT_H_

#include <string>
#include <vector>
#include <map>

#include "BPUtils/bptypeutil.h"


namespace bp { namespace html {
  class ScriptableFunctionHost {
    public:
      // dispatch function to be implemented by derived class
      // NOTE: may throw a std::string which will be propogated to
      //       javascript
      virtual bp::Object * invoke(const std::string & functionName,
                                  unsigned int id,
                                  std::vector<const bp::Object *> args) = 0;
      virtual ~ScriptableFunctionHost() { };

    protected:
      // for the invocation of callbacks
      bp::Object * invokeCallback(unsigned int id,
                                  bp::CallBack cb,
                                  std::vector<const bp::Object *> args);

      // TODO: implement the ability to throw exceptions into JavaScript
      // void raiseException(std::string exception);

      void retain(unsigned int id);
      void release(unsigned int id);      

    private:
      static unsigned int s_uniqueId;
      // a map of open transactions.  This map allows callback invocation
      // and lifetime management of callback arguments from javascript to
      // native code
      std::map<unsigned int, class BPHTMLTransactionContext *>
          m_transactions;
      // add a transaction to the map with a unique ID.  This is
      // used by the ScriptableObject implementation
      unsigned int addTransaction(class BPHTMLTransactionContext * t);

      // hack to keep implementation private
      friend class ScriptFunctionHostProxyClass;
  };

  class ScriptableObject {
  public:
    ScriptableObject();
    ~ScriptableObject();

    // add a function to the top level object that may be invoked from
    // javascript.  All functions should be added by the time render()
    // is called
    bool mountFunction(ScriptableFunctionHost * host,
                       const std::string & functionName);
      
    // Get an OS specific scriptable object:
    //   on OSX this is a id to a OBJC class that may be
    //   attached to the top level js context
    //   on OSX the underlying object is created by this method.
    //   on Win32 this method not implemented.
    void * scriptableObject(void * osSpecificArg);      

    // Return the host for the specified method name, or null if no match found.
    ScriptableFunctionHost *
    findFunctionHost(const std::string& functionName) const;
    
    // Returns a reference to our function map.
    const std::map<std::string, ScriptableFunctionHost *>& functionMap() const;
    
  private:
    std::map<std::string, ScriptableFunctionHost *> m_functions;

    void * m_osSpecific;
  };
}};

#endif
