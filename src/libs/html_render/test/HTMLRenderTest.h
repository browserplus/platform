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
 * HTMLRenderTest.h
 * test of html rendering & javascript scripting
 *
 * Created by Lloyd Hilaiel on 6/20/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __HTMLRENDERTEST_H__
#define __HTMLRENDERTEST_H__

#include "TestingFramework/TestingFramework.h"
#include "HTMLRender/HTMLRender.h"
#include "BPUtils/bpfile.h"

class HTMLRenderTest : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(HTMLRenderTest);
    // test basic ability to call from javascript to c++
    CPPUNIT_TEST(basicScriptingTest);
    // verify that exceptions are sent up to javascript when an undefined
    // method is called
    CPPUNIT_TEST(exceptionForUndefinedMethod);
    // verify that objects are correctly mapped from javascript into c++
    CPPUNIT_TEST(argsJSToNative);
    // verify that objects are correctly mapped from c++ into javascript
    CPPUNIT_TEST(argsNativeToJS);
    // verify that the basic callback invocation mechanisms are working
    CPPUNIT_TEST(basicCallbackTest);
    // verify that types are correctly mapped to and from callbacks
    CPPUNIT_TEST(callbackTypeMappingTest);
    CPPUNIT_TEST_SUITE_END();
    
protected:
    void basicScriptingTest();
    void exceptionForUndefinedMethod();
    void argsJSToNative();
    void argsNativeToJS();
    void basicCallbackTest();
    void callbackTypeMappingTest();

    // platform specific object to "run" a webpage given a scriptable object.  Top level object should be mapped
    // Scriptable object must be attached to window object at specified
    // name.
    class JavascriptRunner {
      public:
        JavascriptRunner();
        ~JavascriptRunner();        

        void run(const bp::file::Path & path,
                 bp::html::ScriptableObject & so,
                 const std::string & soName);

      private:
        void * m_osSpecific;  
    };
};

#endif
