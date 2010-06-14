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
# CMake script to run various post-build things
#   - package sdk
#   - make installers
#   - install locally
#   - compress SDK

SET (BP_EXTERNAL_DIR 
     "${CMAKE_CURRENT_SOURCE_DIR}/../external/${CMAKE_SYSTEM_NAME}")

SET (BUILD_SCRIPTS_DIR 
     "${CMAKE_CURRENT_BINARY_DIR}/support/build_scripts")

SET (arch "i386") 	   	

IF (WIN32)
  SET (platform "Win32")
  SET (tarCmd "7z.exe") 	   	
  SET (tarArgs a -bd -r -y -tzip) 	   	
  SET (tarTarArgs a -bd -r -y -ttar) 	   	
  SET (tarSuffix zip) 	   	
  SET (elzmaCmd "${BP_EXTERNAL_DIR}/bin/elzma.exe") 	   	
ELSE ()
  IF (APPLE)
    SET (platform "Darwin")
  ELSE ()
    SET (arch `uname -m`)
    SET (platform "Linux")
  ENDIF ()
  SET (tarCmd tar) 	   	
  SET (tarArgs czvhf) 	   	
  SET (tarTarArgs cvhf) 	   	
  SET (tarSuffix tgz) 	   	
  SET (elzmaCmd "${BP_EXTERNAL_DIR}/bin/elzma")
ENDIF () 	 

SET (pabSdkFileName "pab_${VersionString}-${platform}-${arch}.tar")
SET (pabDir "pab_${VersionString}-${platform}")
SET (pabWorkDir "${CMAKE_CURRENT_BINARY_DIR}/${pabDir}/work")
SET (sdkFileName "bpsdk_${VersionString}-${platform}-${arch}.${tarSuffix}")
SET (sdkIntFileName "bpsdk_internal_${VersionString}-${platform}-${arch}.${tarSuffix}")

IF (WIN32)
    SET( platformDepends npybrowserplus YBPAddon BrowserPlusBootstrapper)
ELSEIF (APPLE)
    SET( platformDepends BrowserPlus BrowserPlusPrefs ) 
ENDIF ()

SET (allShippingDepends
     bpclient BrowserPlusCore ServiceInstaller ServicePublisher ServiceRunner
     bpsigner BrowserPlusInstaller BrowserPlusUninstaller BrowserPlusUpdater
     BrowserPlusPrefs BPProtocol bptar bpkg ${platformDepends})
     
#############################################################
# A cmake target to package BrowserPlus SDK via a ruby script
#############################################################
FILE(TO_NATIVE_PATH "${BUILD_SCRIPTS_DIR}/packageSDK.rb"
                 rubyPackageSDK)
ADD_CUSTOM_TARGET(PackageSDK ALL
                  DEPENDS ${allShippingDepends}
                  COMMAND ruby -s \"${rubyPackageSDK}\" -buildDir=\"${CMAKE_CURRENT_BINARY_DIR}\" -intDir=${CMAKE_CFG_INTDIR}
                  COMMENT "Package BrowserPlus SDK")
ADD_DEPENDENCIES(PackageSDK JavaScript)

                   
#############################################################
# A cmake target to make BrowserPlus installer via a ruby script
#############################################################
FILE(TO_NATIVE_PATH "${BUILD_SCRIPTS_DIR}/makeInstaller.rb"
                    rubyMakeInstaller)
ADD_CUSTOM_TARGET(MakeInstaller ALL
                  DEPENDS ${allShippingDepends}
                  COMMAND ruby -s \"${rubyMakeInstaller}\" -intDir=${CMAKE_CFG_INTDIR} -buildDir=\"${CMAKE_CURRENT_BINARY_DIR}\"
                  COMMENT "Make BrowserPlus installer")
ADD_DEPENDENCIES(MakeInstaller PackageSDK)

#############################################################
# A cmake target to install BrowserPlus locally at build time 
# Install is done via a ruby script
#############################################################
FILE(TO_NATIVE_PATH "${BUILD_SCRIPTS_DIR}/installLocally.rb" rubyInstaller)
ADD_CUSTOM_TARGET(InstallLocally ALL
                  DEPENDS ${allShippingDepends}
                  COMMAND ruby -s \"${rubyInstaller}\"
                  COMMENT "Install BrowserPlus on this machine")
ADD_DEPENDENCIES(InstallLocally MakeInstaller)

##### Zip it up, chris.  I'm about to.
# define a custom target for the compressing of the SDK
# this command is not run by default.  A good reason for this is if
# you want to copy anything else, say.. some services, into the
# sdk before compressing 
ADD_CUSTOM_TARGET(CompressSDK COMMENT "Compressing the BrowserPlus SDK")

# compress the public SDK
ADD_CUSTOM_COMMAND(TARGET CompressSDK
                   COMMAND ${tarCmd} ${tarArgs} ${sdkFileName} bpsdk
                   COMMENT "Compressing the BrowserPlus SDK")

# compress the internal SDK
ADD_CUSTOM_COMMAND(TARGET CompressSDK
                   COMMAND ${tarCmd} ${tarArgs} ${sdkIntFileName} bpsdk_internal
                   COMMENT "Compressing the Internal BrowserPlus SDK")

ADD_CUSTOM_COMMAND(TARGET CompressSDK
                   COMMAND ${CMAKE_COMMAND} -E remove_directory ${pabWorkDir}
                   COMMAND ${tarCmd} ${tarTarArgs} ${pabSdkFileName} ${pabDir}
                   COMMAND ${elzmaCmd} --lzip -v -z ${pabSdkFileName}
                   COMMENT "Compressing the BrowserPlus Installer SDK")
