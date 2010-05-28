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
 * BPObjectTest.h
 * Unit tests for the in memory object representation (stored in
 * format appropriate for transmission over boundaries that use C types
 * defined in ServiceAPI)
 *
 * Created by Lloyd Hilaiel on 7/24/08.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "BPObjectTest.h"
#include "BPUtils/bptypeutil.h"
#include "BPUtils/bpfile.h"


CPPUNIT_TEST_SUITE_REGISTRATION(BPObjectTest);

void BPObjectTest::boolTest()
{
    bp::Bool b(true);

    CPPUNIT_ASSERT( ((bool)b) );
    CPPUNIT_ASSERT( b.value() );    
    
    bp::Object * bp = bp::Object::fromJsonString(b.toJsonString());

    CPPUNIT_ASSERT( true == (bool) *bp );
    CPPUNIT_ASSERT( dynamic_cast<bp::Bool *>(bp) );
    CPPUNIT_ASSERT( dynamic_cast<bp::Bool *>(bp)->value() );    

    delete bp;

    bp = b.clone();

    CPPUNIT_ASSERT( true == (bool) *bp );
    CPPUNIT_ASSERT( dynamic_cast<bp::Bool *>(bp) );
    CPPUNIT_ASSERT( dynamic_cast<bp::Bool *>(bp)->value() );    
    delete bp;

    CPPUNIT_ASSERT( !b.toJsonString().compare(
                        "{\"t\":\"boolean\",\"v\":true}") );
    CPPUNIT_ASSERT( !b.toPlainJsonString().compare("true") );

    // copy test
    bp::Bool copy(b);
    CPPUNIT_ASSERT( (bool) b == (bool) copy );

    // assignment test
    bp::Bool copy2(false);    
    copy2 = b;
    CPPUNIT_ASSERT( (bool) b == (bool) copy2 );    
}

void BPObjectTest::nullTest()
{
    bp::Object * bp = bp::Object::fromPlainJsonString("null");
    CPPUNIT_ASSERT( bp->type() == BPTNull );
    CPPUNIT_ASSERT( !bp->toJsonString().compare(
                        "{\"t\":\"null\",\"v\":null}") );
    CPPUNIT_ASSERT( !bp->toPlainJsonString().compare("null") );

    bp::Object * clone = bp->clone();
    CPPUNIT_ASSERT( clone->type() == BPTNull );
    delete clone;

    bp::Null copy(*((bp::Null *) bp));
    CPPUNIT_ASSERT( copy.type() == BPTNull );    
        
    delete bp;
}

void
BPObjectTest::doubleTest()
{
    bp::Object * bp = bp::Object::fromPlainJsonString("7.898989");
    CPPUNIT_ASSERT( bp->type() == BPTDouble );
    CPPUNIT_ASSERT( ((double) *bp) == 7.898989 );

    bp::Object * clone = bp->clone();
    CPPUNIT_ASSERT( (double) *clone == (double) *bp );
    delete clone;

    bp::Double copy(*((bp::Double *)bp));
    CPPUNIT_ASSERT( copy.value() == (double) *bp );

    bp::Double ass(22.7);    
    ass = *((bp::Double *) bp);
    CPPUNIT_ASSERT( ass.value() == (double) *bp );    

    delete bp;
}

void
BPObjectTest::stringTest()
{
    bp::Object * bp = bp::Object::fromPlainJsonString("\"foobarbaz\"");
    CPPUNIT_ASSERT( !std::string("foobarbaz").compare(*bp) );
    CPPUNIT_ASSERT( bp->type() == BPTString );
    delete bp;

    bp::String s("bing bang boom");
    CPPUNIT_ASSERT( !std::string("bing bang boom").compare(s) );    

    bp::String copy(s);
    CPPUNIT_ASSERT( !std::string(copy).compare(s) );        

    bp::Object * clone = s.clone();
    CPPUNIT_ASSERT( !std::string(*clone).compare(s) );            
    delete clone;

    bp::String ass("ass");
    ass = s;
    CPPUNIT_ASSERT( !std::string(ass).compare(s) );            
}

void
BPObjectTest::pathTest()
{
#ifdef win32
	std::string somePath("D:\some\path.exe");
	std::wstring somePathNative(somePath);
#else
	std::string somePath("/some/path");
	std::string somePathNative = somePath;
#endif
	std::string json = "{\"t\":\"path\",\"v\":\"";
	json += somePath;
	json += "\"}";

    bp::Object * bp = bp::Object::fromJsonString(json);

    bp::file::Path orig = *((bp::Path *) bp);
	CPPUNIT_ASSERT( bp::file::Path(somePathNative) == orig );
    CPPUNIT_ASSERT( bp->type() == BPTNativePath);

    bp::Object * clone = bp->clone();
	bp::file::Path rhs(somePathNative);
	bp::file::Path lhs = *((bp::Path *)clone);
    CPPUNIT_ASSERT( clone->type() == BPTNativePath );    
	CPPUNIT_ASSERT( rhs.string() == lhs.string() );
    delete clone;

    bp::file::Path p(somePathNative);
    bp::Path asp(p);
    bp::file::Path after = asp;
	CPPUNIT_ASSERT( p == after );

    delete bp;
}

void
BPObjectTest::integerTest()
{
    // positive
    bp::Object * bp = bp::Object::fromPlainJsonString("1777889");
    CPPUNIT_ASSERT( 1777889 == (long long) *bp );
    CPPUNIT_ASSERT( bp->type() == BPTInteger);
    delete bp;

    // negative
    bp = bp::Object::fromPlainJsonString("-21777889");
    CPPUNIT_ASSERT( -21777889 == (long long) *bp );
    CPPUNIT_ASSERT( bp->type() == BPTInteger);
    delete bp;

    // zero
    bp = bp::Object::fromPlainJsonString("0");
    CPPUNIT_ASSERT( 0 == (long long) *bp );
    CPPUNIT_ASSERT( bp->type() == BPTInteger);
    delete bp;

    // clone & copy
    bp::Integer i(-7890123);
    bp::Object * clone = i.clone();
    CPPUNIT_ASSERT_EQUAL( i.value(), (long long) *clone );    
    delete clone;

    bp::Integer copy(i);
    CPPUNIT_ASSERT_EQUAL( i.value(), copy.value() );        
    
    bp::Integer ass(0);
    ass = i;
    CPPUNIT_ASSERT_EQUAL( i.value(), ass.value() );
}

void
BPObjectTest::callbackTest()
{
    bp::Object * bp = bp::Object::fromJsonString(
        "{\"t\":\"callback\",\"v\":12345678}");
    CPPUNIT_ASSERT( 12345678 == (BPCallBack) *bp );
    CPPUNIT_ASSERT( bp->type() == BPTCallBack );
    delete bp;
}

static void
verifyList(std::vector<const bp::Object *> v) 
{
    CPPUNIT_ASSERT( v.size() == 7 );
    CPPUNIT_ASSERT( (long long) *(v[0]) == 1777889 );    
    CPPUNIT_ASSERT( v[1]->type() == BPTNull );
    CPPUNIT_ASSERT( (bool) *(v[2]) == true );
    CPPUNIT_ASSERT( (bool) *(v[3]) == false );
    CPPUNIT_ASSERT( !std::string("test string").compare(*(v[4])) );
    CPPUNIT_ASSERT( 3.1415 == (double) *(v[6]) );
    std::vector<const bp::Object *> v1 = *(v[5]);
    CPPUNIT_ASSERT_EQUAL( 3, (int) v1.size() );
    CPPUNIT_ASSERT_EQUAL( (long long) 1, (long long) *(v1[0]) );
    CPPUNIT_ASSERT_EQUAL( (long long) 2, (long long) *(v1[1]) );
    CPPUNIT_ASSERT_EQUAL( (long long) 3, (long long) *(v1[2]) );
}

void
BPObjectTest::listTest()
{
    bp::Object * bp = bp::Object::fromPlainJsonString(
        "[1777889,null,true,false,\"test string\",[1,2,3],3.1415]");
    verifyList(*bp);
    
    // copy
    {
        bp::List copy(*((bp::List *) bp));
        verifyList(copy);
    }

    // clone
    {
        bp::Object * clone = bp->clone();
        verifyList(*clone);
        delete clone;
    }

    // ass
    {
        bp::List ass;
        ass.append(new bp::String("foobar"));
        ass.append(new bp::Integer(77));        
        ass = *((bp::List *) bp);
        verifyList(ass);
    }

    delete bp;
}

void
BPObjectTest::mapTest()
{
    // test basic map creation, and serialization/rehydration
    bp::Map m1;
    m1.add("key", new bp::Integer(77));
    m1.add("t", new bp::String("how about 't' as a key?"));
    m1.add("v", new bp::String("v"));    
    // make sure a callback makes it through
    m1.add("cb", new bp::CallBack(42));    

    std::string jsonRep = m1.toJsonString();
    CPPUNIT_ASSERT( !jsonRep.empty() );

    // rehydrate
    bp::Object * o = bp::Object::fromJsonString(jsonRep);

    CPPUNIT_ASSERT( o->has("key", BPTInteger) );
    CPPUNIT_ASSERT( o->has("t", BPTString) );    
    CPPUNIT_ASSERT( o->has("v", BPTString) );
    CPPUNIT_ASSERT( o->has("cb", BPTCallBack) );

    CPPUNIT_ASSERT( o != NULL );    

    delete o;
}

void
BPObjectTest::exceptionTest()
{
    bp::String s("this is NOT a boolean.  duh.");
    CPPUNIT_ASSERT_THROW( (bool) s, bp::ConversionException );

    // NOTE: this is messed up on Darwin.  The throwing of the exception
    // causes abort() to be called!  it never gets out of the operator.
    //
    // #0  0x9184ab9e in __kill ()
    // #1  0x9184ab91 in kill$UNIX2003 ()
    // #2  0x918c1ec2 in raise ()
    // #3  0x918d147f in abort ()
    // #4  0x929ed005 in __gnu_cxx::__verbose_terminate_handler ()
    // #5  0x929eb10c in __gxx_personality_v0 ()
    // #6  0x929eb14b in std::terminate ()
    // #7  0x929eb190 in std::set_unexpected ()
    // #8  0x929eac0f in __cxa_call_unexpected ()
    // #9  0x0005fcb8 in bp::Map::operator[] (this=0xbfffefd8, key=0x8cfff "doesn't exist") at /Users/lloydh/plane_hacking/Product/Core/Main/BPUtils/src/Utils/bptypeutil.cpp:357
    // #10 0x00019d5c in BPObjectTest::exceptionTest (this=0x30a9b0) at /Users/lloydh/plane_hacking/Product/Core/Main/BPUtils/test/Utils/BPObjectTest.cpp:151

    // bp::Map m;
    // CPPUNIT_ASSERT_THROW ( m["doesn't exist"], bp::error::Exception );
}

void
BPObjectTest::parsingTest()
{
    static const char * json = 
        "{"
        "  \"bool\": false,"
        "  \"integer\": 77577,"        
        "  \"double\": 3.1415,"
        "  \"list\": [false,true,0.00]"
        "}";
    
    // now go from plain json to internal, clone, go to bpobject json to
    // internal to plain json to internal and THEN verify the structure
    // is correct.
    bp::Object *bp, *start = bp::Object::fromPlainJsonString(json);
    bp = start->clone();
    delete start;
    
    std::string s = bp->toJsonString();
    delete bp;
    bp = bp::Object::fromJsonString(s);
    s = bp->toPlainJsonString();
    delete bp;
    bp = bp::Object::fromPlainJsonString(s);
    
    // whew.  now validate
    CPPUNIT_ASSERT( (bool) (*bp)["bool"] == false );
    CPPUNIT_ASSERT( (long long) (*bp)["integer"] == 77577 );
    CPPUNIT_ASSERT( (double) (*bp)["double"] == 3.1415 );
    CPPUNIT_ASSERT( (bool) (*bp)["list"][(unsigned) 1] == true );

    // just ensure we don't crash on bogus json
    static const char * bogusJson1 = "{\"t\":\"integer\",\"v\":\"ooga\"}";
    bp = bp::Object::fromJsonString(bogusJson1);
    if (bp) delete bp;

    static const char * bogusJson2 = "{\"t\":\"string\"}";
    bp = bp::Object::fromJsonString(bogusJson2);
    if (bp) delete bp;
    
    static const char * bogusJson3 = "[\"t\", \"string\"]";
    bp = bp::Object::fromJsonString(bogusJson3);
    if (bp) delete bp;

    static const char * bogusJson4 = "{\"t\": false,\"v\":\"ooga\"}";
    bp = bp::Object::fromJsonString(bogusJson4);
    if (bp) delete bp;
}
