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

/************************************************************************
 * BPHandleMapper.h
 *
 * Written by Gordon Durand, Tue Sep 4 2007
 * (c) 2007, Yahoo! Inc, all rights reserved.
 */

#ifndef __BPHANDLEMAPPER_H__
#define __BPHANDLEMAPPER_H__

#include <map>
#include <string>
#include "BPUtils/bptypeutil.h"
#include "PluginCommonLib/BPHandle.h"



class BPHandleMapper
{
public:
    // get a handle for a path
    static BPHandle pathToHandle(const bp::file::Path& path);
        
    // get the path represented by a handle
    static bp::file::Path handleValue(const BPHandle& handle);
    
    // Move between bp::Object with embedded BPTPaths, and those
    // without, inserting or expanding handles along the way.  
    //
    // caller assumes ownership of returned values
    //
    static bp::Object* insertHandles(const bp::Object* bpObj);
    static bp::Object* expandHandles(const bp::Object* bpObj);

private:
    BPHandleMapper() {};
    ~BPHandleMapper() {};
};

#endif
