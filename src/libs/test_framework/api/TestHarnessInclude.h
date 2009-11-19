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
 * TestHarnessInclude.h - A wrapper around CppUnit that includes
 *                        all cppunit files for you.  Part of the
 *                        TestingFramework static library that simplifies
 *                        the creation of unit tests
 *
 * Created by Lloyd Hilaiel on 6/23/2008
 * (c) Yahoo! 2008
 */

#ifndef _TESTHARNESSINCLUDE_H_
#define _TESTHARNESSINCLUDE_H_

#ifdef WIN32
#pragma warning ( push )
#pragma warning ( disable : 4512 )
#endif

#include <cppunit/Test.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCaller.h>
#include <cppunit/extensions/HelperMacros.h>

#ifdef WIN32
#pragma warning ( pop )
#endif

#endif
