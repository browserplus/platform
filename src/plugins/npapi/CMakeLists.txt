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
# looser syntax
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

PROJECT(BrowserPlusPlugin)

# Manually set up include and link directories
#SET(BP_EXTERNAL_DIR 
#    "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/${CMAKE_SYSTEM_NAME}")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../dist/include")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../dist/include")

IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF ()

# include root source directory for easy header access when in subdirs 
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/DragAndDrop)

# required npapi defines
ADD_DEFINITIONS(-DMOZILLA_STRICT_API -DXPCOM_GLUE -DNPSIMPLE_EXPORTS)

# OSX specifics
IF(APPLE)
  ADD_DEFINITIONS(-D_APPLE -DXP_MACOSX)
  
  # include carbon
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  MARK_AS_ADVANCED(COCOA_LIBRARY)
  FIND_LIBRARY(CARBON_LIBRARY Carbon)
  MARK_AS_ADVANCED(CARBON_LIBRARY)

  FIND_LIBRARY(WEBKIT_LIBRARY WebKit)
  MARK_AS_ADVANCED(WEBKIT_LIBRARY)
  SET(OS_LIBS ${WEBKIT_LIBRARY})
  MESSAGE("WebKit Library: ${WEBKIT_LIBRARY}")

  SET(OS_LIBS ${WEBKIT_LIBRARY} ${COCOA_LIBRARY} ${CARBON_LIBRARY} ssl_s crypto_s)
ELSEIF(WIN32)
  ADD_DEFINITIONS(-D_WINDOWS -DWIN32 -DXP_WIN32 -DXP_WIN -D_X86_)

# some atl settings
  SET(CMAKE_CXX_WARNING_LEVEL "4" CACHE STRING "Warning level" FORCE)
  SET(CMAKE_CXX_USE_RTTI YES CACHE BOOL "RTTI" FORCE)
  SET(CMAKE_CXX_WIN9X_UNICODE NO CACHE BOOL "WIN9X_UNICODE" FORCE)
  SET(CMAKE_ATL_FLAG "2" CACHE STRING "ATL" FORCE)
  ADD_DEFINITIONS(-D_ATL_ATTRIBUTES -D_USRDLL)
  SET(ATL_LIBS comsuppw)

# SET(OS_LIBS Ws2_32 mswsock psapi ssleay32 libeay32 DbgHelp)
  SET(OS_LIBS ${ATL_LIBS} Ws2_32 mswsock psapi ssleay32_s libeay32_s DbgHelp)

  INCLUDE_DIRECTORIES(packaging/windows)
ELSE()
  ADD_DEFINITIONS(-DXP_UNIX)
  SET(OS_LIBS ssl_s crypto_s)
ENDIF()

# headers/sources/resources common to all platforms
SET(BPHDRS src/common.h src/BPScriptableObject.h 
    src/DragAndDrop/DnDPlugletFactoryNPAPI.h
    src/DragAndDrop/DropManagerFactory.h
    src/DragAndDrop/InterceptDropManager.h
    src/DragAndDrop/Html5DropManager.h
    src/ScriptablePluginObjectBase.h src/GenericJSObject.h
    src/nputils.h src/NPAPIObject.h src/NPAPIVariant.h src/NPAPIPlugin.h)
SET(BPSRCS
    # these files are substituted using .erb
    ${CMAKE_CURRENT_BINARY_DIR}/src/main.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/BPScriptableObject.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/DragAndDrop/Html5DropManager.cpp
    # these files are not
    src/NPAPIPlugin.cpp
    src/ScriptablePluginObjectBase.cpp
    src/GenericJSObject.cpp src/nputils.cpp
    src/DragAndDrop/DnDPlugletFactoryNPAPI.cpp
    src/DragAndDrop/DnDPlugletNPAPI.cpp
    src/DragAndDrop/DropManagerFactory.cpp
    src/DragAndDrop/InterceptDropManager.cpp)

SET(BPRSRCS "")

IF(APPLE)
  SET(BPSRCS ${BPSRCS} src/DragAndDrop/InterceptDropManager_Darwin.cpp)
  # MacOSX specific packaging resources
  SET(BPRSRCS ${BPRSRCS} 
      ${CMAKE_CURRENT_BINARY_DIR}/packaging/mac/Localized.r)
ELSEIF(WIN32)
  SET(BPSRCS ${BPSRCS} src/DragAndDrop/InterceptDropManager_Windows.cpp)
  # Windows specific packaging gunk
  SET(BPHDRS ${BPHDRS} packaging/windows/resource.h)
  SET(BPSRCS ${BPSRCS} packaging/windows/npybp.def)
  SET(BPRSRCS ${BPRSRCS} ${CMAKE_CURRENT_BINARY_DIR}/packaging/windows/npybp.rc)
ENDIF()

# Nice groupings for IDEs
SOURCE_GROUP(Headers FILES ${BPHDRS})
SOURCE_GROUP(Sources FILES ${BPSRCS})
SOURCE_GROUP(Resources FILES ${BPRSRCS})

# Library name.  Note that on windows it must be "npxxx"
SET(BPLIB BrowserPlus)
IF(WIN32)
  SET(BPLIB npybrowserplus)
ENDIF()

IF (WIN32)
  ADD_LIBRARY(${BPLIB} MODULE ${BPSRCS} ${BPHDRS} ${BPRSRCS})
ELSE ()
  ADD_LIBRARY(${BPLIB} MODULE ${BPSRCS} ${BPHDRS})
ENDIF ()

TARGET_LINK_LIBRARIES( ${BPLIB} BPUtils_s BPProtocol_s
                                PluginCommonLib_s ${OS_LIBS} )

# Make sure that html5 erb substitution occurs if input js files change
SET(HTML5JS "${CMAKE_CURRENT_SOURCE_DIR}/src/DragAndDrop/addDropTarget.js"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/DragAndDrop/removeDropTarget.js")
SET(TOPDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
SET(TOPBINDIR "${CMAKE_CURRENT_BINARY_DIR}/../../")
SET(ERB_SUBST "${TOPDIR}/support/build_scripts/erb_subst.rb")
FILE(TO_NATIVE_PATH "${TOPBINDIR}/cmakeContext.rb" ctxPath)
SET(l10nData "${TOPDIR}/support/l10n/strings.json")
ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/DragAndDrop/Html5DropManager.cpp"
    COMMAND ruby -s \"${ERB_SUBST}\" -force=true -context=\"${ctxPath}\" -l10n=\"${l10nData}\" -output_dir=\"${TOPBINDIR}\" -input_file=\"plugins/npapi/src/DragAndDrop/Html5DropManager.cpp.erb\"
    COMMAND ruby -s \"${ERB_SUBST}\" -force=true -context=\"${ctxPath}\" -l10n=\"${l10nData}\" -output_dir=\"${TOPBINDIR}\" -input_file=\"plugins/npapi/src/DragAndDrop/dnd_test.html.erb\"
    DEPENDS ${HTML5JS}
    WORKING_DIRECTORY "${TOPDIR}"
    COMMENT "Perform html5 drag/drop .erb file substitution")

IF (APPLE)
  # now we manually setup the plugin "bundle"
  GET_TARGET_PROPERTY(BP_LOCATION ${BPLIB} LOCATION)

  # Text resource file ".r" must be compiled into a ".rsrc", which is put
  # into bundle.
  ADD_CUSTOM_COMMAND(TARGET ${BPLIB} POST_BUILD
      COMMAND /Developer/Tools/Rez  -d SystemSevenOrLater=1 -useDF
              -script Roman -arch i386
              \"${CMAKE_CURRENT_BINARY_DIR}/packaging/mac/Localized.r\"
              -o \"${CMAKE_CURRENT_BINARY_DIR}/Localized.rsrc\"
  )

  SET(pluginDir
      "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${BPLIB}.plugin")

  # create the required directory structure
  ADD_CUSTOM_COMMAND(TARGET ${BPLIB} POST_BUILD
      COMMAND mkdir -p \"${pluginDir}/Contents/MacOS\"
      COMMAND mkdir -p \"${pluginDir}/Contents/Resources/English.lproj\"
      COMMAND ln -sf \"${BP_LOCATION}\" 
                     \"${pluginDir}/Contents/MacOS/BrowserPlus\"
      COMMAND ln -sf \"${CMAKE_CURRENT_BINARY_DIR}/packaging/mac/Info.plist\" 
                     \"${pluginDir}/Contents/Info.plist\"
      COMMAND ln -sf \"${CMAKE_CURRENT_BINARY_DIR}/Localized.rsrc\"
            \"${pluginDir}/Contents/Resources/English.lproj/Localized.rsrc\"
  )
ENDIF (APPLE) 

IF (WIN32)
    SET_TARGET_PROPERTIES(${BPLIB} PROPERTIES LINK_FLAGS
                          "/MACHINE:X86")
ENDIF ()
