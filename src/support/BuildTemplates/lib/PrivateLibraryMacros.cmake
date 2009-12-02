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

############################################################
#  Created by Lloyd Hilaiel on May 1 2006
#  Copyright (c) Yahoo!, Inc 2006
#
#  A CMake include file (pulled in by PublicMacros.cmake)
#  that implements library building
############################################################

######################################################################
# _YBT_CREATE_UBER_HEADER - a macro to create one uber header file
# containing references to all public headers.
# ARGUMENTS:
#  headerName - The target containing public header includes.
#  projectName - The name of the project that this header corresponds to.
#                This value will make it into the documentation of the
#                generated file 
#  ARGN - All public headers.
######################################################################
MACRO(_YBT_CREATE_UBER_HEADER headerName projectName)
  GET_FILENAME_COMPONENT(headerFileName ${headerName} NAME)
  GET_FILENAME_COMPONENT(headerFileNameNoExtension ${headerName} NAME_WE)
  STRING(TOUPPER ${headerFileNameNoExtension} headerFileNameNoExtensionToUpper)
  IF (YBT_NESTED_INCLUDES)
    SET(headerPath "${projectName}/") 
  ELSE ()
    SET(headerPath "") 
  ENDIF ()

  SET(LIST_OF_INCLUDES
"/**
 * ${headerFileName} -- Main include file for the \"${projectName}\" library
 * ${headerFileNameNoExtension}
 *
 * Include this header file when using shared or static libraries
 * associated with \"${projectName}\".  While not _required_ It will
 * ensure that headers are included in the required order.
 *
 * Generated automatically by YCP Build Templates (YBT), using CMake.
 * Copyright (c) 2006 Yahoo!, Inc.  All rights reserved.
 */

#ifndef __${headerFileNameNoExtensionToUpper}_UBER_HEADER_H__
#define __${headerFileNameNoExtensionToUpper}_UBER_HEADER_H__\n\n")

  FOREACH(publicHeader ${ARGN})
    GET_FILENAME_COMPONENT(publicHeader ${publicHeader} NAME)
    SET(LIST_OF_INCLUDES ${LIST_OF_INCLUDES}
        \#include " " \"${headerPath}${publicHeader}\"\n)
  ENDFOREACH ()
  SET(LIST_OF_INCLUDES ${LIST_OF_INCLUDES}
    "\n#endif // __${headerFileNameNoExtensionToUpper}_H__\n")
  SET(LIST_OF_INCLUDES ${LIST_OF_INCLUDES} "\n\n")
  FILE(WRITE ${headerName} ${LIST_OF_INCLUDES})
ENDMACRO ()

############################################################
# _YBT_BUILD_LIBRARY -- 
# Define a library to be built.  A thin wrapper around ADD_LIBRARY
# which defines both shared and static libraries
# 
# ARGUMENTS:
#  NAME -- the "base" name of the library.  
# VARIABLE INPUTS:
#  ${name}_SOURCES
#  ${name}_PRIVATE_HEADERS
#  ${name}_PUBLIC_HEADERS
############################################################
MACRO (_YBT_BUILD_LIBRARY name type)
  SET(allSources ${ARGN})
  SET(majorVersion ${${name}_MAJOR_VERSION})
  SET(minorVersion ${${name}_MINOR_VERSION})
  SET(microVersion ${${name}_MICRO_VERSION})

  IF (WIN32) 
    SET (compilerInclude "/FI")
  ELSE () 
    SET (compilerInclude "-include")
  ENDIF () 
  ADD_DEFINITIONS(-D${name}_BUILD)
  FOREACH (file ${allSources})
    IF (NOT "${file}" MATCHES "\\.c$" AND
        NOT "${file}" MATCHES "\\.h$" AND
        NOT "${file}" MATCHES "\\.js$" AND
        NOT "${file}" MATCHES "\\.rc$" AND
        NOT "${file}" MATCHES "\\.idl$" AND
        NOT "${file}" MATCHES "\\.xpidl$")
      GET_FILENAME_COMPONENT(shortFile ${file} NAME)
      STRING(REGEX REPLACE "_s$" "" baseName ${name})
    ENDIF ()
  ENDFOREACH ()

  ADD_LIBRARY(${name} ${type} ${allSources}) 
  SET_TARGET_PROPERTIES(${name} PROPERTIES SOVERSION ${majorVersion})
  SET_TARGET_PROPERTIES(${name} PROPERTIES VERSION ${majorVersion}.${minorVersion})

  IF (APPLE)
    # don't resolve symbols at shared library link time on mac. 
    SET_TARGET_PROPERTIES(
      ${name} PROPERTIES LINK_FLAGS "-undefined suppress -flat_namespace")
  ENDIF ()

  IF (NOT DEFINED YBT_NO_HEADERS_IN_DIST)
    STRING(REGEX REPLACE "_s$" "" baseName ${name})

    # and schedule public headers for installation
    FOREACH (header ${${name}_PUBLIC_HEADERS})
      STRING(REGEX REPLACE ".*api(.*)/[^/]*\\.h" "\\1" dir ${header})
      IF (WIN32 AND NOT ${CMAKE_MAKE_PROGRAM} STREQUAL "nmake")
        _YBT_SYMLINK_FILES_DURING_TARGET(
          ${name} PRE_BUILD include/${baseName}${dir}  ${header})
      ELSE ()
        # the Makefile generator of cmake doesn't support PRE_BUILD,
        # so we symlink at cmake time
        _YBT_SYMLINK_FILES(include/${baseName}${dir} ${header})
      ENDIF ()
    ENDFOREACH ()
  
    # create an "uber header file" that includes all headers in the library.
  
    # If any public header files share the name of this file, we'll want to
    # error out (dissallow name collision regardless of case)
    STRING(TOLOWER ${baseName}.h uberHeaderName)
  
    FOREACH (header ${${name}_PUBLIC_HEADERS})
      GET_FILENAME_COMPONENT(header ${header} NAME)
      STRING(TOLOWER ${header} header)
      IF ("${header}" STREQUAL "${uberHeaderName}")   
        MESSAGE(FATAL_ERROR
"!! ERROR: you have a header named \"${header}\", which collides with the
!!        uber header we create for you.  Please rename that header!")
      ENDIF ()   
    ENDFOREACH ()

    # now actually create the header
    _YBT_CREATE_UBER_HEADER(
        ${CMAKE_INSTALL_PREFIX}/include/${baseName}/${baseName}.h
        ${baseName} ${${name}_PUBLIC_HEADERS})
  ENDIF ()

  # now we've got to figure out how to get the built bits into 
  # your dist directory for you.  output path is probably a bad
  # solution, because you may not have different output paths
  # per cmake invocation.
  SET(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
  # move dlls on win32 to bin/
  SET(DLL_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)

  # now we must set ${name}_INCLUDE_DIR and ${name}_LINK_DIR
  IF (YBT_NESTED_INCLUDES)
    SET(${name}_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include
        CACHE STRING "include path for ${name}" FORCE)
  ELSE ()
    SET(${name}_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/${name}
        CACHE STRING "include path for ${name}" FORCE)
  ENDIF ()

  SET(${name}_LINK_DIR ${CMAKE_INSTALL_PREFIX}/lib
      CACHE STRING "link path for ${name}" FORCE)

  IF (DEFINED YBT_DEBUG)
    MESSAGE("DD   ${name}_INCLUDE_DIR ${${name}_INCLUDE_DIR}")
  ENDIF ()

  # Setup PCH for both libs
  _YBT_SETUP_PCH(${name} ${name})

  # Sets up XPConnect client stuff properly.
  IF (WIN32)
    ADD_DEFINITIONS(-DXP_WIN)
  ELSE ()
    ADD_DEFINITIONS(-DXP_UNIX)
  ENDIF ()

  # keep a list of all libraries in the cmake cache
  IF (DEFINED YBT_ALL_LIBRARIES)
    LIST(REMOVE_ITEM YBT_ALL_LIBRARIES ${name})
  ENDIF ()
  LIST(APPEND YBT_ALL_LIBRARIES ${name})

  SET (YBT_ALL_LIBRARIES "${YBT_ALL_LIBRARIES}"
       CACHE INTERNAL "A list of all the built libraries")
ENDMACRO ()

MACRO (_YBT_BUILD_STATIC_LIBRARY name)
  SET(allSources ${${name}_SOURCES} ${${name}_PRIVATE_HEADERS}
      ${${name}_PUBLIC_HEADERS}) 
  _YBT_BUILD_LIBRARY(${name} STATIC ${allSources})
ENDMACRO ()

MACRO (_YBT_BUILD_SHARED_LIBRARY name)
  IF (WIN32 AND NOT DEFINED ${name}_NO_VERSION_RESOURCES)
    _YBT_HANDLE_WIN32_VERSION_RESOURCES(${name})
  ENDIF ()

  SET(allSources ${${name}_SOURCES} ${${name}_PRIVATE_HEADERS}
      ${${name}_PUBLIC_HEADERS}) 

  _YBT_BUILD_LIBRARY(${name} SHARED ${allSources})

  SET_TARGET_PROPERTIES(${name} PROPERTIES DEFINE_SYMBOL ${name}_SHARED)
ENDMACRO ()

