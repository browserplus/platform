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
SET(BPInstaller_MAJOR_VERSION 0)
SET(BPInstaller_MINOR_VERSION 1)

IF(APPLE)
    # need carbon headers and library
    #  INCLUDE_DIRECTORIES(/Developer/Headers/FlatCarbon)
    FIND_LIBRARY(CARBON_LIBRARY Carbon)
    MARK_AS_ADVANCED(CARBON_LIBRARY)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    MARK_AS_ADVANCED(COCOA_LIBRARY)
    FIND_LIBRARY(SYSTEM_CONFIGURATION SystemConfiguration)
    MARK_AS_ADVANCED(SYSTEM_CONFIGURATION)

    SET(OS_LIBS ${COCOA_LIBRARY} ${CARBON_LIBRARY} ${SYSTEM_CONFIGURATION})
ELSE()
    SET(OS_LIBS ssleay32_s libeay32_s)
ENDIF()

SET(BPInstaller_LINK_STATIC BPUtils DistributionClient
                            CoreletManager ServiceRunnerLib)
YBT_BUILD(LIBRARY_STATIC BPInstaller)
ADD_DEPENDENCIES( BPInstaller_s DistributionClient_s BPUtils_s
                  CoreletManager_s ServiceRunnerLib_s)
