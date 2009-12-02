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
SET(progName ServiceInstaller)

SET(${progName}_MAJOR_VERSION 0)
SET(${progName}_MINOR_VERSION 1)
SET(${progName}_LINK_STATIC ServiceRunnerLib BPUtils)

# for bptypes header required by BPUtils
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../ServiceAPI/api")

YBT_BUILD(BINARY ${progName})

IF (WIN32)
   # Make ServiceInstaller run asInvoked on vista
   # See http://msdn.microsoft.com/en-us/library/bb756960.aspx
   SET (mtExe "$ENV{WindowsSdkDir}\\bin\\mt.exe")
   GET_TARGET_PROPERTY(exeName ${progName} LOCATION)
   ADD_CUSTOM_COMMAND(TARGET ${progName} POST_BUILD
       COMMAND "${mtExe}" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\asInvoker.manifest\" -outputresource:\"${exeName}\"\;\#1)
ENDIF (WIN32)

