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
 *  bplocalization.h
 *    Localization utils
 *  
 *  Created by Gordon Durand on Thu March 27 2008.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 */

#ifndef __BPLOCALIZATION_H__
#define __BPLOCALIZATION_H__

#include <string>
#include <vector>
#include <map>

#include "BPUtils/bpfile.h"

namespace bp {
    namespace localization {

        /**
         * get the current user's locale
         */
        std::string getUsersLocale();

        /** get a localized string
         *
         *  parameters: 
         *    key - the key.  multi-level keys are represented as "key1/key2"
         *    locale - desired locale
         *    outVal - a reference where the value is placed if the key
         *             is present.  If the key is not present, outval will
         *             contain the input key.
         *    major, minor, micro - cached platform version
         *    useUpdateCache - if true, look in platform update cache, else
         *                     look in installed platforms
         *
         *  \returns true if the key exists, false otherwise 
         */ 
        bool getLocalizedString(const std::string& key, 
                                const std::string& locale,
                                std::string & outVal,
                                unsigned int major = -1,
                                unsigned int minor = -1,
                                unsigned int micro = -1,
                                bool useUpdateCache = false);
        
        /** get a localized string
        *
        *  parameters: 
        *    key - the key.  multi-level keys are represented as "key1/key2"
        *    locale - desired locale
        *    outVal - a reference where the value is placed if the key
        *             is present.  If the key is not present, outval will
        *             contain the input key.
        *    stringsPath - pathname of string localizations to use
        *
        *  \returns true if the key exists, false otherwise 
        */ 
        bool getLocalizedString(const std::string& key, 
                                const std::string& locale,
                                std::string & outVal,
                                const bp::file::Path& stringsPath);

        /**
         *  get all localizations for a given key
         * 
         * \param key the key
         *
         * \returns a map of locale -> string
         */ 
        std::map<std::string, std::string>
            getLocalizations(const std::string& key);
                                
        /** Get an ordered vector of candidate locales for a given locale.  
         *  The format of a locale is "language[-country][.codeset][@modifier]" 
         *  ('_' will be accepted in place of '-').
         *  The candidates represent a progressive "loosening" of the locale with 
         *  "en" at the root.  For example, given an input of "en-GB.ASCII@Default",
         *  the returned vector will be "en-GB.ASCII@Default, en-GB.ASCII, en-GB, en" 
         */
        std::vector<std::string>
            getLocaleCandidates(const std::string& locale);

        /**
         * Given a path to a directory containing subdirectories, one
         * per supported locale, and the user's locale, return the best
         * match, or an empty string if no match was found.
         */
        bp::file::Path getLocalizedUIPath(const bp::file::Path & uiDir,
                                          const std::string & locale);
    }
}

#endif
