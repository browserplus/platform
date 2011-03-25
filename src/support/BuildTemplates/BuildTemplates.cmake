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
#  Copyright (c) Yahoo!, Inc 2006-2009
#
#  This file is the main include for using "BuildTemplates" 
#  By including this file, you gain access to all of the build
#  macros provided by these templates.
#
#  See the "USAGE" file for tutorial style documentation, or read
#  the comments before Macros in PublicMacros.cmake to determine what
#  macros are available for use.
#
#  To correctly use the templates, add the following to your top
#  level cmake file
#  
#  PROJECT(MyProjectName)
#  INCLUDE("path/to/BuildTemplates/BuildTemplates.cmake")
#
#  Note about cmake output:
#    the first two chars of output denote the type of message:
#    -- is a message from cmake
#    ++ is a informational message  
#    ** is a verbose message (enable by defining YBT_VERBOSE,
#                             like this: "cmake -DYBT_VERBOSE=1")
#    DD is a debug message (enable by defining YBT_DEBUG,
#                           like this: "cmake -DYBT_DEBUG=1")
#       __debug implies verbose__
#    !! is a fatal error message.  Your cmake run terminates with this type
#       of message.  The return code of cmake is non-zero in this case.
#
#  Note about out of source builds:
#   USE THEM.  we fully support and advocate out of source builds.  that
#   means the directory into which all intermediate generated files are
#   placed is different than your source directory.  This makes it trivial
#   to determine what's source, and what's build output.  
#
#   $ cd source_dir
#   $ mkdir work
#   $ cd work
#   $ cmake ..
#   $ gmake (or devenv, or xcode, or whatever)
#
#   to dist_clean?
#   $ cd .. && rm -rf work
#
#   simple and good!
############################################################

# require cmake 2.4 or higher
# CMAKE_MINIMUM_REQUIRED(VERSION 2.4 FATAL_ERROR)
# client to require (!)

# reduce redundancy in the cmake language
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

##### It is not required that you understand the CMake code beneath
##### this point to use the templates.  

##### See the "USAGE" file for tutorial style documentation, or read
##### the comments before Macros in PublicMacros.cmake to determine what
##### macros are available for use.

# set YBT_PLATFORM_SEPARATOR (the separator string used to append platform
# names to files for implicit source discovery) if it's not set 
#
# Note: this means by default files with embedded _'s in their names may be
# interpreted as platform-specific and filtered out of your project - if you 
# don't want this behavior you should modify YBT_PLATFORM_SEPARATOR.
IF(NOT DEFINED YBT_PLATFORM_SEPARATOR)
  SET(YBT_PLATFORM_SEPARATOR "_")
ENDIF ()

# manage verbose/debug flags
IF(DEFINED YBT_DEBUG)
  MESSAGE("DD debug and verbose output enabled")
  SET(YBT_VERBOSE 1)
ELSE ()
  IF(NOT DEFINED YBT_VERBOSE)
    IF(NOT DEFINED YBT_SECOND_RUN) 
      MESSAGE("-- want verbose output?  cmake -DYBT_VERBOSE=1 <path>")
    ENDIF ()
  ELSE ()
    MESSAGE("** VERBOSE output enabled, (messages have ** prepended)")
    IF(NOT DEFINED YBT_SECOND_RUN) 
      MESSAGE("** want debug output?  cmake -DYBT_DEBUG=1 <path>")
    ENDIF ()
  ENDIF ()
ENDIF ()

############################################################
# populate YBT_PATH is set and is a directory
############################################################
GET_FILENAME_COMPONENT(YBT_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

IF(DEFINED YBT_VERBOSE)
  MESSAGE("** Path to BuildTemplates: ${YBT_PATH}")
ENDIF ()

############################################################
# Set the build type
############################################################
IF (NOT CMAKE_BUILD_TYPE)
  IF(NOT WIN32 OR ${CMAKE_MAKE_PROGRAM} STREQUAL "nmake") 
    IF(NOT DEFINED YBT_SECOND_RUN) 
      MESSAGE("++ Build type not set, defaulting to Debug")
      MESSAGE("++ Want a Release (stripped and optimized) build?  try: ")
      MESSAGE("++ cmake -DCMAKE_BUILD_TYPE:STRING=Release .") 
    ENDIF ()
  ENDIF ()
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF ()

# only give the helpful hints on the first go around
SET (YBT_SECOND_RUN "1" CACHE INTERNAL "An anti-verbosity flag")

############################################################
# Set default C++ compiler parameters
# TODO: We must figure out how to generalize this to allow the
#       client to control it in a more convenient way.
############################################################
IF(WIN32)
    ADD_DEFINITIONS(-DWIN32)
    SET(CXX_BUILD_FLAGS
        " /D _CRT_SECURE_NO_DEPRECATE /Zm1000 /EHa /wd4025 /wd4100 /wd4127 /wd4251 /wd4256 /wd4335 /wd4996 /Z7 /MT")
    SET(CXX_DEBUG_BUILD_FLAGS "/Od -DDEBUG -D_DEBUG")
    SET(CXX_RELEASE_BUILD_FLAGS "/O2")
    SET(LINK_FLAGS "/INCREMENTAL:NO /OPT:REF /OPT:ICF /DEBUG /PDB:NONE")
    SET(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")
    SET(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")
    SET(CMAKE_MODULE_LINKER_FLAGS "${LINK_FLAGS}")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LINK_FLAGS}")
    SET(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${LINK_FLAGS}")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${LINK_FLAGS}")
    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${LINK_FLAGS}")
    SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${LINK_FLAGS} /MTd")
    SET(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "${LINK_FLAGS}")
    SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${LINK_FLAGS}")
    SET(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${LINK_FLAGS}")
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${LINK_FLAGS}")
    SET(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${LINK_FLAGS}")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${LINK_FLAGS}")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${LINK_FLAGS}")
    IF (MSVC71)
      SET(CMAKE_CXX_COMPILER_VERSION "MSVC71")
    ELSEIF (MSVC80)
      SET(CMAKE_CXX_COMPILER_VERSION "MSVC80")
    ELSEIF (MSVC90)
      SET(CMAKE_CXX_COMPILER_VERSION "MSVC90")
    ELSEIF (MSVC10)
      SET(CMAKE_CXX_COMPILER_VERSION "MSVC10")
    ELSE ()
      MESSAGE(FATAL_ERROR "Using unsupported Windows compiler")
    ENDIF ()
    MESSAGE("Using cl ${CMAKE_CXX_COMPILER_VERSION}")
ELSE ()
    EXEC_PROGRAM("${CMAKE_C_COMPILER} ARGS --version |head -1 |awk '{print $1}'"
                 OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION)
    EXEC_PROGRAM("${CMAKE_CXX_COMPILER} ARGS --version |head -1 |awk '{print $1}'"
                 OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION)
    MESSAGE("Using C compiler ${CMAKE_C_COMPILER}, version ${CMAKE_C_COMPILER_VERSION}")
    MESSAGE("Using C++ compiler ${CMAKE_CXX_COMPILER}, version ${CMAKE_CXX_COMPILER_VERSION}")

    SET(CXX_BUILD_FLAGS "-Wall -fPIC")
    IF (APPLE)
        SET(CMAKE_EXE_LINKER_FLAGS "-bind_at_load")
        SET(CMAKE_SHARED_LINKER_FLAGS "-single_module")
    ENDIF ()
    IF (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-E")
        SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-E")
    ENDIF () 

    SET(CXX_DEBUG_BUILD_FLAGS "-DDEBUG -g")

    # GCC 3.3 will cause the following kinds of errors when compiled
    # with -O3 or higher on mac:
    # ld: Undefined symbols:
    # virtual thunk [v:0,-16] to YCPEventListener::~YCPEventListener [in-charge deleting]()
    # virtual thunk [v:0,-16] to YCPEventListener::~YCPEventListener [in-charge]()
    # virtual thunk [v:0,-16] to YCPCommandLineParser::~YCPCommandLineParser [in-charge deleting]()
    # virtual thunk [v:0,-16] to YCPCommandLineParser::~YCPCommandLineParser [in-charge]()
    #
    # we use -O2 on all platforms along with NDEBUG to strip out
    # assertions... (among other things?)
    SET(CXX_RELEASE_BUILD_FLAGS "-DNDEBUG -O2")

    # (LtH) don't warn about unused params.  Some third party headers
    # we use (cppunit) have named unused parameters for documenation
    # purposes.  Some macros we use make defining callbacks easier.
    # But in callbacks, sometimes there's legitimate reasons to ignore
    # parameters, but still want the convenience of a macro.
    # All this hopefully justifies the addition of -Wno-unused-parameter...
    #
    #  #pragma like behavior for temporary disabling of these warnings
    #          would be preferred
    #
    # On gcc versions less that 3.x we don't want to do this cause the
    # compiler doesn't warn, and doesn't understand the flag.
    IF (CMAKE_CXX_COMPILER_VERSION MATCHES ".*-g\\+\\+3\\.[4-9].*")
        SET(CXX_BUILD_FLAGS "${CXX_BUILD_FLAGS} -Wno-unused-parameter")
        IF(DEFINED YBT_VERBOSE)
          MESSAGE("** NOTE: disabling \"unused parameter\" compiler warnings!")
    ENDIF ()
    ENDIF ()

    # gcc versions less that 3.x will not tolerate a directory in the link
    # path with the name of "string" (or vector, or anything else).  For
    # that reason we insure the path to the include directory is first in our
    # include path

    IF (CMAKE_CXX_COMPILER_VERSION MATCHES ".*-g\\+\\+-3\\.[4-9].*")
        SET(CXX_BUILD_FLAGS "${CXX_BUILD_FLAGS} -ftemplate-depth-100")
        FIND_PATH(GXX_STRING_INCLUDE_PATH string /usr/include/g++)
        IF (NOT GXX_STRING_INCLUDE_PATH)
            MESSAGE(FATAL_ERROR
                    "!! ERROR: could not find 'string' in '/usr/include/g++'")
        ENDIF ()
        INCLUDE_DIRECTORIES(BEFORE ${GXX_STRING_INCLUDE_PATH})
    ENDIF ()

    IF (APPLE)
        SET(CXX_BUILD_FLAGS "${CXX_BUILD_FLAGS} -Wno-long-double")
    ENDIF ()
ENDIF ()

IF (CXX_RELEASE_BUILD_FLAGS)
    SET(CMAKE_CXX_FLAGS_RELEASE ${CXX_RELEASE_BUILD_FLAGS}
        CACHE STRING "Release Compiler options" FORCE)
ENDIF ()
SET(CMAKE_CXX_FLAGS ${CXX_BUILD_FLAGS} CACHE STRING "Compiler options"
    FORCE)
SET(CMAKE_CXX_FLAGS_DEBUG ${CXX_DEBUG_BUILD_FLAGS}
    CACHE STRING "Debug Compiler options" FORCE)
SET(CMAKE_CXX_WARNING_LEVEL "4" CACHE STRING "Warning level" FORCE)
SET(CMAKE_CXX_USE_RTTI YES CACHE BOOL "RTTI" FORCE)
SET(CMAKE_CXX_WIN9X_UNICODE YES CACHE BOOL "WIN9X_UNICODE" FORCE)

IF(DEFINED YBT_VERBOSE)
  MESSAGE("** Compiler options: ${CMAKE_CXX_FLAGS}")
  MESSAGE("** Debug compiler options: ${CMAKE_CXX_FLAGS_DEBUG}")
ENDIF ()

######################################################################
# Set INSTALL_PREFIX for all sub directories.
# XXX: yeesh.  how's this gonna work?
######################################################################
SET(CMAKE_INSTALL_PREFIX
    ${CMAKE_CURRENT_BINARY_DIR}/dist
    CACHE PATH "Where to install build results (the \"SDK\")" FORCE)

IF(DEFINED YBT_VERBOSE)
  MESSAGE("** install to: \"${CMAKE_INSTALL_PREFIX}\"")
ENDIF ()

######################################################################
# load up some macros for our client
######################################################################
INCLUDE("${YBT_PATH}/PublicMacros.cmake")
