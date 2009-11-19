#############################################################
# Samples CMakeLists for the YStockQuote project
#
# Created by Lloyd Hilaiel on Mon May 1 2006.
# Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
#############################################################

######
# this is a CMakeLists.txt file to build mySample
######

# State the libraries to use headers from and link against.  also
# specify how to link (static or dynamic).
SET (sampleStockQuoteFetcher_LINK_SHARED YStockQuote)

# sources are implicitly defined.

# Declare the sample, which does the following:
#  - finds required source and headers in and benath this directory
#  - creates rules to build the sample program
#  - schedules source files for installation in your dist/ directory
YBT_BUILD(SAMPLE sampleStockQuoteFetcher)
