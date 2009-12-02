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
MACRO(BPRandDefn myCPPDefinition)
  STRING(RANDOM LENGTH 5 randStr)
  # this cache magic ensures that the rand name doesn't
  # change through multiple cmake invocations.  Which makes
  # visual studio angry
  SET(cacheVarName ${myCPPDefinition}_RAND_NAME)
  SET(${cacheVarName} "${myCPPDefinition}${randStr}"
      CACHE STRING "random name generated for preproc def: ${myCPPDefinition}")
  ADD_DEFINITIONS("-D${myCPPDefinition}=${${cacheVarName}}")
ENDMACRO(BPRandDefn myCPPDefinition)
