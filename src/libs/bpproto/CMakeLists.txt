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
SET( libName BPProtocol )

SET( ${libName}_MAJOR_VERSION 0)
SET( ${libName}_MINOR_VERSION 1)
SET( ${libName}_LINK_STATIC BPUtils)

YBT_BUILD(LIBRARY ${libName})

ADD_DEPENDENCIES(BPProtocol BPUtils_s) 
ADD_DEPENDENCIES(BPProtocol_s BPUtils_s) 

############################################################
# now copy in the required subset of the ServiceAPI to make this thing
# more self contained.
############################################################

FILE(TO_NATIVE_PATH
     "${CMAKE_CURRENT_BINARY_DIR}/dist/include/ServiceAPI"
	 apiDir)

SET(mkdirArg) 

IF (NOT WIN32)
  SET(mkdirArg "-p")
ENDIF ()

ADD_CUSTOM_COMMAND(TARGET BPProtocol_s POST_BUILD
                   COMMAND mkdir ${mkdirArg} \"${apiDir}\")

FILE(GLOB headers
"${CMAKE_CURRENT_SOURCE_DIR}/../ServiceAPI/api/ServiceAPI/bptypes.h"
"${CMAKE_CURRENT_SOURCE_DIR}/../ServiceAPI/api/ServiceAPI/bperror.h"
"${CMAKE_CURRENT_SOURCE_DIR}/../ServiceAPI/api/ServiceAPI/bpdefinition.h"
"${CMAKE_CURRENT_SOURCE_DIR}/../ServiceAPI/api/ServiceAPI/bpuserprompt.h"
)

FOREACH (header ${headers})
  GET_FILENAME_COMPONENT(shortname "${header}" NAME)
  ADD_CUSTOM_COMMAND(TARGET BPProtocol_s POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo
	           "Copying ${shortname} to BPProtocol SDK to include/ServiceAPI/"
      COMMAND ${CMAKE_COMMAND} -E copy "${header}"
              "${CMAKE_CURRENT_BINARY_DIR}/dist/include/ServiceAPI")
ENDFOREACH ()
