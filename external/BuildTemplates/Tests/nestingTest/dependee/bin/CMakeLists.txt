#############################################################
# Binary CMakeLists for the YStockQuote project
#
# Created by Lloyd Hilaiel on Mon May 1 2006.
# Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
#############################################################

######
# this is a CMakeLists.txt file to build myBinary
######

# State the libraries to use headers from and link against.  also
# specify how to link (static or dynamic).
SET (getquote_LINK_SHARED YStockQuote)

# The same source inclusion rules apply as for "libraries" above

# Declare the binary, which does the following:
#  - finds required source and headers in and benath this directory
#  - creates rules to build the binary
YBT_BUILD(BINARY getquote)
