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
 * HTMLRenderTest.cpp
 *
 * Created by Lloyd Hilaiel on 6/20/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "HTMLRenderTest.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpstopwatch.h"
#include "BPUtils/bptimer.h"
#include "HTMLRender/HTMLRender.h"


CPPUNIT_TEST_SUITE_REGISTRATION(HTMLRenderTest);

static
bp::file::Path writeToTempFile(const std::string & content)
{
    bp::file::Path path = bp::file::getTempPath(bp::file::getTempDirectory(), "foobar");
    CPPUNIT_ASSERT( bp::strutil::storeToFile(path, content) );
    return path;
}

class BasicScriptableFuncHost : public bp::html::ScriptableFunctionHost
{
public:
    BasicScriptableFuncHost(bp::runloop::RunLoop * rl) : m_rl(rl) 
    { }

    bp::Object * invoke(const std::string & functionName,
                        unsigned int,
                        std::vector<const bp::Object *> args)
    {
        if (!functionName.compare("stop")) {
            m_rl->stop();            
        } else {
            BP_THROW_FATAL("no such function");
        }
        return NULL;
    }
private:
    bp::runloop::RunLoop * m_rl;    
};

void HTMLRenderTest::basicScriptingTest()
{
    bp::file::Path path = writeToTempFile(
        "<html><body>"
        "hello world" 
        "<script>"
        " var stopFunc = function() { BPScriptObj.stop(); };"
        " if (window.addEventListener) {"
        "   window.addEventListener(\"load\", stopFunc, true);"
        " } else if (window.attachEvent) {"
        "   window.attachEvent(\"onload\", stopFunc);"
        " }"
        "</script>"
        "</body></html>");

    bp::runloop::RunLoop rl;
    bp::html::ScriptableObject scriptableObject;
    BasicScriptableFuncHost funcHost(&rl);

    rl.init();

    // put a stop() function into javascript's reach
    scriptableObject.mountFunction(&funcHost, "stop");
    
    {
        JavascriptRunner runner;
        runner.run(path, scriptableObject, "BPScriptObj");

        rl.run();
    }
    
    rl.shutdown();

    CPPUNIT_ASSERT(bp::file::remove(path));
}


class ExceptionScriptableFuncHost : public bp::html::ScriptableFunctionHost
{
public:
    ExceptionScriptableFuncHost(bp::runloop::RunLoop * rl)
        : gotException(false), m_rl(rl) { }

    bp::Object * invoke(const std::string & functionName,
                        unsigned int,
                        std::vector<const bp::Object *> args)
    {
        if (!functionName.compare("stop")) {
            m_rl->stop();            
        } else if (!functionName.compare("gotException")) {
            gotException = true;
        } else {
            BP_THROW_FATAL("no such function");
        }
        return NULL;
    }
    bool gotException;

private:
    bp::runloop::RunLoop * m_rl;    
};

void HTMLRenderTest::exceptionForUndefinedMethod()
{
    bp::file::Path path = writeToTempFile(
        "<html><body>"
        "hello world" 
        "<script>"
        "var testFunc = function() {"
        "  try {"
        "    BPDialog.undefinedFunction();"
        "  } catch (e) {"
        "    BPDialog.gotException();"
        "  }"
        "  BPDialog.stop();"
        "};"
        "if (window.addEventListener) {"
        "  window.addEventListener(\"load\", testFunc, true);"
        "} else if (window.attachEvent) {"
        "  window.attachEvent(\"onload\", testFunc);"
        "}"
        "</script>"
        "</body></html>");

    bp::runloop::RunLoop rl;
    bp::html::ScriptableObject scriptableObject;
    ExceptionScriptableFuncHost funcHost(&rl);

    rl.init();

    // put a stop() function into javascript's reach
    scriptableObject.mountFunction(&funcHost, "stop");
    scriptableObject.mountFunction(&funcHost, "gotException");
    
    {
        JavascriptRunner runner;
        runner.run(path, scriptableObject, "BPDialog");

        rl.run();
    }
    
    rl.shutdown();

    CPPUNIT_ASSERT( funcHost.gotException );
    CPPUNIT_ASSERT(bp::file::remove(path));
}

class ArgsTestScriptableFuncHost : public bp::html::ScriptableFunctionHost
{
public:
    ArgsTestScriptableFuncHost(bp::runloop::RunLoop * rl)
        : m_rl(rl) { }

    bp::Object * invoke(const std::string & functionName,
                        unsigned int,
                        std::vector<const bp::Object *> args)
    {
        if (!functionName.compare("checkArgs")) {
            // validate arguments
            CPPUNIT_ASSERT( args.size() == 6 ); 
            CPPUNIT_ASSERT( args[0]->type() == BPTBoolean ); 
            CPPUNIT_ASSERT( (bool)(*args[0]) );
            CPPUNIT_ASSERT( args[1]->type() == BPTInteger ); 
            CPPUNIT_ASSERT( (long long)(*args[1]) == 0 );
            CPPUNIT_ASSERT( args[2]->type() == BPTNull ); 
            CPPUNIT_ASSERT( args[3]->type() == BPTDouble ); 
            CPPUNIT_ASSERT( ((bp::Double *) args[3])->value() == 7.77 );
            CPPUNIT_ASSERT( args[4]->type() == BPTMap ); 
            CPPUNIT_ASSERT( ((bp::Map *) args[4])->size() == 1 );
            CPPUNIT_ASSERT( args[4]->has("foo", BPTString) );
            CPPUNIT_ASSERT( args[5]->type() == BPTList ); 
            CPPUNIT_ASSERT( ((bp::List *) args[5])->size() == 3 );
        } else if (!functionName.compare("stop")) {
            m_rl->stop();            
        } else {
            BP_THROW_FATAL("no such function");
        }
        return NULL;
    }

private:
    bp::runloop::RunLoop * m_rl;    
};

void HTMLRenderTest::argsJSToNative()
{
    bp::file::Path path = writeToTempFile(
        "<html><body>"
        "hello world" 
        "<script>"
        "var testFunc = function() {"
           // TODO: investigate passing null through checkargs
           "BPDialog.checkArgs(true, 0, undefined, 7.77, {foo:'bar'}, [1,2,3]);"
           "BPDialog.stop();"
        "};"
        "if (window.addEventListener) {"
        "  window.addEventListener(\"load\", testFunc, true);"
        "} else if (window.attachEvent) {"
        "  window.attachEvent(\"onload\", testFunc);"
        "}"
        "</script>"
        "</body></html>");

    bp::runloop::RunLoop rl;
    bp::html::ScriptableObject scriptableObject;
    ArgsTestScriptableFuncHost funcHost(&rl);

    rl.init();

    // put a stop() function into javascript's reach
    scriptableObject.mountFunction(&funcHost, "stop");
    scriptableObject.mountFunction(&funcHost, "checkArgs");
    
    {
        JavascriptRunner runner;
        runner.run(path, scriptableObject, "BPDialog");

        rl.run();
    }

    rl.shutdown();

    CPPUNIT_ASSERT( bp::file::remove(path) );
}

class ArgsTwoScriptableFuncHost : public bp::html::ScriptableFunctionHost
{
public:
    ArgsTwoScriptableFuncHost(bp::runloop::RunLoop * rl)
        : happy(true), m_rl(rl) { }

    bp::Object * invoke(const std::string & functionName,
                        unsigned int,
                        std::vector<const bp::Object *> args)
    {
        if (!functionName.compare("notHappy")) {
            happy = false;
        } else if (!functionName.compare("stop")) {
            m_rl->stop();            
        } else if (!functionName.compare("getData")) {
            return bp::Object::fromPlainJsonString(
                "[true, 0, null, 7.77, {\"foo\":\"bar\"}, [1,2,3]]");
        } else {
            BP_THROW_FATAL("no such function");
        }
        return NULL;
    }
    bool happy;

private:
    bp::runloop::RunLoop * m_rl;    
};

void HTMLRenderTest::argsNativeToJS()
{
    bp::file::Path path = writeToTempFile(
        "<html><body>"
        "hello world" 
        "<script>"
        "var testFunc = function() {"
        "  var x = BPDialog.getData();"
           // now validate we got what we expected
        "  try {"
        "    if (x.length != 6) BPDialog.notHappy();"
        "    if (x[0] !== true) BPDialog.notHappy();"
        "    if (x[1] !== 0) BPDialog.notHappy();"
        "    if (x[2] !== null) BPDialog.notHappy();"
        "    if (x[3] !== 7.77) BPDialog.notHappy();"
        "    if (x[4].foo !== 'bar') BPDialog.notHappy();"
        "    if (x[5].length !== 3) BPDialog.notHappy();"
        "    if (x[5][0] !== 1) BPDialog.notHappy();"
        "    if (x[5][1] !== 2) BPDialog.notHappy();"
        "    if (x[5][2] !== 3) BPDialog.notHappy();"
        "  } catch(e) { BPDialog.notHappy(); }"
        "  BPDialog.stop();"
        "};"
        "if (window.addEventListener) {"
        "  window.addEventListener(\"load\", testFunc, true);"
        "} else if (window.attachEvent) {"
        "  window.attachEvent(\"onload\", testFunc);"
        "}"
        "</script>"
        "</body></html>");

    bp::runloop::RunLoop rl;
    bp::html::ScriptableObject scriptableObject;
    ArgsTwoScriptableFuncHost funcHost(&rl);

    rl.init();

    scriptableObject.mountFunction(&funcHost, "getData");
    scriptableObject.mountFunction(&funcHost, "notHappy");
    scriptableObject.mountFunction(&funcHost, "stop");

    {
        JavascriptRunner runner;
        runner.run(path, scriptableObject, "BPDialog");

        rl.run();
    }
    
    rl.shutdown();

    CPPUNIT_ASSERT( funcHost.happy );
    CPPUNIT_ASSERT(bp::file::remove(path));
}


// a class to stop a runloop when a timer fires
class RunLoopStopper : public bp::time::ITimerListener
{
public:
    RunLoopStopper(bp::runloop::RunLoop * rl) : m_rl(rl) { }
    void timesUp(bp::time::Timer *) { m_rl->stop(); }
private:
    bp::runloop::RunLoop * m_rl;
};

class CallbackScriptableFuncHost : public bp::html::ScriptableFunctionHost,
                                   public bp::time::ITimerListener
{
public:
    CallbackScriptableFuncHost(bp::runloop::RunLoop * rl)
        : m_rl(rl), m_cb(0)
    {
        m_t.setListener(this);
    }

    bp::Object * invoke(const std::string & functionName,
                        unsigned int id,
                        std::vector<const bp::Object *> args)
    {
        if (!functionName.compare("stop")) {
            m_rl->stop();            
        } else if (!functionName.compare("setTimeout")) {
            // extract arguments
            CPPUNIT_ASSERT( args.size() == 2 ); 
            unsigned int msec = (unsigned int) ((long long) *args[1]);
            CPPUNIT_ASSERT_EQUAL( args[0]->type(), BPTCallBack ); 
            m_tid = id;
            m_cb = bp::CallBack(*(args[0]));
            retain(id);
            m_t.setMsec(msec);
        } else {
            BP_THROW_FATAL("no such function");
        }
        return NULL;
    }

private:
    void timesUp(bp::time::Timer *) {
        bp::Object * o = invokeCallback(m_tid, m_cb,
                                        std::vector<const bp::Object *>());
        
        CPPUNIT_ASSERT(o != NULL);
        CPPUNIT_ASSERT(o->type() == BPTNull);
        delete o;
        release(m_tid);
    }

    bp::runloop::RunLoop * m_rl;    

    // in the real world this would be a map.  we know there is only
    // one transaction happening here.
    bp::time::Timer m_t;
    unsigned int m_tid;
    bp::CallBack m_cb;
};

void
HTMLRenderTest::basicCallbackTest()
{
    // Theory of this test:
    // We implement a setTimeout function which will call a callback
    // after a period elapses.  We use setTimeout to stop the runloop
    // after 50ms.  We set another timer to stop the runloop in 120ms.
    // if the test takes longer than 100ms we assume that the setTimer
    // function didn't work, implying that callbacks are broken.

    bp::runloop::RunLoop rl;
    rl.init();

    bp::file::Path path = writeToTempFile(
        "<html><body>"
        "hello world" 
        "<script>"
        "var testFunc = function() {"
        "  BPDialog.setTimeout(function() { BPDialog.stop() }, 50);"
        "};"
        "if (window.addEventListener) {"
        "  window.addEventListener(\"load\", testFunc, true);"
        "} else if (window.attachEvent) {"
        "  window.attachEvent(\"onload\", testFunc);"
        "}"
        "</script>"
        "</body></html>");

    bp::html::ScriptableObject scriptableObject;
    CallbackScriptableFuncHost funcHost(&rl);

    // a function to stop the runloop via js callback
    scriptableObject.mountFunction(&funcHost, "setTimeout");
    scriptableObject.mountFunction(&funcHost, "stop");

    // a timer to stop the runloop in case the callback is not fired.
    bp::time::Timer t;
    RunLoopStopper rls(&rl);
    t.setListener(&rls);
    t.setMsec(250);

    {
        JavascriptRunner runner;
        runner.run(path, scriptableObject, "BPDialog");

        // now verify that we shutdown in less than 100ms with a stopwatch
        bp::time::Stopwatch sw;
        sw.start();
        rl.run();
        CPPUNIT_ASSERT( sw.elapsedSec() < .2 );
    }
    
    rl.shutdown();

    CPPUNIT_ASSERT(bp::file::remove(path));
}

class CallbackTypeScriptableFuncHost : public bp::html::ScriptableFunctionHost
{
public:
    CallbackTypeScriptableFuncHost(bp::runloop::RunLoop * rl)
        : happy(true), m_rl(rl) { }

    bp::Object * invoke(const std::string & functionName,
                        unsigned int id,
                        std::vector<const bp::Object *> args)
    {
        if (!functionName.compare("passTypes")) {
            bp::Object * rv = NULL;
            if (args.size() == 1 && args[0]->type() == BPTCallBack) {
                bp::Object * o = bp::Object::fromPlainJsonString(
                    "[true, 0, null, 7.77, {\"foo\":\"bar\"}, [1,2,3]]");
                std::vector<const bp::Object *> input;
                input.push_back(o);
                rv = invokeCallback(id, *((bp::CallBack *) args[0]), input);
                delete o;
            }
            CPPUNIT_ASSERT( rv != NULL && rv->type() == BPTList );
            // [true, 0, null, 7.77, {\"foo\":\"bar\"}, [1,2,3]]
            bp::List * l = (bp::List *) rv;
            CPPUNIT_ASSERT( (bool) *(l->value(0)) );
            CPPUNIT_ASSERT( 0 == (long long) *(l->value(1)) );            
            CPPUNIT_ASSERT( l->value(2)->type() == BPTNull );            
            CPPUNIT_ASSERT( 7.77 == (double) *(l->value(3)) );            
            CPPUNIT_ASSERT( l->value(4)->type() == BPTMap );            
            CPPUNIT_ASSERT( l->value(4)->has("foo", BPTString) );
            CPPUNIT_ASSERT( l->value(5)->type() == BPTList );            
            if (rv) delete rv;
        } else if (!functionName.compare("notHappy")) {
            happy = false;
        } else if (!functionName.compare("stop")) {
            m_rl->stop();            
        } else if (!functionName.compare("log")) {
            if (args.size() == 1 && args[0]->type() == BPTString) {
                std::cerr << "log: " << std::string(*args[0]) << std::endl;
            }
        } else {
            BP_THROW_FATAL("no such function");
        }
        return NULL;
    }
    bool happy;

private:
    bp::runloop::RunLoop * m_rl;    
};

void
HTMLRenderTest::callbackTypeMappingTest()
{
    bp::runloop::RunLoop rl;
    rl.init();

    bp::file::Path path = writeToTempFile(
        "<html><body>"
        "hello world" 
        "<script>"
        "var testFunc = function() {"
        "  notCalled = true;"
        "  BPDialog.passTypes(function(x) {"
        "    notCalled = false;"
        "    try {"
        "      if (x.length != 6) BPDialog.notHappy();"
        "      if (x[0] !== true) BPDialog.notHappy();"
        "      if (x[1] !== 0) BPDialog.notHappy();"
        "      if (x[2] !== null) BPDialog.notHappy();"
        "      if (x[3] !== 7.77) BPDialog.notHappy();"
        "      if (x[4].foo !== 'bar') BPDialog.notHappy();"
        "      if (x[5].length !== 3) BPDialog.notHappy();"
        "      if (x[5][0] !== 1) BPDialog.notHappy();"
        "      if (x[5][1] !== 2) BPDialog.notHappy();"
        "      if (x[5][2] !== 3) BPDialog.notHappy();"
        "    } catch(e) { BPDialog.notHappy(); }"
//        "  BPDialog.log('returning from nested function');"
        "    return [true, 0, null, 7.77, {\"foo\":\"bar\"}, [1,2,3]];"
        "  });"
        "  if (notCalled) BPDialog.notHappy();"  
//        "BPDialog.log('calling stop');"
        "  BPDialog.stop();"
       "};"
       "if (window.addEventListener) {"
       "  window.addEventListener(\"load\", testFunc, true);"
       "} else if (window.attachEvent) {"
       "  window.attachEvent(\"onload\", testFunc);"
       "}"
        "</script>"
        "</body></html>");

    bp::html::ScriptableObject scriptableObject;
    CallbackTypeScriptableFuncHost funcHost(&rl);

    // a function to stop the runloop after 100ms elapse
    scriptableObject.mountFunction(&funcHost, "stop");
    scriptableObject.mountFunction(&funcHost, "passTypes");
    scriptableObject.mountFunction(&funcHost, "notHappy");
    scriptableObject.mountFunction(&funcHost, "log");

    {
        JavascriptRunner runner;
        runner.run(path, scriptableObject, "BPDialog");
        rl.run();
    }
    
    rl.shutdown();

    CPPUNIT_ASSERT( funcHost.happy );
    CPPUNIT_ASSERT(bp::file::remove(path));
}
