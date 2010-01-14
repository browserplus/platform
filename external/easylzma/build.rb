#!/usr/bin/env ruby

require '../common'

# Get command line args
parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))
buildDir = File.join(topDir, "build")

version = "0.0.7"
pkg = "easylzma-#{version}"
pkgDir = File.join(topDir, pkg)
tarball = "#{pkg}.tar.gz"
md5 = "3e42cbf52a398ccb0535e4b428686707"
url = "http://github.com/lloyd/easylzma/tarball/#{version}"

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/easylzma/common.h")
    puts "easylzma doesn't need to be built"
    exit 0
end

# Start with a clean slate
#
puts "removing previous build artifacts..."
FileUtils.rm_f("pax_global_header")
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

# unpack distro
#
unpack(tarball)

# since tarball came from github, it unpacks into a whacky name
whackyName = Dir.glob("lloyd-easylzma-*")[0]
if File.exists?(whackyName)
    FileUtils.rm_rf(pkgDir)
    FileUtils.mv(whackyName, pkgDir)
end

# apply patches
#
applyPatches(pkgDir)

# build
#
FileUtils.mkdir_p(buildDir)
Dir.chdir(buildDir) do
    $buildTypes.each() do |buildType|
    system("cmake #{$cmakeArgs} #{$cmakeGenerator} -DCMAKE_BUILD_TYPE:STRING=#{buildType} #{pkgDir}")

        if $platform == "Windows"
            system("devenv easylzma.sln /Build #{buildType}")
        else
            system("make")
        end

        # copy static lib
        libName = ""
        if $platform == "Windows"
            libName = File.join(buildDir, pkg, "lib", buildType, "easylzma_s.lib")
        else
            libName = File.join(buildDir, pkg, "lib", "libeasylzma_s.a")
        end
        FileUtils.mkdir_p("#{$libInstallDir}/#{buildType}", :verbose => true)
        FileUtils.cp(libName, "#{$libInstallDir}/#{buildType}", :verbose => true)
    end
end

# copy headers 
FileUtils.mkdir_p("#{$headerInstallDir}/easylzma")
Dir.glob(File.join(buildDir, pkg, "include", "easylzma", "*.h")) do |x|
    FileUtils.cp(x, "#{$headerInstallDir}/easylzma", :verbose => true)
end

# and command line tools
FileUtils.mkdir_p($binInstallDir)
Dir.glob(File.join(buildDir, pkg, "bin", "*")) do |x|
  FileUtils.cp(x, $binInstallDir, :verbose => true)
end

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir)
