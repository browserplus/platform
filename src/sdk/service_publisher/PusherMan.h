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

#ifndef __PUSHERMAN_H__
#define __PUSHERMAN_H__

#include <string>
#include "BPUtils/bpfile.h"

/**
 * publish a zipfile to the browser plus distribution server
 * \return bool - true iff server returned 200
 * \param file - the path to the zipfile
 * \param baseurl - the base url to the webservice
 *                  (e.g. http://somehost.yahoo.com/api)
 */
bool pushFile(bp::file::Path file, std::string baseurl,
              std::string coreletName, std::string coreletVersion,
              std::string platform);

#endif
