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

#ifndef __JSFUNCTIONWRAPPER_H__
#define __JSFUNCTIONWRAPPER_H__

#include <string>
#include <vector>
#include <map>

namespace bp
{
    class Object;
}

class JSFunctionWrapper
{
  public:
    JSFunctionWrapper();
    JSFunctionWrapper(void * osSpecificContextPtr,
                      void * osSpecificCallbackPtr);
    JSFunctionWrapper(const JSFunctionWrapper & o);
    JSFunctionWrapper & operator=(const JSFunctionWrapper & o);
    ~JSFunctionWrapper();

    bp::Object * invoke(std::vector<const bp::Object *> args);
    unsigned int id();
  private:    
    // memory management for os specific callback representation
    static void retain(void * callback);
    static void release(void * callback);
    unsigned int m_id;
    void * m_osContext;
    void * m_osCallback;
    static unsigned int s_globalId;
};

#endif
