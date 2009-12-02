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
# set the test output directory
GET_FILENAME_COMPONENT(testOutputDirectoryPath
                       "${CMAKE_CURRENT_BINARY_DIR}/tests"
					   ABSOLUTE)

SET(testOutputDirectory "${testOutputDirectoryPath}" CACHE STRING
    "Where BrowserPlus tests live" FORCE)

FILE(MAKE_DIRECTORY ${testOutputDirectory})

# and a macro for building tests
MACRO (BPAddTest binName)
  GET_TARGET_PROPERTY(${binName}_outputPath ${binName} LOCATION)
  GET_FILENAME_COMPONENT(origPath ${${binName}_outputPath} ABSOLUTE)
  GET_FILENAME_COMPONENT(baseName ${${binName}_outputPath} NAME)

  # Set custom flags for unit tests.
  IF (WIN32)
    # Disable 4127 due to cppunit issue.
    SET_TARGET_PROPERTIES(${binName} PROPERTIES COMPILE_FLAGS "/wd4127")
  ENDIF ()

  MESSAGE("++ Adding Test: ${binName}")
  ADD_CUSTOM_COMMAND(TARGET ${binName} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "${origPath}" "${testOutputDirectory}"
	  COMMAND ${CMAKE_COMMAND} -E echo "++ Running Test: ${baseName}"
	  COMMAND "./${baseName}"
	  WORKING_DIRECTORY "${testOutputDirectory}")
ENDMACRO ()


