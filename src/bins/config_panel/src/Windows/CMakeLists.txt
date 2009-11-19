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
# INCLUDE path for config panel Common headers
INCLUDE_DIRECTORIES("../common")

# INCLUDE files
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../../dist/include")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../../dist/lib")

# headers/sources/resources common to all platforms
SET(HDRS ../common/ScriptableConfigObject.h)
SET(SRCS ConfigPanel.cpp ../common/ScriptableConfigObject.cpp)

# setup the build target and linkage
SET(EXENAME BrowserPlusPrefs)
ADD_EXECUTABLE(${EXENAME} WIN32 ${SRCS} ${HDRS} ConfigPanel.rc )
TARGET_LINK_LIBRARIES(${EXENAME} BPProtocol_s
                      HTMLRender_s BPUtils_s ArchiveLib_s)
