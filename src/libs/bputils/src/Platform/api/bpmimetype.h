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
 *  bpmimetype.h
 *
 *  Created by Lloyd Hilaiel on 6/20/08.
 */

#ifndef _BPMIMETYPE_H_
#define _BPMIMETYPE_H_

#include <string>
#include <vector>
#include <set>

#include "BPUtils/bpfile.h"

// Known mimetypes are from  http://www.webmaster-toolkit.com/mime-types.shtml,
// http://support.microsoft.com/kb/288102, and http://developer.apple.com/documentation/AppleApplications/Reference/SafariWebContent/CreatingContentforSafarioniPhone/chapter_2_section_11.html
// with "application/x-folder" added for folders.  Can also augment mimetypes
// via config file.
//
namespace bp {
    namespace mimetype {
        // "application/x-folder"
        extern const std::string kFolderMimeType;
        
        // Given a path, return a set of common mime types.
        // If file extension is unrecognized,  "application/unknown"
        // is returned.
        std::set<std::string> fromPath(const bp::file::Path& path);

        // given a mime type, return a list of possible file extensions
        // the returned list is sorted
        std::vector<std::string> extensionsFromMimeType(
            const std::string& mimeType);
        
        // Does one of a path's mimetypes match one of a specifed set
        // of mimetypes?  An empty filter matches everything.
        bool pathMatchesFilter(const bp::file::Path& path,
                               const std::set<std::string>& mimeTypes);
    }
}

#endif
