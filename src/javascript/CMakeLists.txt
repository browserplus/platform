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
# set vars

SET(JSOutputDir "${CMAKE_CURRENT_BINARY_DIR}/../ShippingResources")
FILE(MAKE_DIRECTORY ${JSOutputDir})

SET(extPath "${CMAKE_CURRENT_SOURCE_DIR}/../../external/${CMAKE_SYSTEM_NAME}")

FILE(TO_NATIVE_PATH 
     "${extPath}/share/jars/yuicompressor-2.4.jar"
     yuiCompressorJarPath)

SET(outputDir "${JSOutputDir}/${VersionString}") 

ADD_CUSTOM_TARGET(JavaScript ALL COMMENT "Generate platform JavaScript")
ADD_CUSTOM_COMMAND(TARGET JavaScript POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${outputDir}) 
                    
# A macro to minify a file, creating xxx-min.js from xxx.js in the
# same directory
MACRO(MINIFY file) 
  # turn file into an absolute path
  IF (IS_ABSOLUTE ${file}) 
    SET(mySrc "${file}")
  ELSE ()
    SET(mySrc "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  ENDIF ()

  FIND_PROGRAM(javaPath "java")
  # transmorgify source into destination
  GET_FILENAME_COMPONENT(path ${mySrc} PATH)  
  GET_FILENAME_COMPONENT(name ${mySrc} NAME_WE)  
  GET_FILENAME_COMPONENT(ext ${mySrc} EXT)    
  FILE(TO_NATIVE_PATH "${mySrc}" mySrc)
  FILE(TO_NATIVE_PATH "${path}/${name}-min${ext}" myDst)

  ADD_CUSTOM_COMMAND(
    TARGET JavaScript POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo minifying --${mySrc}--
  )

  IF (javaPath)
    FILE(TO_NATIVE_PATH ${javaPath} javaPath)
    ADD_CUSTOM_COMMAND(
      TARGET JavaScript POST_BUILD
      COMMAND ${javaPath} -jar ${yuiCompressorJarPath} ${mySrc} -o ${myDst}
    )
  ELSE ()
    ADD_CUSTOM_COMMAND(
      TARGET JavaScript POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${mySrc} ${myDst}
    )
  ENDIF ()

ENDMACRO(MINIFY file) 

# A macro to minify a file, creating xxx-min.js from xxx.js in the
# same directory
MACRO(INSTALL_RESOURCE file path)
  IF (IS_ABSOLUTE ${file}) 
    SET(src ${file})
  ELSE ()
    SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  ENDIF ()

  SET(targetDir "${JSOutputDir}/${path}")
  FILE(MAKE_DIRECTORY ${targetDir})

  ADD_CUSTOM_COMMAND(
    TARGET JavaScript POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${targetDir}
  )
ENDMACRO(INSTALL_RESOURCE file path)

MINIFY(${CMAKE_CURRENT_BINARY_DIR}/src/browserplus.js)
INSTALL_RESOURCE(${CMAKE_CURRENT_BINARY_DIR}/src/browserplus.js
                 "${VersionString}")
INSTALL_RESOURCE(${CMAKE_CURRENT_BINARY_DIR}/src/browserplus-min.js
                 "${VersionString}")

ADD_DEPENDENCIES( JavaScript PerformSubstitution )
