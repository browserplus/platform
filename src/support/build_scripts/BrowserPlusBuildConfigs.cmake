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

SET (CMAKE_CONFIGURATION_TYPES Debug Release CodeCoverage
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
    SET(CMAKE_CXX_FLAGS_CODECOVERAGE "${CMAKE_CXX_FLAGS_DEBUG}"
        CACHE STRING "BrowserPlus codecoverage CXX flags" FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE "/MT /DNDEBUG /O2"
        CACHE STRING "BrowserPlus release CXX flags" FORCE)
  
    # libs to ignore, from http://msdn.microsoft.com/en-us/library/aa267384.aspx
    #
    SET(noDefaultLibFlagsDebug "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:msvcrtd.lib")
    SET(noDefaultLibFlagsCodeCoverage "${noDefaultLibFlagsDebug}")
    SET(noDefaultLibFlagsRelease "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")

    SET(linkFlags "/DEBUG /MANIFEST:NO")
    SET(linkFlagsDebug " ${noDefaultLibFlagsDebug}")
    SET(linkFlagsCodeCoverage " ${linkFlagsDebug}")
    SET(linkFlagsRelease " /INCREMENTAL:NO /OPT:REF /OPT:ICF ${noDefaultLibFlagsRelease}")
  
    SET(CMAKE_EXE_LINKER_FLAGS "${linkFlags}"
        CACHE STRING "BrowserPlus linker flags" FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${linkFlagsDebug}"
        CACHE STRING "BrowserPlus debug linker flags" FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS_CODECOVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
        CACHE STRING "BrowserPlus codecoverage linker flags" FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${linkFlagsRelease}"
        CACHE STRING "BrowserPlus release linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS "${linkFlags}"
        CACHE STRING "BrowserPlus shared linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${linkFlagsDebug}"
        CACHE STRING "BrowserPlus shared debug linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS_CODECOVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
        CACHE STRING "BrowserPlus shared codecoverage linker flags" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${linkFlagsRelease}"
        CACHE STRING "BrowserPlus shared release linker flags" FORCE)

    SET(CMAKE_MODULE_LINKER_FLAGS "${linkFlags}"
        CACHE STRING "BrowserPlus module linker flags" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${linkFlagsDebug}"
        CACHE STRING "BrowserPlus module debug linker flags" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS_CODECOVERAGE "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}"
        CACHE STRING "BrowserPlus module codecoverage linker flags" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${linkFlagsRelease}"
        CACHE STRING "BrowserPlus module release linker flags" FORCE)
ELSE ()
    # Must tell cmake that we really, really, really want certain
    # compiler/sdk
    SET(isysrootFlag)
    SET(minVersionFlag)
    INCLUDE(CMakeForceCompiler)
    IF (APPLE)
      SET(CMAKE_OSX_ARCHITECTURES i386)
      # XXX when 10.4 dropped, remove 10.4 clause
      IF (OSX10.4_BUILD) 
        SET(CMAKE_C_COMPILER gcc-4.0)
        SET(CMAKE_CXX_COMPILER g++-4.0)
        IF ("${CMAKE_BUILD_TYPE}" STREQUAL "CodeCoverage") 
          MESSAGE(FATAL_ERROR "OSX CodeCoverage build not supported on 10.4 due to XCode shipping incorrect libgcov.a binary")
        ELSE ()
          SET (CMAKE_OSX_DEPLOYMENT_TARGET "10.4"
               CACHE STRING "Compile for tiger deployment" FORCE)
          SET(minVersionFlag "-mmacosx-version-min=10.4")
        ENDIF ()
        SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk"
            CACHE STRING "Compile for tiger deployment" FORCE)
        SET(isysrootFlag "-isysroot ${CMAKE_OSX_SYSROOT}")
        SET(CMAKE_FRAMEWORK_PATH "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks"
            CACHE STRING "use 10.4 frameworks" FORCE)
        SET(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "4.0"
            CACHE STRING "BrowserPlus debug CXX flags" FORCE)
      ELSE ()
        # Use full paths since 10.5 doesn't have llvm in /usr/bin.
        # Even with all of this, 10.5 xcode generator doesn't honor this.
        # XXX llvm doesn't support code coverage!
        #SET(CMAKE_C_COMPILER /Developer/usr/bin/llvm-gcc-4.2)
        #SET(CMAKE_CXX_COMPILER /Developer/usr/bin/llvm-g++-4.2)
        SET(CMAKE_C_COMPILER gcc-4.2)
        SET(CMAKE_CXX_COMPILER g++-4.2)
        IF ("${CMAKE_BUILD_TYPE}" STREQUAL "CodeCoverage") 
          SET (CMAKE_OSX_DEPLOYMENT_TARGET "10.6"
               CACHE STRING "Compile for snow leopard deployment" FORCE)
          SET(minVersionFlag "-mmacosx-version-min=10.6")
        ELSE ()
          SET (CMAKE_OSX_DEPLOYMENT_TARGET "10.5"
               CACHE STRING "Compile for leopard deployment" FORCE)
          SET(minVersionFlag "-mmacosx-version-min=10.5")
        ENDIF ()
        SET(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "4.2"
            CACHE STRING "BrowserPlus debug CXX flags" FORCE)
      ENDIF ()

      # now tell cmake to tell xcode that we really, really, really want i386
      SET(CMAKE_XCODE_ATTRIBUTE_ARCHS i386)

      SET(ENV{CC} ${CMAKE_C_COMPILER})
      SET(ENV{CXX} ${CMAKE_CXX_COMPILER})
      CMAKE_FORCE_C_COMPILER(${CMAKE_C_COMPILER} GNU)
      CMAKE_FORCE_CXX_COMPILER(${CMAKE_CXX_COMPILER} GNU)

      SET(CMAKE_MODULE_LINKER_FLAGS "${minVersionFlag} ${isysrootFlag}")
      SET(CMAKE_EXE_LINKER_FLAGS "-dead_strip -dead_strip_dylibs ${minVersionFlag} ${isysrootFlag}")
      SET(CMAKE_SHARED_LINKER_FLAGS "${minVersionFlag} ${isysrootFlag}  -Wl,-single_module")
      ADD_DEFINITIONS(-DMACOSX -D_MACOSX -DMAC -D_MAC -DXP_MACOSX)
    ELSE()
      ADD_DEFINITIONS(-DLINUX -D_LINUX -DXP_LINUX)
      set(FPICFlag "-fPIC")
    ENDIF()

    SET(CMAKE_CXX_FLAGS "-Wall ${isysrootFlag} ${minVersionFlag} ${FPICFlag}")
    SET(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
    SET(CMAKE_CXX_FLAGS_CODECOVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os")
    SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "-Wl,-x")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-x")
    SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,-x")
ENDIF ()

# define this for a platform build
#
ADD_DEFINITIONS(-DBP_PLATFORM_BUILD)

# Stuff from global Externals is used throughout our tree. 
#
GET_FILENAME_COMPONENT( pathToThisFile "${CMAKE_CURRENT_LIST_FILE}" PATH )
SET( BP_EXTERNAL "${pathToThisFile}/../../../external/dist" )
INCLUDE_DIRECTORIES( "${BP_EXTERNAL}/include" )

# We must link with correct Debug or Release libs, which requires that we 
# know if we are using a single or multiple config generator.  For a
# single generator (like Makefile), we must append the build type.  
# For the Xcode and VS multiple generators, they append the build type for us.

# NOTE: trying to test CMAKE_CONFIGURATION_TYPES doesn't work, 
#       it returns "Debug;Release", even for a single config generator

IF (CMAKE_CFG_INTDIR STREQUAL "." OR CMAKE_CFG_INTDIR STREQUAL "/")
   IF ("${CMAKE_BUILD_TYPE}" STREQUAL "CodeCoverage")
     # using a single configuration generator
     LINK_DIRECTORIES("${BP_EXTERNAL}/lib/Debug")
   ELSE ()
     # using a single configuration generator
     LINK_DIRECTORIES("${BP_EXTERNAL}/lib/${CMAKE_BUILD_TYPE}")
   ENDIF ()
ELSE ()
   # using a multiple configuration generator
   # vs and xcode append a build type to this path
   LINK_DIRECTORIES("${BP_EXTERNAL}/lib")
ENDIF ()
