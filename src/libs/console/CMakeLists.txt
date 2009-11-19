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
PROJECT(ConsoleLib)

SET(ConsoleLib_MAJOR_VERSION 0)
SET(ConsoleLib_MINOR_VERSION 1)

SET(ConsoleLib_LINK_STATIC BPUtils)

INCLUDE_DIRECTORIES("${BP_EXTERNAL}/include"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../ServiceAPI/api")

YBT_BUILD(LIBRARY_STATIC ConsoleLib)
IF (NOT WIN32) 
   SET(OSLIBS)
   FIND_LIBRARY(termcapLib termcap)           
   IF (${termcapLib})
     SET(OS_LIBS ${termcapLib})
   ENDIF (${termcapLib})   
   FIND_LIBRARY(cursesLib curses)           
   MESSAGE("cursesLib: ${cursesLib}")
   IF (cursesLib)
     SET(OS_LIBS ${OS_LIBS} ${cursesLib})
     MESSAGE("TRUE!}")
   ENDIF (cursesLib)   
   MESSAGE("OS_LIBS: ${OS_LIBS}")
   TARGET_LINK_LIBRARIES(ConsoleLib_s edit_s ${OS_LIBS})  
ENDIF ()


ADD_DEPENDENCIES(ConsoleLib_s BPUtils_s) 
