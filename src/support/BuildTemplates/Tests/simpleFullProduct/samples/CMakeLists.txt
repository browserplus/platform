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
