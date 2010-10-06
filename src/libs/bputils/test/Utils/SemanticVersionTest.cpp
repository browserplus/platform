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

#include "SemanticVersionTest.h"
#include "BPUtils/bpsemanticversion.h"

using namespace bp;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(SemanticVersionTest);


void
SemanticVersionTest::matchTest()
{
    SemanticVersion v1;
    CPPUNIT_ASSERT(v1.parse("1.2.3.4"));
    SemanticVersion v2;
    v2.setMajor(1); v2.setMinor(2); v2.setMicro(3); v2.setNano(4);
    CPPUNIT_ASSERT(v1.match(v2));

    v2.setMicro(0);
    CPPUNIT_ASSERT(v1.match(v2) == false);
}


void
SemanticVersionTest::compareTest()
{
    SemanticVersion v1;
    CPPUNIT_ASSERT(v1.parse("1.2.3.4"));
    SemanticVersion v2;
    v2.setMajor(1); v2.setMinor(2); v2.setMicro(3); v2.setNano(4);
    CPPUNIT_ASSERT(v1.compare(v2) == 0);

    v2.setMicro(0);
    CPPUNIT_ASSERT(v1.compare(v2) > 0);

    v2.setMicro(5);
    CPPUNIT_ASSERT(v1.compare(v2) < 0);
}


void
SemanticVersionTest::rangeTest()
{
    SemanticVersion min, max;
    CPPUNIT_ASSERT(min.parse("1.2.3"));
    CPPUNIT_ASSERT(max.parse("1.5"));

    SemanticVersion v;
    CPPUNIT_ASSERT(v.parse("2"));
    CPPUNIT_ASSERT(v.withinRange(min, max) == false);

    max.setMajor(2);
    CPPUNIT_ASSERT(v.withinRange(min, max));
}


void
SemanticVersionTest::wildNanoTest()
{
    // XXX current compare() semantics are broken wrt wildcards,
    // XXX this test uses the bad semantics
    SemanticVersion v1, v2;
    CPPUNIT_ASSERT(v1.parse("1.2.3.4"));
    CPPUNIT_ASSERT(v2.parse("1.2.3"));
    CPPUNIT_ASSERT(v1.match(v2));
    CPPUNIT_ASSERT(v1.compare(v2) > 0); // XXX this should be ==
    CPPUNIT_ASSERT(v2.match(v1));
    CPPUNIT_ASSERT(v2.compare(v1) < 0); // XXX this should be ==

    //    SemanticVersion v3;
    //    CPPUNIT_ASSERT(v3.parse("1.2.3.7"));
    //    CPPUNIT_ASSERT(v3.withinRange(v1, v2));
    //    v3.setNano(2);
    //    CPPUNIT_ASSERT(v3.withinRange(v2, v1));
}


void
SemanticVersionTest::wildMicroTest()
{
    // XXX current compare() semantics are broken wrt wildcards,
    // XXX this test uses the bad semantics
    SemanticVersion v1, v2;
    CPPUNIT_ASSERT(v1.parse("1.2.3"));
    CPPUNIT_ASSERT(v2.parse("1.2"));
    CPPUNIT_ASSERT(v1.match(v2));
    CPPUNIT_ASSERT(v1.compare(v2) > 0); // XXX this should be ==
    CPPUNIT_ASSERT(v2.match(v1));
    CPPUNIT_ASSERT(v2.compare(v1) < 0); // XXX this should be ==

    //    SemanticVersion v3;
    //    CPPUNIT_ASSERT(v3.parse("1.2.7"));
    //    CPPUNIT_ASSERT(v3.withinRange(v1, v2));
    //    v3.setMicro(2);
    //    CPPUNIT_ASSERT(v3.withinRange(v2, v1));
}


void
SemanticVersionTest::wildMinorTest()
{
    // XXX current compare() semantics are broken wrt wildcards,
    // XXX this test uses the bad semantics
    SemanticVersion v1, v2;
    CPPUNIT_ASSERT(v1.parse("1.4"));
    CPPUNIT_ASSERT(v2.parse("1"));
    CPPUNIT_ASSERT(v1.match(v2));
    CPPUNIT_ASSERT(v1.compare(v2) > 0); // XXX this should be ==
    CPPUNIT_ASSERT(v2.match(v1));
    CPPUNIT_ASSERT(v2.compare(v1) < 0); // XXX this should be ==

    //    SemanticVersion v3;
    //    CPPUNIT_ASSERT(v3.parse("1.7"));
    //    CPPUNIT_ASSERT(v3.withinRange(v1, v2));
    //    v3.setMinor(2);
    //    CPPUNIT_ASSERT(v3.withinRange(v2, v1));
}


void
SemanticVersionTest::wildMajorTest()
{
    // XXX current compare() semantics are broken wrt wildcards,
    // XXX this test uses the bad semantics
    SemanticVersion v1, v2;
    CPPUNIT_ASSERT(v1.parse("5"));
    CPPUNIT_ASSERT(v1.match(v2));
    CPPUNIT_ASSERT(v1.compare(v2) > 0); // XXX this should be ==
    CPPUNIT_ASSERT(v2.match(v1));
    CPPUNIT_ASSERT(v2.compare(v1) < 0); // XXX this should be ==

    //    SemanticVersion v3;
    //    CPPUNIT_ASSERT(v3.parse("7"));
    //    CPPUNIT_ASSERT(v3.withinRange(v1, v2));
    //    v3.setMajor(3);
    //    CPPUNIT_ASSERT(v3.withinRange(v2, v1));
}
