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
# Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
# All Rights Reserved.
# 
# Contributor(s): 
# ***** END LICENSE BLOCK *****
######################################################################
# cmakelists.txt for BP Archiver library
#
SET(libName ArchiveLib)

SET(${libName}_MAJOR_VERSION 0)
SET(${libName}_MINOR_VERSION 1)

# Tell build templates who we consume.
# This enables bt to emit link statements for either ourselves (dll case) 
# or our clients (static lib case).
# Note: currently (01/28/09): libs listed here must have name ending *_s
SET(${libName}_LINK_STATIC BPUtils archive easylzma)

# Build our target as a static lib.
YBT_BUILD(LIBRARY_STATIC ${libName})

ADD_DEPENDENCIES(${libName}_s BPUtils_s)
