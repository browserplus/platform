#!/usr/bin/env ruby

require "../common"

# Get command line args
parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))

version = "1.12.1"
pkg = "cppunit-#{version}"
pkgDir = File.join(topDir, pkg)
tarball = "#{pkg}.tar.gz"
md5 = "bd30e9cf5523cdfc019b94f5e1d7fd19"
url = "http://prdownloads.sourceforge.net/cppunit/#{tarball}?download"

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/cppunit/Asserter.h")
    puts "cppunit doesn't need to be built"
    exit 0
end

# Note that doze build occurs in pkgDir
buildDir = $platform == "Windows" ? pkgDir : File.join(topDir, "build")

# Start with a clean slate
#
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

# Unpack distro and patch.
#
unpack(tarball)
applyPatches(pkgDir)

# Do the build.
#
Dir.chdir(pkgDir) do
    libPrefix = $platform == "Windows" ? "" : "lib"
    $buildTypes.each() do |buildType|
        puts "Building #{buildType} bits..."
        if $platform != "Windows"
            ldflags = ""
            cflags = buildType == "Debug" ? "-g -O0" : ""
            if $platform == "Darwin"
                cflags += $darwinCompatCompileFlags
            end
            configCmd = "./configure --enable-static --disable-shared"
            configCmd += " --prefix=#{buildDir} --disable-doxygen"
            configCmd += " CXXFLAGS=\"#{cflags}\""
            if $platform == "Darwin"
                configCmd += " LDFLAGS=\"#{$darwinCompatLinkFlags}\""
            end
            system(configCmd)
            system("make")
            system("make check")
            system("make install")
        else
            Dir.chdir("src") do
                system("devenv CppUnitLibraries.sln /build \"#{buildType} static\"")
            end
        end

        # install static lib
        puts "installing #{buildType} static library..."
        libTrailer = $platform == "Windows" && buildType == "Debug" ? "d" : ""
        FileUtils.cp("#{buildDir}/lib/#{libPrefix}cppunit#{libTrailer}.#{$libSuffix}",
                     "#{$libInstallDir}/#{buildType}/#{libPrefix}cppunit.#{$libSuffix}",
                     :verbose => $verbose)
    end

    # install headers
    puts "installing headers..."
    FileUtils.rm_rf("#{$headerInstallDir}/cppunit", :verbose => $verbose)
    FileUtils.cp_r("#{buildDir}/include/cppunit",
                   "#{$headerInstallDir}/cppunit", 
                   :verbose => $verbose)
end

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir) if $platform != "Windows"

