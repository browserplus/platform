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
############################################################
#  Created by Lloyd Hilaiel on May 1 2006
#  Copyright (c) Yahoo!, Inc 2006
#
#  A CMake include file (pulled in by PublicMacros.cmake)
#  that implements test building
############################################################

############################################################
# _YBT_BUILD_TEST -- 
# Define a test to be built. 
############################################################
MACRO (_YBT_BUILD_TEST name)

  IF (WIN32 AND NOT DEFINED ${name}_NO_VERSION_RESOURCES)
    _YBT_HANDLE_WIN32_VERSION_RESOURCES(${name})
  ENDIF ()

  # schedule the test for building
  IF ("${ARGN}" STREQUAL "WIN32")
	ADD_EXECUTABLE(${name} WIN32 ${${name}_SOURCES} ${${name}_HEADERS})
  ELSE ()
	ADD_EXECUTABLE(${name} ${${name}_SOURCES} ${${name}_HEADERS})
  ENDIF ()

  # now we've got to figure out how to get the built bits into 
  # your dist directory for you.  output path is probably a bad
  # solution, because you may not have different output paths
  # per cmake invocation.
  # (XXX so in the future, we'll symlink this over...)
  SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/tests)

  _YBT_SETUP_PCH(${name} ${name})

  # Sets up XPConnect client stuff properly.
  IF (WIN32)
    ADD_DEFINITIONS(-DXP_WIN)
  ELSE ()
    ADD_DEFINITIONS(-DXP_UNIX)
  ENDIF ()
ENDMACRO ()
