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
