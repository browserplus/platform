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
# A CMake build file for the HelloWorld service, a simple demonstration
# of how to write and build a service.
# 
# Author: Lloyd Hilaiel
# (c) Yahoo 2007, all rights reserved
#

# looser syntax
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# service name
SET (serviceName SampleService)

# Name our project
PROJECT(${serviceName})

IF (WIN32)
  IF (MSVC90)
    INCLUDE_EXTERNAL_MSPROJECT(
      "${serviceName}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${serviceName}_vs9.vcproj"
      PackageSDK)
  ELSE ()
    INCLUDE_EXTERNAL_MSPROJECT(
      "${serviceName}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${serviceName}.vcxproj"
      PackageSDK)
  ENDIF ()
ELSE ()
  ADD_CUSTOM_TARGET(
      ${serviceName} ALL
      COMMAND echo SRC_PATH=${CMAKE_CURRENT_SOURCE_DIR}
	          BPSDK_PATH=${CMAKE_CURRENT_BINARY_DIR}/../../../bpsdk
			  make
      COMMAND SRC_PATH=${CMAKE_CURRENT_SOURCE_DIR}
	          BPSDK_PATH="../../../bpsdk"
			  make -f "${CMAKE_CURRENT_SOURCE_DIR}/Makefile"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  ADD_DEPENDENCIES(${serviceName} PackageSDK)
ENDIF ()
