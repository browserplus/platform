#!/usr/bin/env ruby

require '../common'

# Get command line args
parseArgs()

thisDir = File.dirname(File.expand_path(__FILE__))

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/npapi/npapi.h")
    puts "npapi headers don't need to be copied"
    exit 0
end

puts "removing previous build artifacts..."
FileUtils.rm_rf("#{$headerInstallDir}/npapi")

# install headers
FileUtils.cp_r("#{thisDir}/headers",
               "#{$headerInstallDir}/npapi",
               :verbose => true)

# done!  easy like a sunday morning. 
