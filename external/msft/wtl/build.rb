#!/usr/bin/env ruby

require '../../common'

if $platform != "Windows"
    puts "wtl only built on Windows"
    exit 0
end

# Get command line args
parseArgs()

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/wtl/atlapp.h")
    puts "wtl doesn't need to be built"
    exit 0
end

# build, which is just copying headers
#
FileUtils.mkdir_p("#{$headerInstallDir}/wtl", :verbose => $verbose)
Dir.chdir("include") do
    Dir.glob("*.h") do |h|
        FileUtils.cp(h, "#{$headerInstallDir}/wtl", :verbose => $verbose)
    end
end

