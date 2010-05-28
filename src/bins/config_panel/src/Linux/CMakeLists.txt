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
# Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
# All rights reserved.
# 
# Contributor(s): 
# ***** END LICENSE BLOCK *****
# looser syntax

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# INCLUDE and link paths for platform libraries
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../../dist/include")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../../dist/lib")

# INCLUDE path for config panel Common headers
INCLUDE_DIRECTORIES("../Common")

SET(HDRS ../Common/ScriptableConfigObject.h)
SET(SRCS bpprefpane.cpp ../Common/ScriptableConfigObject.cpp)

SET(BINNAME BrowserPlusPrefs)

ADD_EXECUTABLE(${BINNAME} ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(${BINNAME} BPProtocol_s
                      HTMLRender_s BPUtils_s ArchiveLib_s ${OS_LIBS})
