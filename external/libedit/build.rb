#!/usr/bin/env ruby

require '../common'

if $platform == "Windows"
    puts "libedit isn't built for windows"
    exit 0
end

# Get command line args
parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))
pkg = "libedit-20090722-3.0"
tarball = "#{pkg}.tar.gz"
md5 = "379afe3fa302e41fc3cb82ad5c969596"
url = "http://www.thrysoee.dk/editline/#{tarball}"
pkgDir = File.join(topDir, pkg)
buildDir = File.join(topDir, "build")

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/editline/histedit.h")
    puts "libedit doesn't need to be built"
    exit 0
end

puts "removing previous build artifacts..."
FileUtils.rm_rf(pkgDir)
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir)
if $clean
    FileUtils.rm_f(tarball)
    exit 0
end

# Fetch distro if needed
#
fetch(tarball, url, md5)

# unpack distro and patch
#
unpack(tarball)
applyPatches(pkgDir)

# build
#
Dir.chdir(pkgDir) do
    $buildTypes.each() do |buildType|
        if $platform == "Darwin"
            ENV['CFLAGS'] = ENV['CFLAGS'].to_s + $darwinCompatCompileFlags
            ENV['CXXFLAGS'] = ENV['CXXFLAGS'].to_s + $darwinCompatCompileFlags
        end

        if buildType == "Debug"
            ENV['CFLAGS'] = ENV['CFLAGS'].to_s + " -g -O0"
        end

        configstr = "./configure --disable-shared --prefix=#{buildDir}"
        puts "running configure: #{configstr}"
        system(configstr)
        
        system("make")
        system("make test")
        system("make install")
        
        # install static lib
        FileUtils.cp("#{buildDir}/lib/libedit.a",
                     "#{$libInstallDir}/#{buildType}/libedit_s.a",
                     :verbose => $verbose)
    end
end
        
# install headers

FileUtils.mv("#{buildDir}/include/histedit.h", "#{buildDir}/include/editline/histedit.h")
FileUtils.cp_r("#{buildDir}/include/editline",
               "#{$headerInstallDir}",
               :verbose => true)

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir)

