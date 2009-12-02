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

#include <iostream>
#include <stdexcept>

#ifdef WIN32
#pragma warning ( disable : 4512 )
#define sleep Sleep
#include <Windows.h>
#endif

#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestProgressListener.h>

#include "api/TestHarnessInclude.h"
#include "BPUtils/APTArgParse.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpfile.h"

// definition of program arguments
static APTArgDefinition s_args[] =
{
    {
        "single", APT::TAKES_ARG, "", APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "Specify a single suite to run (see -list for available suites)"
    },
    {
        "list", APT::NO_ARG, "", APT::NOT_REQUIRED, APT::NOT_INTEGER,
        APT::MAY_NOT_RECUR,
        "List available test suites and exit"
    },
    {
        "logToConsole", APT::TAKES_ARG, "", APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "log to console, specify a logging level"
    },
    {
        "logToDebugger", APT::TAKES_ARG, "", APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "log to debugger, specify a logging level"
    },
    {
        "logToFile", APT::TAKES_ARG, "", APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "log to 'TestHarness.log', specify a logging level"
    },
    {
        "wait", APT::NO_ARG, "", APT::NOT_REQUIRED,
        APT::NOT_INTEGER, APT::MAY_NOT_RECUR,
        "sleep for 5 minutes after completing tests.  This allows tools "
        "such as 'leaks' to be used"
    }
};

int main( int argc, const char ** argv )
{
    int rv = 0;
    bool wait = false;
    
    {
        APTArgParse ap(" <options>\n  run BPUtils unit tests");

        int nargs;
        nargs = ap.parse(sizeof(s_args) / sizeof(s_args[0]), s_args, argc, argv);
        if (nargs < 0) {
            std::cerr << ap.error();
            return 1;
        }
        if (nargs != argc) {
            std::cerr << argv[0] << ": " << "unexpected command line args"
                      << std::endl;
            return 1;
        }

        // options and arguments
        CPPUNIT_NS::Test* suite =
            CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

        if (ap.argumentPresent("wait")) wait = true;

        // Handle the -S flag: running of a single test                             
        if (ap.argumentPresent("single")) {
            try
            {
                suite = suite->findTest(ap.argument("single"));
            }
            catch (std::invalid_argument& /*e*/)
            {
                std::cerr << "No such test: " << ap.argument("single") << std::endl;
                return 1;
            }
        }

        if (ap.argumentPresent("list")) {
            int num = suite->getChildTestCount();
            std::cout << "\"" << suite->getName() << "\" has "
                      << num << " child" << ((num > 1) ? "ren" : "") << "):"
                      << std::endl;
        
            for (int x = 0; x < num; x++) {
                CPPUNIT_NS::Test * child;
                child = suite->getChildTestAt(x);
                std::cout << "  " << child->getName() << std::endl;
            }
            return 0;
        }

        if (ap.argumentPresent("logToConsole")) {
            bp::log::setupLogToConsole(ap.argument("logToConsole"));
        }

        if (ap.argumentPresent("logToDebugger")) {
            bp::log::setupLogToDebugger(ap.argument("logToDebugger"));
        }

        if (ap.argumentPresent("logToFile")) {
            bp::file::Path logPath("TestHarness.log");
            bp::log::setupLogToFile(logPath,ap.argument("logToFile"));
        }
    
        //--- Create the event manager and test controller
        CPPUNIT_NS::TestResult controller;

        //--- Add a listener that colllects test result
        CPPUNIT_NS::TestResultCollector result;
        controller.addListener( &result );

        //--- Add a listener that print dots as test run.
        CPPUNIT_NS::TextTestProgressListener progress;
        controller.addListener( &progress );

        CPPUNIT_NS::TextOutputter textOutputter(&result, std::cerr);

        //--- Add the top suite to the test runner
        CPPUNIT_NS::TestRunner runner;
        runner.addTest( suite );
        runner.run( controller );

        textOutputter.write();

        rv = result.wasSuccessful() ? 0 : 1;
    }

    if (wait) sleep(60 * 5);
    
    return rv;
}
