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

/*
 *  BPLogFile.h
 *
 *  Created by David Grigsby on 4/1/2010.
 *  Copyright 2010 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef BPLOGFILE_H_
#define BPLOGFILE_H_


namespace bp {
namespace log {


enum FileMode
{
    kTruncate,
    kAppend,
    kSizeRollover
};

const int kDefaultRolloverKB = 256;


} // log
} // bp


#endif 
