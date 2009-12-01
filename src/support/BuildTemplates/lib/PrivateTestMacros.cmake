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
