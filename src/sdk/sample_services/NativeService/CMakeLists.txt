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
# A CMake build file for the HelloWorld corelet, a simple demonstration
# of how to write and build a corelet.
# 
# Author: Lloyd Hilaiel
# (c) Yahoo 2007, all rights reserved
#

# looser syntax
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# corelet name
SET (coreletName SampleService)

# Name our project
PROJECT(${coreletName}Service)

IF (APPLE)
  ADD_CUSTOM_TARGET(
      ${coreletName} ALL
      COMMAND echo SRC_PATH=${CMAKE_CURRENT_SOURCE_DIR}
	          BPSDK_PATH=${CMAKE_CURRENT_BINARY_DIR}/../../../bpsdk
			  make
      COMMAND SRC_PATH=${CMAKE_CURRENT_SOURCE_DIR}
	          BPSDK_PATH="../../../bpsdk"
			  make -f "${CMAKE_CURRENT_SOURCE_DIR}/Makefile"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  ADD_DEPENDENCIES(${coreletName} PackageSDK)
ELSEIF (WIN32)
  INCLUDE_EXTERNAL_MSPROJECT(
    "${coreletName}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${coreletName}.vcproj"
    PackageSDK)
ENDIF ()
