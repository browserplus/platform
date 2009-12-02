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
# Note: This file sets up 3 targets (4 on Windows): 
#   BrowserPlusInstaller 
#   BrowserPlusUninstaller
#   BrowserPlusUpdater


SET(installerXplatSources
    BrowserPlusInstaller.cpp 
    InstallerSkin.h InstallerSkin.cpp 
    InstallerSkinGUI.h
    ScriptableInstallerObject.h ScriptableInstallerObject.cpp
    InstallerSkinVerbose.h InstallerSkinVerbose.cpp
    InstallerRunner.h InstallerRunner.cpp)

IF (WIN32)
    SET(BrowserPlusInstaller_SOURCES 
        ${installerXplatSources} 
        ${CMAKE_SYSTEM_NAME}/InstallerSkinGUI_Windows.cpp 
        ${CMAKE_SYSTEM_NAME}/resource.h 
        ${CMAKE_SYSTEM_NAME}/BrowserPlusInstaller.rc)

    SET(BrowserPlusUninstaller_SOURCES 
        BrowserPlusUninstaller_Windows.cpp
        ${CMAKE_SYSTEM_NAME}/resource.h 
        ${CMAKE_SYSTEM_NAME}/BrowserPlusUninstaller.rc)
ELSEIF (APPLE)
    SET(BrowserPlusInstaller_SOURCES 
        ${installerXplatSources} 
        ${CMAKE_SYSTEM_NAME}/InstallerSkinGUI_Darwin.mm)

    SET(BrowserPlusUninstaller_SOURCES 
        BrowserPlusUninstaller_UNIX.cpp)
ELSE ()
    SET(BrowserPlusInstaller_SOURCES 
        ${installerXplatSources} 
        ${CMAKE_SYSTEM_NAME}/InstallerSkinGUI_Linux.cpp)

    SET(BrowserPlusUninstaller_SOURCES 
        BrowserPlusUninstaller_UNIX.cpp)
ENDIF ()

SET(BrowserPlusUpdater_SOURCES BrowserPlusUpdater.cpp)


SET(BrowserPlusInstaller_LINK_STATIC 
    BPInstaller BPUtils HTMLRender DistributionClient CoreletManager)
SET(BrowserPlusUninstaller_LINK_STATIC BPInstaller BPUtils)
SET(BrowserPlusUpdater_LINK_STATIC BPInstaller BPUtils)

SET(BrowserPlusInstaller_NO_VERSION_RESOURCES 1)
SET(BrowserPlusUninstaller_NO_VERSION_RESOURCES 1)
SET(BrowserPlusUpdater_NO_VERSION_RESOURCES 1)

YBT_BUILD(BINARY BrowserPlusInstaller)
ADD_DEPENDENCIES(BrowserPlusInstaller BPInstaller_s)

# The WIN32 arg causes linker subsytem GUI instead of CONSOLE.
YBT_BUILD(BINARY BrowserPlusUninstaller WIN32)
ADD_DEPENDENCIES(BrowserPlusUninstaller BPInstaller_s)

YBT_BUILD(BINARY BrowserPlusUpdater)
ADD_DEPENDENCIES(BrowserPlusUpdater BPInstaller_s)

IF (WIN32)
    SET_TARGET_PROPERTIES(BrowserPlusInstaller PROPERTIES LINK_FLAGS
                          "/MACHINE:X86")
    SET_TARGET_PROPERTIES(BrowserPlusUninstaller PROPERTIES LINK_FLAGS
                          "/MACHINE:X86")
ENDIF (WIN32)

IF (WIN32)
   # Make the installer, updater, uninstaller, and removeOld run 
   # asInvoked on vista
   # See http://msdn.microsoft.com/en-us/library/bb756960.aspx
   # Must use correct version of mt.exe
   SET (mtExe "$ENV{WindowsSdkDir}\\bin\\mt.exe")
   GET_TARGET_PROPERTY(installerExe BrowserPlusInstaller LOCATION)
   ADD_CUSTOM_COMMAND(TARGET BrowserPlusInstaller POST_BUILD
       COMMAND "${mtExe}" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\Windows\\asInvoker.manifest\" -outputresource:\"${installerExe}\"\;\#1)
   GET_TARGET_PROPERTY(updaterExe BrowserPlusUpdater LOCATION)
   ADD_CUSTOM_COMMAND(TARGET BrowserPlusUpdater POST_BUILD
       COMMAND "${mtExe}" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\Windows\\asInvoker.manifest\" -outputresource:\"${updaterExe}\"\;\#1)
   GET_TARGET_PROPERTY(uninstallerExe BrowserPlusUninstaller LOCATION)
   ADD_CUSTOM_COMMAND(TARGET BrowserPlusUninstaller POST_BUILD
       COMMAND "${mtExe}" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\Windows\\asInvoker.manifest\" -outputresource:\"${uninstallerExe}\"\;\#1)
ENDIF (WIN32)


