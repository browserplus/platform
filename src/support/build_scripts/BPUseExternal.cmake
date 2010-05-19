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
############################################################
# BP_USE_EXTERNAL -- 
# set include and link paths correctly to "use" external
# libraries
# also set BP_EXTERNAL_BINS and BP_EXTERNAL_LIBS and BP_EXTERNAL_INCLUDES
# to contain lists of header and library paths 
# NOTE: this function takes no arguments, it just sets up include
#       and link paths! 
############################################################
MACRO (BP_USE_EXTERNAL)
  SET(BP_EXTERNAL_BINS)
  SET(BP_EXTERNAL_LIBS)
  SET(BP_EXTERNAL_INCLUDES)
  FOREACH (binDir ${BP_EXTERNAL_DIR}/${CMAKE_SYSTEM}/bin
                  ${BP_EXTERNAL_DIR}/${CMAKE_SYSTEM_NAME}/bin)
    IF (EXISTS ${binDir})
      LINK_DIRECTORIES(${binDir})
      LINK_DIRECTORIES(${binDir}/$ENV{EXTERNAL_SUFFIX})
      SET(BP_EXTERNAL_BINS ${BP_EXTERNAL_BINS} ${binDir})
      SET(BP_EXTERNAL_BINS ${BP_EXTERNAL_BINS} ${binDir}/$ENV{EXTERNAL_SUFFIX})
    ENDIF ()
  ENDFOREACH ()

  FOREACH (incDir ${BP_EXTERNAL_DIR}/${CMAKE_SYSTEM}/include
                   ${BP_EXTERNAL_DIR}/${CMAKE_SYSTEM_NAME}/include)
    IF (EXISTS ${incDir})
      INCLUDE_DIRECTORIES(${incDir})
      SET(BP_EXTERNAL_INCLUDES ${BP_EXTERNAL_INCLUDES} ${incDir})
    ENDIF ()
  ENDFOREACH ()

  FOREACH (libDir ${BP_EXTERNAL_DIR}/${CMAKE_SYSTEM}/lib
                  ${BP_EXTERNAL_DIR}/${CMAKE_SYSTEM_NAME}/lib)
    IF (EXISTS ${libDir})
      LINK_DIRECTORIES(${libDir})
      LINK_DIRECTORIES(${libDir}/$ENV{EXTERNAL_SUFFIX})
      SET(BP_EXTERNAL_LIBS ${BP_EXTERNAL_LIBS} ${libDir})
      SET(BP_EXTERNAL_LIBS ${BP_EXTERNAL_LIBS} ${libDir}/$ENV{EXTERNAL_SUFFIX})
    ENDIF ()
  ENDFOREACH ()
ENDMACRO (BP_USE_EXTERNAL)
