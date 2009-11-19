#!/usr/bin/env ruby

require '../common'

parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))

version = "0.9.8k"
pkg = "openssl-#{version}"
pkgDir = File.join(topDir, pkg)
buildDir = File.join(topDir, "build")
tarball = "#{pkg}.tar.gz"
md5 = "e555c6d58d276aec7fdc53363e338ab3"
url = "http://openssl.org/source/#{tarball}"

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/openssl/aes.h")
    puts "openssl doesn't need to be built"
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

if $platform == "Windows"
    makeCmd = "nmake -f ms\\nt.mak"
    configureCmd = "perl Configure VC-WIN32"
else
    makeCmd = "make"
    configureCmd = "sh ./config"
end

# build
#
includeDir = File.join(pkgDir, 'include')
libDir = pkgDir
Dir.chdir(pkgDir) do
    $buildTypes.each() do |buildType|
        # configure build
        if $platform == "Darwin"
            extraCFlags = "-I#{includeDir} -L#{libDir}"
            extraCFlags += " #{$darwinCompatLinkFlags}" if $platform == "Darwin"
            extraCFlags += " -g -O0" if buildType == "Debug"
            ENV['BP_EXTRA_CFLAGS'] = extraCFlags
        end

        system("#{configureCmd} no-shared --prefix=#{buildDir}")

        system("ms\\do_masm.bat") if $platform == "Windows"

        # actually build
        system("#{makeCmd}")
        system("#{makeCmd} test")
        system("#{makeCmd} install")
        
        # install static libs
        if $platform == "Windows"
            ["ssleay32", "libeay32"].each() do |l|
                FileUtils.cp("#{buildDir}/lib/#{l}.lib",
                             "#{$libInstallDir}/#{buildType}/#{l}_s.lib",
                             :verbose => $verbose)
            end
        else
            ["libssl", "libcrypto"].each() do |l|
                FileUtils.cp("#{buildDir}/lib/#{l}.a",
                             "#{$libInstallDir}/#{buildType}/#{l}_s.a",
                             :verbose => $verbose)
            end
        end
    end
end
        
# install openssl.cnf 
#
FileUtils.mkdir_p("#{$installDir}/ssl")
if $platform == "Windows"
    FileUtils.cp_r("#{buildDir}/openssl.cnf",
                   "#{$installDir}/ssl",
                   :verbose => true)
else
    FileUtils.cp_r("#{buildDir}/ssl/openssl.cnf",
                   "#{$installDir}/ssl",
                   :verbose => true)
end

# install headers
#
FileUtils.cp_r("#{buildDir}/include/openssl",
               $headerInstallDir,
               :verbose => $verbose)

# install binary
#
exeSuffix = $platform == "Windows" ? ".exe" : ""
FileUtils.cp("#{buildDir}/bin/openssl#{exeSuffix}",
             "#{$binInstallDir}/openssl#{exeSuffix}",
             :verbose => true)

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir)
