#!/usr/bin/env ruby

#
# A script to generate installers and bpkgs for the BrowserPlus platform.
# The idea is that given one bundle of build artifacts its possible to
# quickly generate different .exe/.dmg/.bpkg packages that are possibly
# skinned and/or parameterized to a specific set of distribution servers 
# and/or signing certificates.
# 
# Possible Inputs:
#   -pubKey    path to public signing key 
#   -privKey   path to private signing key
#   -authKey   path authenticode signing key (only meaningful on win32)
#   -output    path where output should be left (should be a directory)
#   -prefix    filename prefix to append to installer and bpkg
#   -primary   hostname of primary distribution server
#   -secondary hostname of secondary distribution server (may be repeated)
#   

# step 1: copy into build directry
# step 2: sub in distribution servers
# step 3: authenticode sign binaries if required
# step 4: generate bpkg
# step 5: generate "net" installer
# step 6: generate "offline" installer
