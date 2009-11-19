#!/usr/bin/env ruby

# build script for mongoose

require '../common'

# Get command line args
parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))

pkgName = "mongoose"
version = "2.8"
pkg = "#{pkgName}-#{version}"
tarball = "#{pkg}.tgz"
url = "http://mongoose.googlecode.com/files/#{tarball}"
md5 = "b72e937a356d3f3cd80cfe6653f0168d"

pkgDir = File.join(topDir, pkgName)
buildDir = File.join(topDir, "build")
libBaseName = "#{pkgName}_s"
mainHeader = "#{pkgName}.h"

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/#{pkgName}/#{mainHeader}")
    puts "#{pkg} doesn't need to be built"
    exit 0
end

puts "removing previous build artifacts..."
FileUtils.rm_rf(pkgDir)
FileUtils.rm_f(tarball)
FileUtils.rm_rf(buildDir)
exit 0 if $clean

# Fetch distro if needed
#
fetch(tarball, url, md5)

# unpack distro and patch
#
unpack(tarball)
applyPatches(pkgDir)

# build
#
makeCmd = $platform == "Windows" ? "nmake" : "make"
Dir.chdir("#{pkgDir}") do
    $buildTypes.each() do |buildType|
        cflags = "-DNO_CGI -DNO_SSL -DNO_SSI"
        if $platform == "Windows"
            cflags += " -nologo -Os"
            if buildType == "Debug"
                cflags += " -Zi -DDEBUG -D_DEBUG -MTd"
            else 
                cflags += " -DNDEBUG -MT"
            end
            system("#{makeCmd} CL_FLAGS=\"#{cflags}\" msvc")
        else
            cflags += " -Wall"
            if buildType == "Debug"
                cflags += " -g -O0"
            else 
                cflags += " -O2"
            end
            cflags += " #{$darwinCompatLinkFlags}" if $platform == "Darwin"
            ENV['CFLAGS'] = cflags
            if !system("#{makeCmd} unix") || $? != 0
              puts "Build Failed: #{$?}"
              exit 1
            end
        end

        # install static lib
        Dir.glob("*#{libBaseName}.#{$libSuffix}") do |l|
            FileUtils.cp(l, "#{$libInstallDir}/#{buildType}/#{l}",
                         :verbose => true)
        end
    end
end
        
# install headers
#
FileUtils.mkdir_p("#{$headerInstallDir}/#{pkgName}", :verbose => $verbose)
FileUtils.cp_r("#{pkgDir}/#{mainHeader}",
               "#{$headerInstallDir}/#{pkgName}",
               :verbose => true)

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f(tarball)
FileUtils.rm_rf(buildDir)

