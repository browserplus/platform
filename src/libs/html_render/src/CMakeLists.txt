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
SET(libName HTMLRender)

SET(${libName}_MAJOR_VERSION 0)
SET(${libName}_MINOR_VERSION 1)
SET(${libName}_LINK_STATIC BPUtils)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../ServiceAPI/api")

IF (WIN32)
  SET(OS_LIBS "")
  ADD_DEFINITIONS(-D_ATL_ATTRIBUTES)
ELSE ()
  SET(OS_LIBS)
  IF(APPLE)
    # need carbon headers and library
    #  INCLUDE_DIRECTORIES(/Developer/Headers/FlatCarbon)
    FIND_LIBRARY(CARBON_LIBRARY Carbon)
    MARK_AS_ADVANCED(CARBON_LIBRARY)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    MARK_AS_ADVANCED(COCOA_LIBRARY)
    FIND_LIBRARY(WEBKIT_LIBRARY WebKit)
    MARK_AS_ADVANCED(WEBKIT_LIBRARY)

    SET(OS_LIBS ${WEBKIT_LIBRARY} ${COCOA_LIBRARY} ${CARBON_LIBRARY})
  ENDIF ()
ENDIF ()

YBT_BUILD(LIBRARY_STATIC ${libName})
TARGET_LINK_LIBRARIES(${libName}_s ${OS_LIBS})
ADD_DEPENDENCIES( ${libName}_s BPUtils_s )
