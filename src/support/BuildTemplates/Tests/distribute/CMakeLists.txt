# ***** BEGIN LICENSE BLOCK *****
# The contents of this file are subject to the Mozilla Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
# 
# The Original Code is BrowserPlus (tm).
# 
# The Initial Developer of the Original Code is Yahoo!.
# Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
# All rights reserved.
# 
# Contributor(s): 
# ***** END LICENSE BLOCK *****

# Test YBT_DISTRIBUTE_FILE

# include build templates
SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

YBT_DISTRIBUTE_FILES(share/foo some/path/afile.txt some/path/bfile.txt)
YBT_DISTRIBUTE_FILES(share PRESERVE
                     some/other/path/afile.txt
                     some/path/afile.txt)

