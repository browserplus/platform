#!/usr/bin/env ruby

require "../common"

# Get command line args
parseArgs()

# don't use 2.7.0, it's broken
version = "2.6.2"

topDir = File.dirname(File.expand_path(__FILE__))
buildDir = File.join(topDir, "build")
pkg = "libarchive-#{version}"
pkgDir = File.join(topDir, pkg)
tarball = "#{pkg}.tar.gz"
md5 = "e31fcacd3f2b996988c0852a5edfc680"
url = "http://libarchive.googlecode.com/files/#{tarball}"

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/libarchive/archive.h")
    puts "libarchive doesn't need to be built"
    exit 0
end

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

# unpack distro and patch
#
unpack(tarball)
applyPatches(pkgDir)

def installLibs(buildDir, buildMode) 
    puts "Installing #{buildMode} libraries..."
    FileUtils.mkdir_p("#{$libInstallDir}/#{buildMode}")
    if $platform == "Windows"
        debug_str = buildMode == "Debug" ? "-d" : ""
        FileUtils.cp("#{buildDir}/lib/libarchive-vc90-mt#{debug_str}.lib",
                     "#{$libInstallDir}/#{buildMode}/archive_s.lib", 
                    :verbose => $verbose)
    else
        FileUtils.cp("#{buildDir}/lib/libarchive.a",
                     "#{$libInstallDir}/#{buildMode}/libarchive_s.a",
                     :verbose => $verbose)
    end
end


def installHeaders(headerDir) 
    puts "Installing headers..."
    FileUtils.mkdir_p("#{$headerInstallDir}/libarchive")
    FileUtils.cp("#{headerDir}/archive.h",
                 "#{$headerInstallDir}/libarchive", 
                 :verbose => $verbose)
    FileUtils.cp("#{headerDir}/archive_entry.h",
                 "#{$headerInstallDir}/libarchive", 
                 :verbose => $verbose)
end


Dir.chdir(pkgDir) do
    $buildTypes.each() do |buildType|
        puts "Building #{buildType} bits..."
        if $platform == "Windows"
            system("devenv windows\\vc90\\libarchive.sln /Build #{buildType}")
        else
            if $platform == "Darwin"
                ENV['CFLAGS'] = ENV['CFLAGS'].to_s + $darwinCompatCompileFlags
                ENV['CXXFLAGS'] = ENV['CXXFLAGS'].to_s + $darwinCompatCompileFlags
            end
            if buildType == "Debug"
                ENV['CFLAGS'] = ENV['CFLAGS'].to_s + " -g -O0"
            end

            configCmd = "./configure --without-lzmadec --without-bz2lib"
            configCmd += " --without-zlib --disable-shared --disable-bsdcpio"
            configCmd += " --disable-bsdtar --enable-static --prefix=#{buildDir}"
            system(configCmd)
            system("make")
            system("make test")
            system("make install")
        end
        if $platform == "Windows"
            installLibs(pkgDir, buildType)
        else 
            installLibs(buildDir, buildType)
        end
    end
end

if $platform == "Windows"
    installHeaders("#{pkgDir}/libarchive")
else
    installHeaders("#{buildDir}/include")
end

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir)

