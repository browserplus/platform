#!/usr/bin/env ruby

require '../common'

parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))

pkg = "icu"
version = "4_0_1"
tarballSuffix = $platform == "Windows" ? "zip" : "tgz"
tarball = "#{pkg}4c-#{version}-src.#{tarballSuffix}"
md5 = $platform == "Windows" ? "ca082eee2ca1a510243be1521ae76271" : "2f6ecca935948f7db92d925d88d0d078"
url = "http://download.icu-project.org/files/icu4c/4.0.1/#{tarball}"

pkgDir = File.join(topDir, pkg)
buildDir = ""
if ($platform == "Windows")
    buildDir = File.join(topDir, "icu")
else
    buildDir = File.join(topDir, "build")
end

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/icu/unicode/basictz.h")
    puts "icu doesn't need to be built"
    exit 0
end

# Start with a clean slate
#
puts "removing previous build artifacts..."
FileUtils.rm_rf(pkgDir)
FileUtils.rm_rf(buildDir)
if $clean
    FileUtils.rm_f(tarball)
    exit 0
end

# fetch distro if needed
#
fetch(tarball, url, md5)

# unpack distro and patch
#
unpack(tarball)
applyPatches(pkgDir)

# Use our smaller datafile instead of the one in the distro.
#
puts "substituting BP data file..."
FileUtils.cp("#{topDir}/icudt40l.dat", 
             "#{pkgDir}/source/data/in", 
             :verbose => $verbose)

# static libs we will install
#
libsToInstall = []
if ($platform == "Windows") 
    libsToInstall = %w(icudt_s.lib icuuc_s.lib icuin_s.lib)
else
    libsToInstall = %w(libsicudata.a libsicuuc.a libsicui18n.a)
end

# build
#
if $platform == "Windows"
    $buildTypes.each() do |buildType|
        puts "building #{buildType}, output is in make_out.txt..."
        #   ENV['PATH'] = "#{ENV['PATH']};#{topDir}/icu/bin"
        system("devenv icu\\source\\allinone\\allinone.sln /build #{buildType} > make_out.txt")

        # install static libs
        libsToInstall.each do |l|
            # debug builds may have 'd' on the lib name
            src = l
            if buildType == "Debug" && !File.exists?("#{buildDir}/lib/#{src}")
                src = l.sub("_s.lib", "d_s.lib")
            end
            FileUtils.install("#{buildDir}/lib/#{src}",
                              "#{$libInstallDir}/#{buildType}/#{l}",
                              :verbose => $verbose)
        end
    end
else
    Dir.chdir("icu/source") do
        # configure
        system("chmod +x runConfigureICU configure install-sh")
        $buildTypes.each() do |buildType|
            plat = (($platform == "Darwin") ? "MacOSX" : $platform)
          
            configCmd = "./runConfigureICU #{plat} --prefix=#{buildDir} --enable-static"
            configCmd += " --disable-icuio --disable-layout"

            if buildType == "Debug"
                configCmd += " --enable-debug --disable-release"
            end

            if $platform == "Darwin"
                ENV['CFLAGS'] = ENV['CFLAGS'].to_s + $darwinCompatCompileFlags
                ENV['CXXFLAGS'] = ENV['CXXFLAGS'].to_s + $darwinCompatCompileFlags
            end

            if buildType == "Debug"
                ENV['CFLAGS'] = ENV['CFLAGS'].to_s + " -g -O0"
            end

            system(configCmd)

            # build
            system("make")
            
            # test
            system("make check")
            
            # install to buildDir
            system("make install")

            # install static libs
            libsToInstall.each do |l|
                FileUtils.install("#{buildDir}/lib/#{l}",
                                  "#{$libInstallDir}/#{buildType}/#{l}",
                                  :verbose => $verbose)
            end
        end
    end
end

# install headers
puts "installing headers..."
FileUtils.mkdir_p("#{$headerInstallDir}/icu")
FileUtils.cp_r("#{buildDir}/include/unicode",
               "#{$headerInstallDir}/icu/",
               :verbose => $verbose)

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_rf(buildDir)
FileUtils.rm_f("make_out.txt")
