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

#include <string>
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"


namespace bp {
namespace install {
    
class Uninstaller 
{
 public:
    Uninstaller(const boost::filesystem::path& logFile,
				bp::log::Level logLevel);
    virtual ~Uninstaller();

    void run(bool fromRunonce = false);

 protected:
     boost::filesystem::path m_dir;
	 boost::filesystem::path m_logFile;
	 bp::log::Level m_logLevel;
     bool m_error;
     void removeDirIfEmpty(const boost::filesystem::path& dir);
     void removeServices();
#ifdef WIN32
     void scheduleRunonce();
#endif
};

}}
