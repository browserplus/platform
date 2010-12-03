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
 *  bpurlcollection.h
 *  A persistent collection of URLs suitable for whitelisting or
 *  blacklisting tasks.
 *  
 *  Created by Lloyd Hilaiel on 04/29/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 */

#ifndef __BPURLCOLLECTION_H__
#define __BPURLCOLLECTION_H__

#include <string>
#include "BPUtils/bpfile.h"
#include "BPUtils/bptypeutil.h"

namespace bp {
    class URLCollection 
    {
      public:
        URLCollection();
        ~URLCollection();
        
        /**
         * initialize a URLCollection from a disk file.  If the file doesn't
         * exist, it will be created.
         *
         * \param useDomainForHTTP - if true, when http(s) urls are
         *              encountered the domain will be parsed out and used
         *              as the element of comparison.  All other domains
         *              will be treated as opaque strings.
         *
         * \returns false if file could not be created or couldn't be
         *          parsed.
         */ 
        bool init(const boost::filesystem::path & path, bool useDomainForHTTP = true);

        /**
         * Check if a url exists in the collection
         *
         * \returns false if the url (string) does not exist,
         *                or if initialization failed
         */
        bool has(const std::string & url);

        /**
         * Add a url to the collection
         *
         * \returns false if the url (string) does not exist,
         *                or if initialization failed
         */
        bool add(const std::string & url);        

      private:
        boost::filesystem::path m_path;
        bool m_useDomainForHTTP;
        bp::Map * m_data;
        bp::List * m_list;

        bool writeToDisk();
        std::string normalizeURL(const std::string & url);
    };
}

#endif


   
