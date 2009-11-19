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
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)
SET(YBT_NESTED_INCLUDES 1)

# set include paths to build configs cmake and build templates cmake
SET(YBP_BuildTemplates
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/BuildTemplates/BuildTemplates.cmake"
)

SET(YBP_BuildConfigs
"${CMAKE_CURRENT_SOURCE_DIR}/support/build_scripts/BrowserPlusBuildConfigs.cmake"
)

INCLUDE(${YBP_BuildConfigs})
PROJECT(BrowserPlus)
INCLUDE(${YBP_BuildTemplates})
INCLUDE(${YBP_BuildConfigs})

# include platform version defines
INCLUDE(PlatformVersion.cmake)

# what signing certificates do we use?
# To make a production build:
#   "cmake -DBP_BUILD_TYPE:STRING=production -DCMAKE_BUILD_TYPE:STRING=Release .."
IF (NOT BP_BUILD_TYPE)
  SET(BP_BUILD_TYPE "devel")
ENDIF ()

# write the current build type to a file in current binary dir
SET(buildTypeFile "${CMAKE_CURRENT_BINARY_DIR}/build_type.txt")
FILE(WRITE ${buildTypeFile} ${BP_BUILD_TYPE})

SET(develDistroURL http://bp-dev.corp.yahoo.com)
SET(internalDistroURL http://bp-int.corp.yahoo.com)
SET(prodDistroURL http://browserplus.yahoo.com)

# Setup blacklist, which will be substituted into
# BrowserPlus.config and permissions.json files.
SET (BP_SERVICE_BLACKLIST_LIST)
SET (BP_PLATFORM_BLACKLIST_LIST)

# at the time we shipped 2.4.15 we blacklisted all platforms that we've
# shipped up to 2.4.8
LIST (APPEND BP_PLATFORM_BLACKLIST_LIST
      "\"2.0.3\""
      "\"2.0.4\""
      "\"2.0.6\""
      "\"2.1.6\""
      "\"2.1.7\""
      "\"2.1.11\""
      "\"2.1.14\""
      "\"2.1.19\""
      "\"2.2.0\""
      "\"2.3.1\""
      "\"2.4.6\""
     )

SET (BP_PRIMARY_DISTRO_SERVER_URL)
SET (BP_SECONDARY_DISTRO_SERVER_URL)
SET (BP_TERTIARY_DISTRO_SERVER_URL)

IF (NOT BP_BUILD_TYPE OR NOT ${BP_BUILD_TYPE} STREQUAL "production")
    # internal and devel builds use dev certs
    SET (BP_CERTIFICATE_TYPE devel)

    IF (BP_BUILD_TYPE STREQUAL "internal")
        SET (BP_PRIMARY_DISTRO_SERVER_URL ${internalDistroURL})
        SET (BP_SECONDARY_DISTRO_SERVER_URL ${prodDistroURL})
    ELSE ()
        SET (BP_PRIMARY_DISTRO_SERVER_URL ${develDistroURL})
        SET (BP_SECONDARY_DISTRO_SERVER_URL ${internalDistroURL})
        SET (BP_TERTIARY_DISTRO_SERVER_URL ${prodDistroURL})
    ENDIF ()

    SET (BP_CDN_URL "${BP_PRIMARY_DISTRO_SERVER_URL}/dist/")

    LIST (APPEND BP_SERVICE_BLACKLIST_LIST 
                 "[ \"iTunesLibrary\", \"2.0.3\"]"
                 "[ \"EvilCorelet\", \"1\"]"
                 "[ \"RubyInterpreter\", \"3\"]"
                 "[ \"RubyInterpreter\", \"4.0\"]"
                 "[ \"IRCClient\", \"1\"]")
ELSE ()
    SET (BP_CDN_URL "http://bp.yahooapis.com/")
    SET (BP_CERTIFICATE_TYPE prod)
    SET (BP_PRIMARY_DISTRO_SERVER_URL ${prodDistroURL})
    LIST (APPEND BP_SERVICE_BLACKLIST_LIST 

    # Ruby 1.8.6 has known flaws.  4.1.X version upgrades
    # to 1.8.7 patchlevel 22 (YIB-2071444)
    "[ \"RubyInterpreter\", \"3\"]"
    "[ \"RubyInterpreter\", \"4.0\"]"

    # ircclient uses lgpl ruby lib
    "[ \"IRCClient\", \"1\"]")
ENDIF ()

MESSAGE("!! Build configuration (BP_BUILD_TYPE) is '${BP_BUILD_TYPE}' shipping ${BP_CERTIFICATE_TYPE} certs")
MESSAGE("!! Primary distribution server: ${BP_PRIMARY_DISTRO_SERVER_URL}")
MESSAGE("!! Secondary distribution server: ${BP_SECONDARY_DISTRO_SERVER_URL}")
MESSAGE("!! Tertiary distribution server: ${BP_TERTIARY_DISTRO_SERVER_URL}")
MESSAGE("!! Static Resource URL: ${BP_STATIC_RESOURCES_URL}")

IF (APPLE)
  MESSAGE("!! CMAKE_OSX_SYSROOT == ${CMAKE_OSX_SYSROOT}")
ENDIF ()

# Turn cmake list into json list
MACRO(LIST_TO_JSON_LIST inArg outArg)
    FOREACH(e ${${inArg}})
        IF (NOT ${outArg})
            SET(${outArg} "${e}")
        ELSE ()
            SET(${outArg} "${${outArg}}, ${e}")
        ENDIF ()
    ENDFOREACH()
    MESSAGE("!! ${outArg}: ${${outArg}}")
ENDMACRO(LIST_TO_JSON_LIST)

LIST_TO_JSON_LIST(BP_SERVICE_BLACKLIST_LIST BP_SERVICE_BLACKLIST)
LIST_TO_JSON_LIST(BP_PLATFORM_BLACKLIST_LIST BP_PLATFORM_BLACKLIST)

# Now run .erb and .cmakeIn substitution.  This must occur
# before the ADD_SUBDIRECTORY() commands below so that the
# CMakeLists.txt files in those dirs will see the generated files.
INCLUDE(support/build_scripts/BPSubstitution.cmake)

# include a script that will define the BPAddTest macro, which
# makes it easy to include tests as part of the build
INCLUDE(support/build_scripts/BPAddTest.cmake)

# include a script that will define the BPRandDefn macro, which
# defines a random pre-processor symbol for the current working
# directory
INCLUDE(support/build_scripts/BPRandDefn.cmake)

# everyone under here will build with access to the ServiceAPI headers
INCLUDE_DIRECTORIES(sdk/service_api/api)

# If you add an entry here, you probably also need to add it to 
# "allShippingDepends" in BuildSDK.cmake
ADD_SUBDIRECTORY(libs)
ADD_SUBDIRECTORY(support)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(bins)
ADD_SUBDIRECTORY(sdk)
ADD_SUBDIRECTORY(javascript)

# PerformSubstitutions should run anytime we build anything else
ADD_DEPENDENCIES( BPUtils_s PerformSubstitution )

INCLUDE(support/build_scripts/BuildSDK.cmake)

