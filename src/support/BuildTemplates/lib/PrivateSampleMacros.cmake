############################################################
#  Created by Lloyd Hilaiel on May 1 2006
#  Copyright (c) Yahoo!, Inc 2006
#
#  A CMake include file (pulled in by PublicMacros.cmake)
#  that implements sample building
############################################################

############################################################
# _YBT_BUILD_SAMPLE -- 
# Define a sample to be built. 
############################################################
MACRO (_YBT_BUILD_SAMPLE name)

  IF (WIN32 AND NOT DEFINED ${name}_NO_VERSION_RESOURCES)
    _YBT_HANDLE_WIN32_VERSION_RESOURCES(${name})
  ENDIF ()

  # gather up all sources into the allSources var
  SET(allSources ${${name}_SOURCES} ${${name}_HEADERS}) 

  # schedule the binary for building
  IF ("${ARGN}" STREQUAL "WIN32")
	ADD_EXECUTABLE(${name} WIN32 ${allSources})
  ELSE ()
	ADD_EXECUTABLE(${name} ${allSources})
  ENDIF ()

  # schedule ALL sources for installation in dist/samples	 
  _YBT_SYMLINK_FILES_DURING_TARGET_P(${name} POST_BUILD samples/${name}/
	                                ${CMAKE_CURRENT_SOURCE_DIR}
									${allSources})

  # and install the samples cmake file
  _YBT_SYMLINK_FILES_DURING_TARGET(${name} POST_BUILD samples/${name}/
	                              ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)


  # drop the sample binary in the current binary dir
  SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

  _YBT_SETUP_PCH(${name} ${name})
  # Sets up XPConnect client stuff properly.
  IF (WIN32)
    ADD_DEFINITIONS(-DXP_WIN)
  ELSE ()
    ADD_DEFINITIONS(-DXP_UNIX)
  ENDIF ()
ENDMACRO ()
