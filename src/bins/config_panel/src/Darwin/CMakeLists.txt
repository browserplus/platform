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
# looser syntax

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# INCLUDE and link paths for platform libraries
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../../dist/include")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../../../../dist/lib")

# INCLUDE path for config panel Common headers
INCLUDE_DIRECTORIES("../Common")

# OSX specifics
ADD_DEFINITIONS(-D_APPLE -DXP_MACOSX)
  
# include carbon, cocoa, AppKit, and PreferencePanes
FIND_LIBRARY(COCOA_LIBRARY Cocoa)
MARK_AS_ADVANCED(COCOA_LIBRARY)

FIND_LIBRARY(CARBON_LIBRARY Carbon)
MARK_AS_ADVANCED(CARBON_LIBRARY)

FIND_LIBRARY(WEBKIT_LIBRARY WebKit)
MARK_AS_ADVANCED(WEBKIT_LIBRARY)

FIND_LIBRARY(APPKIT_LIBRARY AppKit)
MARK_AS_ADVANCED(APPKIT_LIBRARY)

FIND_LIBRARY(PREFPANES_LIBRARY PreferencePanes)
MARK_AS_ADVANCED(PREFPANES_LIBRARY)

SET(OS_LIBS
  ${PREFPANES_LIBRARY}
  ${APPKIT_LIBRARY}
  ${WEBKIT_LIBRARY}
  ${COCOA_LIBRARY}
  ${CARBON_LIBRARY}
)

# headers/sources/resources common to all platforms
SET(HDRS bpprefpane.h ../Common/ScriptableConfigObject.h)
SET(SRCS bpprefpane.mm ../Common/ScriptableConfigObject.cpp)

SET(LIBNAME BrowserPlusPrefs)

ADD_LIBRARY(${LIBNAME} MODULE ${SRCS} ${HDRS})

TARGET_LINK_LIBRARIES(${LIBNAME} BPProtocol_s
                      HTMLRender_s BPUtils_s ArchiveLib_s ${OS_LIBS})

# Shipping binaries dir
SET(pluginDir "${CMAKE_CURRENT_BINARY_DIR}/${LIBNAME}.prefPane")

GET_TARGET_PROPERTY(bundleLocation ${LIBNAME} LOCATION)

# path to the localizeUI.rb script
SET(localizeUIScript
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../support/l10n/localizeUI.rb")

# create the required directory structure
ADD_CUSTOM_COMMAND(TARGET ${LIBNAME} POST_BUILD
    COMMAND rm -rf \"${pluginDir}\"
    COMMAND mkdir -p \"${pluginDir}/Contents/MacOS\"
    COMMAND mkdir -p \"${pluginDir}/Contents/Resources\"
    COMMAND ruby -s ${localizeUIScript}
	                -indir=\"${CMAKE_CURRENT_SOURCE_DIR}/../../ui\" 
                    -outdir=\"${pluginDir}/Contents/Resources/ui\"
    COMMAND mkdir -p \"${pluginDir}/Contents/Resources/English.lproj\"
    COMMAND ln -sf \"${bundleLocation}\" 
                   \"${pluginDir}/Contents/MacOS/\"
    COMMAND ln -sf \"${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist\" 
                   \"${pluginDir}/Contents/Info.plist\"
    COMMAND ln -sf \"${CMAKE_CURRENT_SOURCE_DIR}/resources/bppreficon.tiff\" 
                   \"${pluginDir}/Contents/Resources\"
)
