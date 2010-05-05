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
#
# Special build configuration used throughout browserplus.
#

# require cmake 2.8.1 or higher
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1 FATAL_ERROR)

IF (POLICY CMP0011) 
  cmake_policy(SET CMP0011 OLD)
ENDIF (POLICY CMP0011) 
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0009 NEW)

SET (CMAKE_CONFIGURATION_TYPES Debug Release 
     CACHE STRING "BrowserPlus build configs" FORCE)

# reduce redundancy in the cmake language
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF ()

# now set up the build configurations
IF(WIN32)
    SET(win32Defs "/DWINDOWS /D_WINDOWS /DWIN32 /D_WIN32 /DXP_WIN32 /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DNOSOUND /DNOCOMM /DNOMCX /DNOSERVICE /DNOIME /DNORPC /D_CRT_RAND_S")
    SET(disabledWarnings "/wd4100 /wd4127 /wd4201 /wd4250 /wd4251 /wd4275 /wd4800 /wd4297")
    SET(CMAKE_CXX_FLAGS
        "${win32Defs} /EHsc /Gy /MT /W4 ${disabledWarnings} /Zi"
        CACHE STRING "BrowserPlus CXX flags" FORCE)
    SET(CMAKE_CXX_FLAGS_DEBUG "/MTd /DDEBUG /D_DEBUG /Od /RTC1 /RTCc"
        CACHE STRING "BrowserPlus debug CXX flags" FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE "/MT /DNDEBUG /O1"
        CACHE STRING "BrowserPlus release CXX flags" FORCE)
  
    # libs to ignore, from http://msdn.microsoft.com/en-us/library/aa267384.aspx
    #
    SET(noDefaultLibFlagsDebug "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:msvcrtd.lib")
    SET(noDefaultLibFlagsRelease "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")

    SET(linkFlags "/DEBUG /MANIFEST:NO")
    SET(linkFlagsDebug " ${noDefaultLibFlagsDebug}")
    SET(linkFlagsRelease " /INCREMENTAL:NO /OPT:REF /OPT:ICF ${noDefaultLibFlagsRelease}")
  
    SET(CMAKE_EXE_LINKER_FLAGS "${linkFlags}"
        CACHE STRING "BrowserPlus linker flags" FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${linkFlagsDebug}"
        CACHE STRING "BrowserPlus debug linker flags" FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${linkFlagsRelease}"
        CACHE STRING "BrowserPlus release linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS "${linkFlags}"
        CACHE STRING "BrowserPlus shared linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${linkFlagsDebug}"
        CACHE STRING "BrowserPlus shared debug linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${linkFlagsRelease}"
        CACHE STRING "BrowserPlus shared release linker flags" FORCE)

    SET(CMAKE_MODULE_LINKER_FLAGS "${linkFlags}"
        CACHE STRING "BrowserPlus module linker flags" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${linkFlagsDebug}"
        CACHE STRING "BrowserPlus module debug linker flags" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${linkFlagsRelease}"
        CACHE STRING "BrowserPlus module release linker flags" FORCE)
ELSE ()
    SET(isysrootFlag)
    IF (APPLE)
      # Must tell cmake that we really, really, really want gcc-4.0
      INCLUDE(CMakeForceCompiler)
      CMAKE_FORCE_C_COMPILER(gcc-4.0 GNU)
      CMAKE_FORCE_CXX_COMPILER(gcc-4.0 GNU)

      # now tell cmake to tell xcode that we really, really, really,
      # want gcc-4.0 and i386
      SET( CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "4.0"
           CACHE STRING "BrowserPlus debug CXX flags" FORCE )
      SET(CMAKE_XCODE_ATTRIBUTE_ARCHS i386)

      # and we want 32bit i386 for osx 10.4
      SET(CMAKE_OSX_ARCHITECTURES i386)
      Set (CMAKE_OSX_DEPLOYMENT_TARGET "10.4"
	       CACHE STRING "Compile for tiger deployment" FORCE)
      SET (CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk"
	       CACHE STRING "Compile for tiger backwards compat" FORCE)
      SET(isysrootFlag "-isysroot ${CMAKE_OSX_SYSROOT}")
      SET(minVersionFlag "-mmacosx-version-min=10.4")
      SET(CMAKE_FRAMEWORK_PATH "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks"
	      CACHE STRING "use 10.4 frameworks" FORCE)

      SET(CMAKE_MODULE_LINKER_FLAGS "${minVersionFlag} ${isysrootFlag}")
      SET(CMAKE_EXE_LINKER_FLAGS "-dead_strip -dead_strip_dylibs ${minVersionFlag} ${isysrootFlag}")
      SET(CMAKE_SHARED_LINKER_FLAGS "${minVersionFlag} ${isysrootFlag}  -Wl,-single_module")
      ADD_DEFINITIONS(-DMACOSX -D_MACOSX -DMAC -D_MAC -DXP_MACOSX)
      SET(CMAKE_C_COMPILER gcc-4.0)
      SET(CMAKE_CXX_COMPILER g++-4.0)
    ELSE()
      ADD_DEFINITIONS(-DLINUX -D_LINUX -DXP_LINUX)
      set(FPICFlag "-fPIC")
    ENDIF()

    SET(CMAKE_CXX_FLAGS "-Wall ${isysrootFlag} ${minVersionFlag} ${FPICFlag}")
    SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
    #SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os")
    SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "-Wl,-x")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-x")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,-x")
ENDIF ()

# define this for a platform build
#
ADD_DEFINITIONS(-DBP_PLATFORM_BUILD)

# can't rely on CMAKE_SYSTEM_NAME.  It's set by calling PROJECT() or some
# such.  So we define system name ourselves based on other, more reliable
# cmake vars. (lth 7/27/07)
IF (WIN32) 
  SET (systemName "Windows")
ELSEIF (APPLE)  
  SET (systemName "Darwin")
ELSE ()
  SET (systemName "Linux")
ENDIF ()

# Stuff from global Externals is used throughout our tree. 
#
GET_FILENAME_COMPONENT( pathToThisFile "${CMAKE_CURRENT_LIST_FILE}" PATH )
SET( BP_EXTERNAL "${pathToThisFile}/../../../external/${systemName}" )
INCLUDE_DIRECTORIES( "${BP_EXTERNAL}/include" )

# We must link with correct Debug or Release libs, which requires that we 
# know if we are using a single or multiple config generator.  For a
# single generator (like Makefile), we must append the build type.  
# For the Xcode and VS multiple generators, they append the build type for us.

# NOTE: trying to test CMAKE_CONFIGURATION_TYPES doesn't work, 
#       it returns "Debug;Release", even for a single config generator

IF (CMAKE_CFG_INTDIR STREQUAL "." OR CMAKE_CFG_INTDIR STREQUAL "/")
   # using a single configuration generator
   LINK_DIRECTORIES("${BP_EXTERNAL}/lib/${CMAKE_BUILD_TYPE}")
ELSE ()
   # using a multiple configuration generator
   # vs and xcode append a build type to this path
   LINK_DIRECTORIES("${BP_EXTERNAL}/lib")
ENDIF ()
