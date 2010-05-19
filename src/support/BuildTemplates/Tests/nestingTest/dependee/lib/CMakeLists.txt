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
#############################################################
# Library CMakeLists for the YStockQuote library
#
# Created by Lloyd Hilaiel on Mon May 1 2006.
# Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
#############################################################

######
# this is a CMakeLists.txt file to build "myLibrary"
######

# State our dependencies, which will set include and link paths
# XXX

# Set the library version
SET(YStockQuote_MAJOR_VERSION 1) 
SET(YStockQuote_MINOR_VERSION 0) 

# Specify which sources should be compiled into this library.  There
# are a couple options:
#  1. explicitly state source and headers (public and private)
#  2. allow the YBT_BUILD macro to dynamically determine source and headers
#  3. #2, but specific "IGNORE PATTERNS" which is a regex of files to ignore
#
# Example of #1 (explicit statment of source files):
#   FILE(GLOB myLibrary_SOURCES *.cpp) 
#   SET(myLibrary_PRIVATE_HEADERS this.h that.h) 
#   SET(myLibrary_PUBLIC_HEADERS one.h two.h) 
#
# Example of #3 (dynamic with ignore patterns):
#   (put your headers in api/ subdirectories)
#   SET(myLibrary_IGNORE_PATTERNS .*RPCMarshall.*)
# 
# Example of #1
#   (put your headers in api/ subdirectories, and do nothing)
#

# Example of explicit source specification:
# SET(YStockQuote_SOURCES ystockquote.cpp) 

# Declare the library, which does the following:
#  - finds required source and headers based on the means of specifying
#    source files you choose (see above)
#  - creates rules to build static and shared versions of this library
#    (XXX: all them permutations of static libraries on windows??
#          http://www.gamedev.net/reference/articles/article1583.asp)
YBT_BUILD(LIBRARY YStockQuote)
