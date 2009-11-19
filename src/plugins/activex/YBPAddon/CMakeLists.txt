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
# The YBPAddon ActiveX control
#
# TODO: 		
#   Get it registering in release
#   put in stuff from npapi\CMakeLists.txt
#   remove _CRT_SECURE_NO_DEPRECATE

SET (TARGET "YBPAddon")
PROJECT(${TARGET})


# Gather Source Files
FILE (GLOB HDRS *.h)
FILE (GLOB SRCS *.cpp)
FILE (GLOB RESRCS *.rc)


# File groupings for IDEs
SOURCE_GROUP(Headers FILES ${HDRS})
SOURCE_GROUP(Sources FILES ${SRCS})
SOURCE_GROUP(Resources FILES ${RESRCS})

# Add the build target
ADD_LIBRARY(${TARGET} SHARED ${HDRS} ${SRCS} ${RESRCS})

# CMake build flags 
ADD_DEFINITIONS(-DWIN32)

SET(CMAKE_CXX_WARNING_LEVEL "4" CACHE STRING "Warning level" FORCE)
SET(CMAKE_CXX_USE_RTTI YES CACHE BOOL "RTTI" FORCE)
SET(CMAKE_CXX_WIN9X_UNICODE NO CACHE BOOL "WIN9X_UNICODE" FORCE)
SET(CMAKE_ATL_FLAG "1" CACHE STRING "ATL" FORCE)

# Compiler options
ADD_DEFINITIONS(-D_ATL_ATTRIBUTES -D_USRDLL)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../dist/include")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../dist/include")

# Linker options
SET(OS_LIBS comsuppw DbgHelp libeay32_s ssleay32_s Ws2_32)
TARGET_LINK_LIBRARIES(${TARGET} ${OS_LIBS} 
                      BPProtocol_s BPUtils_s HTMLRender_s PluginCommonLib_s)
SET_TARGET_PROPERTIES(${TARGET} PROPERTIES LINK_FLAGS "/MACHINE:X86")
