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

/************************************************************************
 * BPHandle.h
 *
 * Written by Gordon Durand, Tue Sep 4 2007
 * (c) 2007, Yahoo! Inc, all rights reserved.
 */

#ifndef __BPHANDLE_H__
#define __BPHANDLE_H__

#include <vector>
#include <string>
#include "BPUtils/bpfile.h"   // for boost::uintmax_t

namespace bp {
    class Map;
}

// A BPHandle is represented is JavaScript as an object
// with the following key/value pairs:
//  BrowserPlusHandleID : <unique integer id>
//  BrowserPlusType : BPObject type hidden by this handle (e.g. "BPTPath")
//  BrowserPlusHandleName : handle's "safe" name (e.g. leaf name for paths)
//
// BPHandleMapper maps between BPHandles and the real resource
// name that they hide.
//
// Currently, only bp::Path maps to a BPHandle.
//

#define BROWSERPLUS_HANDLETYPE_KEY "BrowserPlusType"
#define BROWSERPLUS_HANDLEID_KEY "BrowserPlusHandleID"

// deprecated (overly verbose)
#define DEPRECATED_BROWSERPLUS_HANDLENAME_KEY "BrowserPlusHandleName"

// moving forward
#define BROWSERPLUS_HANDLENAME_KEY "name"
#define BROWSERPLUS_HANDLESIZE_KEY "size"
#define BROWSERPLUS_HANDLEMIMETYPE_KEY "mimeType"

class BPHandle
{
public:
    BPHandle(const std::string& type,
             int id,
             const std::string& safeName,
             boost::uintmax_t size,
             const std::vector<std::string>& mimeTypes);
    ~BPHandle();
    
    std::string type() const;
    int id() const;
    std::string name() const;
    boost::uintmax_t size() const;
    std::vector<std::string> mimeTypes() const;

    bool operator<(const BPHandle& other) const;
    
    // create a Map from a handle.  caller assumes ownership.
    // mostly a convenience for drop handlers
    bp::Map* toBPMap() const;

private:
    friend class BPHandleMapper;
    std::string m_type;
    int m_id;
    std::string m_name;
    boost::uintmax_t m_size;
    std::vector<std::string> m_mimeTypes;
};

#endif
