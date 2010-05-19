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
# CMake script to generate build artifacts from .erb files
               
SET(ERB_SUBST "${CMAKE_CURRENT_SOURCE_DIR}/support/build_scripts/erb_subst.rb")
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cmakeContext.rb" ctxPath)
SET(l10nData "${CMAKE_CURRENT_SOURCE_DIR}/support/l10n/strings.json")

ADD_CUSTOM_TARGET(
    PerformSubstitution ALL
    COMMAND ruby -s \"${ERB_SUBST}\" -context=\"${ctxPath}\" -l10n=\"${l10nData}\" -output_dir=\"${CMAKE_CURRENT_BINARY_DIR}\"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Perform .erb file substitution")

# now generate that cmakeContext.rb file
SET(AAAAAAAAAAA "the first variable name")
GET_CMAKE_PROPERTY(varDump VARIABLES)
SET(rubyFile "{")
SET(first true)

FOREACH(v ${varDump})
  IF (first)
    SET(first false)
  ELSE (first)
    SET (rubyFile "${rubyFile},")
  ENDIF (first)
    
  SET (rubyFile "${rubyFile}\n")
  STRING (REGEX REPLACE "([\\'])" "\\\\\\1" "rubyEscapedVar" "${${v}}")
  SET (rubyFile "${rubyFile}'${v}' => '${rubyEscapedVar}'")   
ENDFOREACH()

SET (rubyFile "${rubyFile}\n}")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cmakeContext.rb" ${rubyFile})

# Now create placeholders for all .erb files so the build can proceed
# properl.
FILE(GLOB_RECURSE filesToConfig ${CMAKE_CURRENT_SOURCE_DIR} "*.erb")
FOREACH(f ${filesToConfig})
  GET_FILENAME_COMPONENT(dir "${f}" PATH)
  GET_FILENAME_COMPONENT(inName "${f}" NAME)
  STRING(REPLACE ".erb" "" outName "${inName}")
  FILE(RELATIVE_PATH relDirName "${CMAKE_CURRENT_SOURCE_DIR}" "${dir}")
  SET(newDir "${CMAKE_CURRENT_BINARY_DIR}/${relDirName}") 
  SET(fName "${newDir}/${outName}")
  IF (NOT EXISTS "${fName}") 
    MESSAGE("Creating placeholder: ${fName}")
    FILE(MAKE_DIRECTORY ${newDir})
    FILE(WRITE "${fName}" "")
  ENDIF ()
ENDFOREACH(f)
