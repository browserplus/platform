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
SET(Permissions_MAJOR_VERSION 0)
SET(Permissions_MINOR_VERSION 1)
SET(Permissions_LINK_STATIC DistributionClient BPUtils ArchiveLib)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../ServiceAPI/api")

YBT_BUILD(LIBRARY_STATIC Permissions)
ADD_DEPENDENCIES(Permissions_s DistributionClient_s BPUtils_s ArchiveLib_s)

