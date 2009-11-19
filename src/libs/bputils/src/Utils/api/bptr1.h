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

/*
 *  bptr1.h
 *
 *  Helps accomodate differences in C++ TR1 implementation between
 *  MSVC and GCC.
 *
 *  Created by David Grigsby on 5/28/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _BPTR1_H_
#define _BPTR1_H_

#ifdef WIN32
#include <memory>
#include <regex>
#else
#include <tr1/memory>
#include <boost/tr1/regex.hpp>
#endif

#endif // _BPTR1_H_
