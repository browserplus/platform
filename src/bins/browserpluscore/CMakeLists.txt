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
IF (APPLE)
  # include carbon
  # INCLUDE_DIRECTORIES(/Developer/Headers/FlatCarbon)
  FIND_LIBRARY(CARBON_LIBRARY Carbon)
  MARK_AS_ADVANCED(CARBON_LIBRARY)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  MARK_AS_ADVANCED(COCOA_LIBRARY)
  SET(OS_LIBS ${CARBON_LIBRARY} ${COCOA_LIBRARY})
ELSEIF (WIN32)
  SET(OS_LIBS) # none (!)
ELSEIF (LINUX)
  SET(OS_LIBS crypto ssl) # hack
ENDIF(APPLE)

SET(BrowserPlusCore_LINK_STATIC 
    DistributionClient CoreletManager Permissions
    ServiceRunnerLib BPUtils I18n)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../ServiceAPI/api")

YBT_BUILD(BINARY BrowserPlusCore)
TARGET_LINK_LIBRARIES(BrowserPlusCore ${OS_LIBS})
IF (APPLE)
  SET_TARGET_PROPERTIES(BrowserPlusCore PROPERTIES
                        LINK_FLAGS "-exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/exported_symbols.txt -flat_namespace")
ENDIF ()
