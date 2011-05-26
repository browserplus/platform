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
SET(BPUtils_MAJOR_VERSION 0)
SET(BPUtils_MINOR_VERSION 1)

# We need openssl from external
SET(BP_EXTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/../../../../external/dist")

LINK_DIRECTORIES("${BP_EXTERNAL}/lib")

INCLUDE_DIRECTORIES("${BP_EXTERNAL}/include")

# randomized definitions for global symbols exported into objc space
BPRandDefn(MY_TIMER_FIRED_CLASS)
BPRandDefn(HTTP_TRANS_HELPER)

IF (WIN32)
  SET(OS_LIBS "rpcrt4" "winmm" "Ws2_32" "Wininet")
  SET(SSL_LIBS "libeay32_s" "ssleay32_s")
   # Visual Studio does some autolink magic with boost, no need to specify 
   # library
   # tr1::regex part of vs2008
  SET(BOOST_LIBS)
ELSE ()
  SET(OS_LIBS)
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
  ELSE() # linux
    SET(OS_LIBS "pthread" "uuid" "dl")
  ENDIF ()
  SET(SSL_LIBS "ssl_s" "crypto_s")
  SET(BOOST_LIBS "boost_filesystem" "boost_system" "boost_regex")
ENDIF ()

YBT_BUILD(LIBRARY_STATIC BPUtils)

YBT_GENERATE_INCLUDE_FILE("${CMAKE_CURRENT_BINARY_DIR}/BPUtils.cmake")
TARGET_LINK_LIBRARIES(BPUtils_s yajl_s mongoose_s ${SSL_LIBS} ${OS_LIBS} ${BOOST_LIBS})
