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
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)
SET(YBT_NESTED_INCLUDES 1)

# set include paths to build configs cmake and build templates cmake
SET(YBP_BuildTemplates
  "${CMAKE_CURRENT_SOURCE_DIR}/support/BuildTemplates/BuildTemplates.cmake"
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

IF (APPLE)
  MESSAGE("!! CMAKE_OSX_SYSROOT == ${CMAKE_OSX_SYSROOT}")
ENDIF ()

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

ADD_DEFINITIONS(-DBPVERSION=\"${VersionString}\")

# If you add an entry here, you probably also need to add it to 
# "allShippingDepends" in BuildSDK.cmake
ADD_SUBDIRECTORY(libs)
ADD_SUBDIRECTORY(support)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(bins)
ADD_SUBDIRECTORY(sdk)
ADD_SUBDIRECTORY(javascript)

# a custom step to build latest externals
SET (pathToExtBuildRb "${CMAKE_CURRENT_SOURCE_DIR}/../external/build.rb")
ADD_CUSTOM_TARGET(
    BuildExternals ALL
    COMMAND ruby ${pathToExtBuildRb}
    COMMENT "Build 3rd party dependencies")

# PerformSubstitutions should run anytime we build anything else
ADD_DEPENDENCIES( BPUtils_s PerformSubstitution )

# And externals build check should run absolutely first
ADD_DEPENDENCIES( PerformSubstitution BuildExternals )

INCLUDE(support/build_scripts/BuildSDK.cmake)

# and drop a time stamp file that can be used by various parts of the
# build system (for instance, erb substitution)
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/time.stamp" "")
