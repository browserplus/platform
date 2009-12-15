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

/*
 *  bpphash.h
 *  Persistent application wide storage of string based key/value pairs
 *  
 *  Created by Lloyd Hilaiel on 10/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */

#ifndef __BPPHASH_H__
#define __BPPHASH_H__

#include <string>

namespace bp {
    namespace phash {

        /** get a key from the application wide persistent disk store
         *
         *  parameters: 
         *    key - the key
         *    outVal - a reference where the value is placed if the key
         *             is present
         *
         *  \returns true if the key exists, false otherwise */ 
        bool get(const std::string & key, std::string & outVal);

        /** set a key from the application wide persistent disk store
         *  will overwrite an existing value.
         *
         *  \returns true if the key was successfully written
         */ 
        bool set(const std::string & key, const std::string& val);

        /** remove a key from the application wide persistent disk store
         */ 
        void remove(const std::string & key);
    }
}

#endif


   
