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
#  that implements and documents all macros used internally by the
#  "BuildTemplates" build system helper.
#
#  You can use these in external projects, but they will probably
#  change more frequently than the set of "public macros".  If you've
#  got reason to reach in here, then drop a mail to client-devel
#  and we'll see how to widen the public macro interface to satisfy
#  your requirements.
############################################################

############################################################
# _YBT_FILTER_PLATFORM_SPECIFIC -- filter out filenames from a list
#           which do not correspond to the current platform.
# Example:
# _YBT_FILTER_STRINGS(outputVariable <filenames>)
#
# ARGUMENTS:
#   outputVariable - the variable to stuff the result into 
#   <strings> -- the list of filenames to filter
############################################################
MACRO(_YBT_FILTER_PLATFORM_SPECIFIC outputVar)
  # clear outputvar
  SET(${outputVar}) 

  # generate a list of allowed platforms
  SET (allowedPlats ${YBT_ALLOWED_PLATFORMS} ${CMAKE_SYSTEM_NAME})
  # now include UNIX if it's defined
  IF (UNIX)
    SET (allowedPlats ${allowedPlats} UNIX)
  ENDIF ()

  # filter unwanted files
  FOREACH (file ${ARGN})
    # first discover if this is a platform specific file 
    SET(filePlat)
    IF (${file} MATCHES "${YBT_PLATFORM_SEPARATOR}\([a-zA-Z]*\)\\.")
        STRING(REGEX REPLACE "^.*${YBT_PLATFORM_SEPARATOR}\([a-zA-Z]*\)\\.[a-zA-Z]*$" "\\1"
               filePlat ${file})
    ENDIF ()
    IF (filePlat) 
      # so we've got a platform specific file.  let's check and see if
      # it's pertinent on this platform
      SET(match false)
      FOREACH (plat ${allowedPlats})   
        IF (${plat} STREQUAL ${filePlat})
          SET(match true)   
        ENDIF ()
      ENDFOREACH ()
      IF (match)
          SET(${outputVar} ${${outputVar}} ${file})          
      ENDIF ()
    ELSE ()
      SET(${outputVar} ${${outputVar}} ${file}) 
    ENDIF ()
  ENDFOREACH ()
ENDMACRO ()


############################################################
# _YBT_FILTER_STRINGS -- filter out strings in a list which matches the 
#                   specified patterns.
# Example:
# _YBT_FILTER_STRINGS(outputVariable <patterns> STRINGS <strings>)
#
# ARGUMENTS:
#   outputVariable - the variable to stuff the result into 
#   <patterns> -- the list of exclusion patterns 
#   <strings> -- the list of strings to filter
############################################################
MACRO(_YBT_FILTER_STRINGS outputVar)
    SET (${outputVar} "")
    SET (patterns "")
    SET (strings "NOTFOUND")

    FOREACH (arg ${ARGN})
        IF (NOT strings STREQUAL "NOTFOUND")           
            SET (strings ${strings} ${arg})
        ELSE ()
            IF (arg STREQUAL "STRINGS")
                SET (strings)
            ELSE ()
                SET (patterns ${patterns} ${arg})
            ENDIF ()
        ENDIF ()
    ENDFOREACH ()

    FOREACH (str ${strings})
      SET(exclude 0)
      FOREACH (pat ${patterns})
        IF (${str} MATCHES ${pat})
          SET(exclude 1)
        ENDIF ()
      ENDFOREACH ()

      IF (NOT ${exclude})
        SET (${outputVar} ${${outputVar}} ${str})
      ENDIF ()
    ENDFOREACH ()

ENDMACRO ()

############################################################
# _YBT_ABSOLUTIFY_PATHS-- given a list of files, make them absolute.
#                   If they're already absolute, leave em absolute
#
# ARGUMENTS:
#   outputVariable - the variable to stuff the result into 
#   <paths> -- the list of paths
############################################################
MACRO(_YBT_ABSOLUTIFY_PATHS outVar)
  SET(${outVar})
  FOREACH (path ${ARGN})
    SET(absPath "absPath-NOTFOUND" CACHE INTERNAL "" FORCE)
    FIND_FILE(absPath ${path} PATHS ${CMAKE_CURRENT_SOURCE_DIR}
              NO_DEFAULT_PATH)

    IF (NOT absPath)
#      MESSAGE("++ WARNING: can't make file \"${path}\" absolute")
      SET (absPath ${path})
    ENDIF ()

    SET(${outVar} ${${outVar}} ${absPath})
  ENDFOREACH ()
  SET(absPath "absPath-NOTFOUND" CACHE INTERNAL "" FORCE)
ENDMACRO ()

############################################################
# _YBT_VERIFY_EXISTENCE -- verify a list of files exists, issuing a
#                   warning if they don't.
#
# ARGUMENTS:
#   message -- a message to output   
#   <ARGN> -- the list of paths
############################################################
MACRO(_YBT_VERIFY_EXISTENCE message)
  FOREACH (path ${ARGN})
    # if an explicitly named source doesn't exist, we issue a warning  
    IF (NOT EXISTS ${path})
      MESSAGE(FATAL_ERROR "++ WARNING (${message}): ${path} does not exist!")
    ENDIF ()
  ENDFOREACH ()
ENDMACRO ()

############################################################
# _YBT_DETERMINE_LIBRARY_SOURCE_FILES -- 
#
# ARGUMENTS: 
#   name - the name of the object we're determining files for
#
# Determine which source files this project consists of.
# There are two different ways that a cmake file writer may
# specify sources:
# 1. with an explicit list
# 2. via implicit discovery of sources starting at the cwd
#    (optionally, ignore patters may be specified to ignore
#     all files matching them) 
#
# So the algorithm here is if the ${name}_SOURCES Soutvar is
# already set, make all files therein absolute paths.
# Otherwise dynamically determine the files to be included,
# filter out sources matching ignore patters (if specified)
# and then filter out platform specific sources that do not
# apply to this platform.
#
# RETURNS (via populated variables):
#   ${name}_SOURCES -- source files
#   ${name}_PRIVATE_HEADERS -- public header files
#   ${name}_PUBLIC_HEADERS -- private header files
#   ${name}_HEADERS -- all header files
#############################################################
MACRO(_YBT_DETERMINE_LIBRARY_SOURCE_FILES name)
  # first, determine source files
  SET(sources)
  # If ${name}_SOURCES is already defined, we're using the explicit
  # method of source specification
  IF (DEFINED ${name}_SOURCES)
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_SOURCES is set: explicit specification.")
    ENDIF ()
    _YBT_ABSOLUTIFY_PATHS(sources ${${name}_SOURCES})
    _YBT_VERIFY_EXISTENCE(${name} ${sources})

  ELSE ()
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_SOURCES is not set, implicit specification.")
    ENDIF ()
    FILE (GLOB_RECURSE sources *.cpp *.mm *.c *.js *.rc *.xpidl *.idl *.def)
    # XXX test file ignore patterns here
    _YBT_FILTER_STRINGS(sources ${${name}_IGNORE_PATTERNS}
                        STRINGS ${sources})
    _YBT_FILTER_PLATFORM_SPECIFIC(sources ${sources})
  ENDIF ()

  SET(${name}_SOURCES ${sources})

  # first we dynamically determine public and private header files.
  # later we will decide wether we want to use the values. 
  SET(publicHeaders)
  SET(privateHeaders)
  FILE (GLOB_RECURSE headers *.h)
  _YBT_FILTER_STRINGS(headers ${${name}_IGNORE_PATTERNS}
                      STRINGS ${headers})
  _YBT_FILTER_PLATFORM_SPECIFIC(headers ${headers})
  FOREACH (header ${headers}) 
    IF (${header} MATCHES "api/.*\\.h$")
      SET(publicHeaders ${header} ${publicHeaders})
    ELSE ()
      SET(privateHeaders ${header} ${privateHeaders})
    ENDIF ()
  ENDFOREACH ()

  # now, determine public header files
  IF (DEFINED ${name}_PUBLIC_HEADERS)
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_PUBLIC_HEADERS is set: explicit specification.")
    ENDIF ()
    _YBT_ABSOLUTIFY_PATHS(publicHeaders ${${name}_PUBLIC_HEADERS})
    _YBT_VERIFY_EXISTENCE(${name} ${publicHeaders})
  ELSE ()
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_PUBLIC_HEADERS is not set, implicit specification.")
    ENDIF ()
  ENDIF ()

  SET(${name}_PUBLIC_HEADERS ${publicHeaders})

  # and next, private headers
  IF (DEFINED ${name}_PRIVATE_HEADERS)
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_PRIVATE_HEADERS is set: explicit specification.")
    ENDIF ()
    _YBT_ABSOLUTIFY_PATHS(privateHeaders ${${name}_PRIVATE_HEADERS})
    _YBT_VERIFY_EXISTENCE(${name} ${privateHeaders})
  ELSE ()
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_PRIVATE_HEADERS is not set, implicit specification.")
    ENDIF ()
  ENDIF ()

  SET(${name}_PRIVATE_HEADERS ${privateHeaders})

  # and finally, all headers
  IF (DEFINED ${name}_HEADERS)
    MESSAGE(FATAL_ERROR "!! ${name}_HEADERS is meaningless for a library")
  ENDIF ()

  SET(${name}_HEADERS ${${name}_PRIVATE_HEADERS} ${${name}_PUBLIC_HEADERS}) 
ENDMACRO ()

############################################################
# _YBT_DETERMINE_SOURCE_FILES -- 
#
# ARGUMENTS: 
#   name - the name of the object we're determining files for
#
# For non-libraries, there is no "private" or "public" headers.
# This simpliefied version of _YBT_DETERMINE_LIBRARY_SOURCE_FILES
# checks if SOURCES is defined, if it is it's broken into
# headers and sources, paths are checked, and the output variables
# are populated.  If SOURCES is not specified, we automatically build
# up a list of sources and headers using the current source directory.
#
# RETURNS (via populated variables):
#   ${name}_SOURCES -- source files
#   ${name}_HEADERS -- all header files
#############################################################
MACRO(_YBT_DETERMINE_SOURCE_FILES name)
  # do some error checking to validate that the client is correctly
  # using the templates..
  IF (DEFINED ${name}_PRIVATE_HEADERS)
    MESSAGE(FATAL_ERROR
          "!! ${name}_PRIVATE_HEADERS is only meaningful for a library")
  ENDIF ()
  IF (DEFINED ${name}_PUBLIC_HEADERS)
    MESSAGE(FATAL_ERROR
          "!! ${name}_PUBLIC_HEADERS is only meaningful for a library")
  ENDIF ()
  IF (DEFINED ${name}_HEADERS)
    MESSAGE(FATAL_ERROR
          "!! don't define ${name}_HEADERS.  define ${name}_SOURCES")
  ENDIF ()

  IF (DEFINED ${name}_SOURCES)
        # if x_SOURCES is set, we split out headers and implementation files
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_SOURCES is set: explicit specification.")
    ENDIF ()

    # do the split
    SET (headers)
    SET (sources)
    FOREACH (file ${${name}_SOURCES}) 
      IF (${header} MATCHES ".*\\.h$")
        SET(headers ${file} ${headers})
          ELSE ()
        SET(sources ${file} ${sources})
          ENDIF ()
    ENDFOREACH ()

        # should we really be doing the platform specific filtering here?
    # seems useful, but there may be use cases where it's not desired
    _YBT_FILTER_PLATFORM_SPECIFIC(sources ${sources})
    _YBT_ABSOLUTIFY_PATHS(sources ${sources})
    _YBT_VERIFY_EXISTENCE(${name} ${sources})
    SET(${name}_SOURCES ${sources})

    _YBT_FILTER_PLATFORM_SPECIFIC(headers ${headers})
    _YBT_ABSOLUTIFY_PATHS(headers ${headers})
    _YBT_VERIFY_EXISTENCE(${name} ${headers})
    SET(${name}_HEADERS ${headers})
  ELSE ()
    IF(DEFINED YBT_VERBOSE)
      MESSAGE("**   ${name}_SOURCES is not set: implicit specification.")
    ENDIF ()

        # if x_SOURCES is not set, we find all headers and sources under the
        # current source dir

    # find source files
    FILE (GLOB_RECURSE sources *.cpp *.mm *.c *.rc)
    _YBT_FILTER_STRINGS(sources ${${name}_IGNORE_PATTERNS}
                        STRINGS ${sources})
    _YBT_FILTER_PLATFORM_SPECIFIC(sources ${sources})
    SET(${name}_SOURCES ${sources})

    # find header files
    FILE (GLOB_RECURSE headers *.h)
    _YBT_FILTER_STRINGS(headers ${${name}_IGNORE_PATTERNS}
                        STRINGS ${headers})
    _YBT_FILTER_PLATFORM_SPECIFIC(headers ${headers})
    SET(${name}_HEADERS ${headers})
  ENDIF ()
ENDMACRO ()

############################################################
# Copy in .rc and .h files containing version information and add them
# to the build
############################################################
MACRO(_YBT_HANDLE_WIN32_VERSION_RESOURCES targetName)
  SET(baseName "${targetName}Version_Windows")
  SET(versionHeader "${CMAKE_CURRENT_BINARY_DIR}/${baseName}.h")
  SET(versionResource "${CMAKE_CURRENT_BINARY_DIR}/${baseName}.rc")

  # first copy in the header
  _YBT_SYMLINK_OR_COPY("${YBT_PATH}/resources/VersionTemplate_Windows.h"
                       ${versionHeader})
  # add it to the list of headers for this target
  SET(${targetName}_HEADERS ${${targetName}_HEADERS} ${versionHeader})
  SET(${targetName}_PRIVATE_HEADERS ${${targetName}_PRIVATE_HEADERS} ${versionHeader})  
  # second copy in the resource
  set(majorVersionNumber "${${targetName}_MAJOR_VERSION}")
  set(minorVersionNumber "${${targetName}_MINOR_VERSION}")  
  set(microVersionNumber "${${targetName}_MICRO_VERSION}")  
  set(targetName "${targetName}")  
  CONFIGURE_FILE("${YBT_PATH}/resources/VersionTemplate_Windows.rc" ${versionResource})
  # add it to the list of source files for this target
  SET(${targetName}_SOURCES ${${targetName}_SOURCES} ${versionResource})
  # add to source group so they are handled properly by cmake
  SOURCE_GROUP("${targetName} version resource files" FILES ${versionResource})
  SOURCE_GROUP("${targetName} version header files" FILES ${versionHeader})
ENDMACRO ()

############################################################
# Set up include directories based on the included headers
############################################################
MACRO(_YBT_SET_INCLUDE_DIRECTORIES)
  SET(headerDirs)
  SET(lastHeaderDir)
  IF (WIN32)
    SET(headerDirs "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)")
  ENDIF ()
  FOREACH (headerPath ${ARGN})
    GET_FILENAME_COMPONENT(headerDir ${headerPath} PATH)
    IF (NOT "${headerDir}" STREQUAL "${lastHeaderDir}") 
      SET(lastHeaderDir ${headerDir})
      SET(headerDirs ${headerDir} ${headerDirs})
    ENDIF ()
  ENDFOREACH ()
  # if the YBT_NESTED_INCLUDES option is set, we don't consume headers
  # out of the source tree, and we _do_ consume headers out of the
  # dist tree, such that from in tree you may consume headers with
  # LIBNAME/HEADERNAME.h
  IF (YBT_NESTED_INCLUDES)
#    we don't filter out api dirs.  This is more convenient, but does
#    not enforce proper prepending of libraries at library build time. 
#    _YBT_FILTER_STRINGS(headerDirs ".*api" STRINGS ${headerDirs})
    INCLUDE_DIRECTORIES (${CMAKE_INSTALL_PREFIX}/include)
  ENDIF ()

  # Now HEADER_DIRS has all the sub-directories containing headers!
  INCLUDE_DIRECTORIES (${headerDirs})
  IF (DEFINED YBT_DEBUG)
    MESSAGE("DD   include dirs from headers: ${headerDirs}")
  ENDIF ()
ENDMACRO ()

############################################################
# _YBT_SYMLINK_FILE_DURING_TARGET -- symlink (or copy on windows) 
#                 a file from a source location to a destination
#                 location before or after building the specified target
#   target   - CMake target to hang the operation off of
#   when     - either PRE_BUILD or POST_BUILD                             
#   srcFile  - source file
#   destFile - destination file name
############################################################
MACRO(_YBT_SYMLINK_FILE_DURING_TARGET target when cmakeSrcFile cmakeDestFile)
    # first we create the directory we're copying into
    GET_FILENAME_COMPONENT(_destDir ${cmakeDestFile} PATH)
    FILE(MAKE_DIRECTORY ${_destDir})

    # now translate paths to native paths 
    FILE(TO_NATIVE_PATH ${cmakeDestFile} destFile)
    FILE(TO_NATIVE_PATH ${cmakeSrcFile} srcFile)

    # Now we symlink or copy
    IF (UNIX)
        ADD_CUSTOM_COMMAND(
            TARGET ${target} ${when}
            COMMAND ln
            ARGS -fs ${srcFile} ${destFile}
    ) 
    ELSE ()
        FILE(TO_NATIVE_PATH ${_destDir} winDir)
        ADD_CUSTOM_COMMAND(
            TARGET ${target} ${when}
            COMMAND xcopy
            ARGS /K /D /Q /R /Y ${srcFile} ${winDir} > NUL ) 
    ENDIF ()
ENDMACRO ()

######################################################################
# _YBT_SYMLINK_FILE_DURING_TARGET_AS -- A macro to schedule a single file for
# installation as a target is built, either before or after, with a new
# filename.
# ARGUMENTS:
#  target - The target after which files should be installed.
#  when - either PRE_BUILD or POST_BUILD
#  installPath - The path relative to CMAKE_INSTALL_PREFIX where files should
#                be installed.
#  fileToInstall - The path to the file to be installed.
#  installAs - The final filename the file should have.
#  Example:  To install CMakeLists.sdk.txt as CMakeLists.txt in examples/
#            as part of a post build step for the target "TargetName".
#  _YBT_SYMLINK_FILE_DURING_TARGET_AS(TargetName examples
#                                     CMakeLists.sdk.txt CMakeLists.txt)  
######################################################################
MACRO(_YBT_SYMLINK_FILE_DURING_TARGET_AS
      target when installPath fileToInstall installAs)
  # convert fileToInstall to an absolute path
  FILE(GLOB absPath ${fileToInstall})

  _YBT_SYMLINK_FILE_DURING_TARGET(
    ${target} ${when} ${absPath}
    ${CMAKE_INSTALL_PREFIX}/${installPath}/${installAs}
  )
ENDMACRO ()

######################################################################
# _YBT_SYMLINK_FILES_DURING_TARGET -- a macro to schedule files for
# installation immediately after a target is build.
# ARGUMENTS:
#  target - The target after which files should be installed.
#  when - either PRE_BUILD or POST_BUILD
#  installPath - The path relative to CMAKE_INSTALL_PREFIX where files should
#         be installed.
#  ARGN - Any number of files to install.
######################################################################
MACRO(_YBT_SYMLINK_FILES_DURING_TARGET target when installPath)
  FOREACH(file ${ARGN})
    # make the file path absolute (even if it is already)
    FILE(GLOB absPathToFile ${file})

    # generate the absolute file install path
    GET_FILENAME_COMPONENT(fileName ${file} NAME)

    _YBT_SYMLINK_FILE_DURING_TARGET(
      ${target} ${when} ${absPathToFile}
      ${CMAKE_INSTALL_PREFIX}/${installPath}/${fileName}
    )
  ENDFOREACH ()
ENDMACRO ()

######################################################################
# _YBT_SYMLINK_FILES_DURING_TARGET_P -- a macro to schedule files for
# installation immediately after a target is build.
# 
# differes from _YBT_SYMLINK_FILES_DURING_TARGET_P in that it preserves
# directory structure underneath a specified base directory, in the
# target directory.
#
# Example: 
#  _YBT_SYMLINK_FILES_DURING_TARGET_P(foo share/
#                                    /x/y/z
#                                    /x/y/z/a.cpp
#                                    /x/y/z/b.cpp
#                                    /x/y/z/f/c.h)
#  will install 3 files under dist/share/a.cpp
#                             dist/share/b.cpp
#                             dist/share/f/c.h
#
# ARGUMENTS:
#  target - The target after which files should be installed.
#  when - either PRE_BUILD or POST_BUILD
#  installPath - The path relative to CMAKE_INSTALL_PREFIX where files should
#         be installed.
#  BASEPATH - basepath will be removed from paths in ARGN, and all pathing
#             after basepath will be preserved
#  ARGN - Any number of files to install.
######################################################################
MACRO(_YBT_SYMLINK_FILES_DURING_TARGET_P target when installPath basePath)
  FOREACH(file ${ARGN})
    GET_FILENAME_COMPONENT(subdir ${file} PATH)
    FILE(RELATIVE_PATH subdir ${basePath} ${subdir})
    _YBT_SYMLINK_FILES_DURING_TARGET(${target} ${when}
                                         "${installPath}/${subdir}"
                                     ${file})
  ENDFOREACH ()
ENDMACRO ()

############################################################
# _YBT_LINK_LIBRARIES -- 
# Specify the libraries to which a target should be linked.
#
# ARGUMENTS:
#  TARGET -- the name of the target to link to the specified libraries
#  HOW -- SHARED or STATIC
#  ARGN -- list of libraries to link to 
############################################################
MACRO (_YBT_LINK_LIBRARIES TARGET HOW)
    FOREACH (lib ${ARGN})
    # regardless of the version (static/shared) we link, we must
    # build _after_ the shared version is build (when all the lib's
    # headers are installed)
      IF ("${HOW}" STREQUAL SHARED)
        TARGET_LINK_LIBRARIES(${TARGET} ${lib})
        GET_TARGET_PROPERTY(x ${TARGET} COMPILE_FLAGS)
        SET(newFlags "-D${lib}_SHARED")
        IF (x)
          set(newFlags "${newFlags} ${x}")
        ENDIF ()
        SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
                              COMPILE_FLAGS "${newFlags}")
      ELSE ()
        TARGET_LINK_LIBRARIES(${TARGET} ${lib}_s)
        IF (APPLE)
          SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
                                LINK_FLAGS "-framework CoreFoundation")
        ENDIF ()
      ENDIF ()
    ENDFOREACH ()
ENDMACRO ()

############################################################
# _YBT_REQUIRE_LIBRARIES -- 
# Specify a required library.  This is used for a library which depends on
# another, and the work performed is platform specific.  On Windows we
# link the source library to the export lib of the lib on which it depends,
# on unix we make them dependant to gaurantee that the dependee will build
# first.
#
# ARGUMENTS:
#  depender -- the name of the library which requires others
#  ARGN -- list of required libraries
############################################################
MACRO (_YBT_REQUIRE_LIBRARIES_SHARED depender)
  FOREACH (lib ${ARGN})
    IF (WIN32) 
      TARGET_LINK_LIBRARIES(${depender} ${lib})
      GET_TARGET_PROPERTY(x ${depender} COMPILE_FLAGS)
      SET(newFlags "-D${lib}_SHARED")
      IF (x)
        SET(newFlags "${newFlags} ${x}")
      ENDIF ()
      SET_TARGET_PROPERTIES(${depender} PROPERTIES
                            COMPILE_FLAGS "${newFlags}")
    ELSE ()
      ADD_DEPENDENCIES(${depender} ${lib})
    ENDIF ()
  ENDFOREACH ()
ENDMACRO ()

MACRO (_YBT_REQUIRE_LIBRARIES_STATIC depender)
  FOREACH (lib ${ARGN})
    IF (WIN32) 
      TARGET_LINK_LIBRARIES(${depender} ${lib}_s)
    ELSE ()
      ADD_DEPENDENCIES(${depender} ${lib}_s)
    ENDIF ()
  ENDFOREACH ()
ENDMACRO ()


############################################################
# _YBT_SYMLINK_OR_COPY -- 
# Symlink (or copy) a file, creating the required directories
#
# ARGUMENTS:
#  from -- create a symlink to this file
#  to   -- leaving the symlink here
############################################################
MACRO (_YBT_SYMLINK_OR_COPY from to)
  GET_FILENAME_COMPONENT(dir ${to} PATH)
  MAKE_DIRECTORY(${dir})

  # now translate paths to native paths 
  FILE(TO_NATIVE_PATH ${from} srcFile)
  FILE(TO_NATIVE_PATH ${to} destFile)

  IF (UNIX)
    EXECUTE_PROCESS(COMMAND ln -fs ${srcFile} ${destFile})
  ELSE ()
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${srcFile} ${destFile})
  ENDIF ()
ENDMACRO ()

############################################################
# _YBT_SYMLINK_FILES -- 
# Symlink (or copy) files into the distribution directory
#
# ARGUMENTS:
#  relativePath -- the path relative to the dist/ directory to copy
#                  into 
#  ARGN -- list of files to link into the dist directory.  the presence of the
#          magic word "PRESERVE" causes all files following that
#          word to have the path to the file relative to the
#          CMAKE_CURRENT_SOURCE_DIR preserved underneath
#          dist/${relativePath}
############################################################
MACRO (_YBT_SYMLINK_FILES relativePath)
  SET (instDir ${CMAKE_INSTALL_PREFIX}/${relativePath})
  SET (preserve)        
  FOREACH (file ${ARGN})
    IF (${file} STREQUAL "PRESERVE")    
      SET (preserve "nonnil")
    ELSE ()
      SET (copyTo)
      # if preserve is set, preserve the path
      _YBT_ABSOLUTIFY_PATHS(file ${file})
      IF (preserve)           
        FILE (RELATIVE_PATH to ${CMAKE_CURRENT_SOURCE_DIR} ${file})
        SET (copyTo ${instDir}/${to})
      ELSE ()
        GET_FILENAME_COMPONENT(to ${file} NAME)
        SET (copyTo ${instDir}/${to})
      ENDIF ()
      _YBT_SYMLINK_OR_COPY(${file} ${copyTo})
    ENDIF ()
  ENDFOREACH ()

ENDMACRO ()

############################################################
# We should define nice SOURCE groups, one per
# directory (sans "api and children") containing source files
############################################################
MACRO(_YBT_DEFINE_LIBRARY_SOURCE_GROUP NAME)
    SET(source)
    SET(publicHeaders)
    SET(privateHeaders)
    FOREACH (x ${ARGN})
        IF (${x} MATCHES ".*\\.h")
            IF (${x} MATCHES ".*api/.*")
                SET(publicHeaders ${x} ${publicHeaders})        
            ELSE ()
                SET(privateHeaders ${x} ${privateHeaders})        
            ENDIF ()
        ELSE ()
            SET(source ${x} ${source})        
        ENDIF ()
    ENDFOREACH ()
    SOURCE_GROUP("${NAME} public header files" FILES ${publicHeaders})
    SOURCE_GROUP("${NAME} private header files" FILES ${privateHeaders})
    SOURCE_GROUP("${NAME} source files" FILES ${source})
ENDMACRO ()

############################################################
# Given a list of files, automatically break them into logical
# "source groups" by parsing paths
# ARGUMENTS
#   ARGN - all the files that are put into the project
############################################################
MACRO(_YBT_DEFINE_SOURCE_GROUPS)
  SET(curGroup)
  SET(curFiles)
  FOREACH (file ${ARGN}) 
    # strip of filename
    GET_FILENAME_COMPONENT(path ${file} PATH)
    # strip off api and children
    STRING(REGEX REPLACE "/api.*" "" path "${path}")
    # strip off leading path
    STRING(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" path "${path}")
    # if the last line didn't match, then it's in the "Base" dir (this one)
    STRING(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "Base" path "${path}")

    IF ("${curGroup}" STREQUAL "")
      SET (curGroup ${path})
      SET (curFiles ${file})
    ELSE ()
      IF (NOT "${curGroup}" STREQUAL "${path}")
        _YBT_DEFINE_LIBRARY_SOURCE_GROUP(${curGroup} ${curFiles})
        SET (curGroup ${path})
        SET (curFiles ${file})
      ELSE ()
        SET (curFiles ${curFiles} ${file})
      ENDIF ()
    ENDIF ()
  ENDFOREACH ()

  IF (NOT "${curGroup}" STREQUAL "")
    _YBT_DEFINE_LIBRARY_SOURCE_GROUP(${curGroup} ${curFiles})
  ENDIF ()
ENDMACRO ()

############################################################
# Setup precompiled headers.
# ARGUMENTS
#   name - project name
############################################################
MACRO(_YBT_SETUP_PCH rootName name)
  # gather up all sources into the allSources var
  SET(allSources ${${rootName}_SOURCES} ${${rootName}_PRIVATE_HEADERS}
          ${${rootName}_PUBLIC_HEADERS} ${${rootName}_HEADERS})
  # look for stdafx.h (by default) or whatever is in YBT_CXX_PCH_H
  IF (NOT DEFINED YBT_CXX_PCH)
    FOREACH (file ${allSources})
      IF ("${file}" MATCHES "stdafx\\.h$" OR "${file}" MATCHES "StdAfx\\.h$")
        SET(YBT_CXX_PCH "${file}")
      ENDIF ()
    ENDFOREACH ()
  ENDIF ()
  IF (DEFINED YBT_CXX_PCH)
    IF (WIN32)
      # go through each file and make sure PCH is included
      SET(firstFile TRUE)
      FOREACH (file ${allSources})
        IF (NOT "${file}" MATCHES "\\.c$" AND
            NOT "${file}" MATCHES "\\.h$" AND
            NOT "${file}" MATCHES "\\.js$" AND
            NOT "${file}" MATCHES "\\.rc$" AND
            NOT "${file}" MATCHES "\\.idl$" AND
            NOT "${file}" MATCHES "\\.xpidl$")
          # Ensure every source file includes the PCH.
          SET(NEW_FLAGS "")
          GET_SOURCE_FILE_PROPERTY(OLD_FLAGS ${file} COMPILE_FLAGS)
          IF ("${OLD_FLAGS}" STREQUAL "NOTFOUND")
            SET(OLD_FLAGS "")
          ENDIF ()
          # force-include the PCH
          IF (NOT "${OLD_FLAGS}" MATCHES "/FI\"${YBT_CXX_PCH}\"")
            SET(NEW_FLAGS "${NEW_FLAGS} /FI\"${YBT_CXX_PCH}\"")
          ENDIF ()
  
          IF (firstFile)
            IF (NOT "${OLD_FLAGS}" MATCHES "/Yc\"${YBT_CXX_PCH}\"")
              SET(NEW_FLAGS "${NEW_FLAGS} /Yc\"${YBT_CXX_PCH}\"")
            ENDIF ()
          ELSE ()
            IF (NOT "${OLD_FLAGS}" MATCHES "/Yu\"${YBT_CXX_PCH}\"")
              SET(NEW_FLAGS "${NEW_FLAGS} /Yu\"${YBT_CXX_PCH}\"")
            ENDIF ()
          ENDIF ()
          SET (firstFile FALSE) 
  
          # Now set both flags.
          SET_SOURCE_FILES_PROPERTIES(${file} PROPERTIES COMPILE_FLAGS
                                      "${OLD_FLAGS} ${NEW_FLAGS}")
        ENDIF ()
      ENDFOREACH ()
    ELSE ()
      # Precompiled headers are supported in gcc 3.4 and higher
      IF (CMAKE_CXX_COMPILER_VERSION MATCHES ".*3\\.[4-9].*" OR
          CMAKE_CXX_COMPILER_VERSION MATCHES ".*4\\.[0-9].*")
        # We need the list of inc dirs to compile propertly.
        SET(pchIncludeDirectories)
        GET_DIRECTORY_PROPERTY(incDirs INCLUDE_DIRECTORIES)
        FOREACH(incDir ${incDirs})
          SET(pchIncludeDirectories "${pchIncludeDirectories} -I${incDir}")
        ENDFOREACH ()
        SET(pchCompileFlags "-c -x c++ -g ${pchIncludeDirectories} -o ${YBT_CXX_PCH}.gch ${YBT_CXX_PCH}")
        # NEEDSWORK!!! Make this work on unix too.
        #SET_SOURCE_FILES_PROPERTIES(${YBT_CXX_PCH} PROPERTIES COMPILE_FLAGS ${pchCompileFlags})
        #SET_SOURCE_FILES_PROPERTIES(${YBT_CXX_PCH} PROPERTIES HEADER_FILE_ONLY FALSE)
        #This should work but doesn't
        #Either use these two lines or subsequent two lines
        #ADD_CUSTOM_TARGET(${name}_GCH_FILE "${CMAKE_C_COMPILER} ${pchCompileFlags}" DEPENDS ${YBT_CXX_PCH})
        #ADD_DEPENDENCIES(${name} ${name}_GCH_FILE)
        #This is modified from python src
        #Either use these two lines or preceding two lines
        #ADD_CUSTOM_TARGET(${name}_GCH_FILE ALL DEPENDS ${YBT_CXX_PCH}.gch)
        #ADD_CUSTOM_COMMAND(
        #  OUTPUT ${YBT_CXX_PCH}.gch
        #  COMMAND "${CMAKE_C_COMPILER} ${pchCompileFlags}"
        #  DEPENDS ${YBT_CXX_PCH}
        #)
      ENDIF ()
    ENDIF ()
  ENDIF ()
ENDMACRO ()
