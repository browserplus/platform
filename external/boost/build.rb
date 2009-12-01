#!/usr/bin/env ruby

require "../common"

# Get command line args
parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))
version = "1_41_0"
pkg = "boost_#{version}"
tarball = "#{pkg}.tar.bz2"
md5 = "8bb65e133907db727a2a825c5400d0a6"
url = "http://downloads.sourceforge.net/boost/#{tarball}"

pkgDir = File.join(topDir, pkg)
buildDir = File.join(topDir, "build")

# Make a stab at determining if we need to build.
if !buildNeeded?("#{$headerInstallDir}/boost/filesystem.hpp")
    puts "boost doesn't need to be built"
    exit 0
end

# Start with a clean slate
#
puts "removing previous build artifacts..."
FileUtils.rm_rf(pkgDir)
FileUtils.rm_rf(buildDir)
FileUtils.rm_f("#{pkg}.tar")
if $clean
    FileUtils.rm_f(tarball)
    exit 0
end

# Fetch boost distro if needed
#
fetch(tarball, url, md5)

# unpack distro and patch
#
unpack(tarball)
applyPatches(pkgDir)

# All available libraries, result of "./configure --show-libraries"
#   date_time, filesystem, function_types, graph, iostreams, 
#   math, mpi, program_options, python, regex, serialization, 
#   signals, system, test, thread, wave

# Libraries we'll build, "all" to build all.  To build
# a subset, specify as "buildLibs = %w(filesystem regex)"
#
buildLibs = %w(system filesystem)
buildLibs.insert(-1, "regex") if $platform != "Windows"

FileUtils.mkdir_p("#{topDir}/../#{$platform}/include")

# Do the build 
#
Dir.chdir(pkgDir) do 
    if !buildLibs.empty? then
        # first build bjam
        bjamSrcPath = File.join("tools", "jam", "src")
        Dir.chdir(bjamSrcPath) do
            puts "building bjam..."
            if $platform == "Windows"
                system(".\\build.bat")
            else 
                system("./build.sh")
            end
        end

        # now hunt down bjam
        bjamPath = Dir.glob("#{bjamSrcPath}/bin.*/**/bjam*")[0]

        toolset=""
        if $platform == "Darwin"
            toolset="darwin"
            # add a user-config.jam for Darwin to allow 10.4 compatibility
            uconfig = File.new("user-config.jam", "w")
            uconfig.write("# Boost.Build Configuration\n")
            uconfig.write("# Allow Darwin to build with 10.4 compatibility\n")
            uconfig.write("\n")
            uconfig.write("# Compiler configuration\n")
            uconfig.write("using darwin : 4.0 : g++-4.0 -arch i386 : ")
            uconfig.write("<compileflags>\"#{$darwinCompatCompileFlags}\" ")
            uconfig.write("<architecture>\"x86\" ")
            uconfig.write("<linkflags>\"#{$darwinCompatLinkFlags}\" ;\n")
            uconfig.close

        elsif $platform == "Windows"
            toolset="msvc"
        elsif $platform == "Linux"
            toolset="gcc"
        else
            puts "Unsupported platform #{$platform}"
            exit -1
        end
        
        # now use bjam to build 
        baseCmd = "#{bjamPath} toolset=#{toolset} "
        baseCmd += " link=static threading=multi runtime-link=static"
        baseCmd += " --build-dir=#{buildDir} stage"
        if $platform == "Darwin"
            baseCmd += " --user-config=user-config.jam"
        end

        $buildTypes.each() do |buildType|
            FileUtils.rm_rf("stage")
            FileUtils.mkdir_p("#{$libInstallDir}/#{buildType}")
            buildLibs.each() do |l|
                puts "building #{buildType} #{l}..."
                buildCmd = baseCmd + " variant=#{buildType.downcase} --with-#{l}"
                puts buildCmd
                system(buildCmd)
            end

            # copy static libs
            puts "copying #{buildType} libraries..."
            Dir.glob("stage/lib/libboost*.#{$libSuffix}").each do |l|
                target = File.join($libInstallDir, buildType, File.basename(l))
                FileUtils.cp(l, target, :verbose => $verbose)
            end
        end
    end 

    puts "copying headers..."
    FileUtils.cp_r("boost", $headerInstallDir, :verbose => $verbose)
end

puts "cleaning up..."
FileUtils.rm_rf(pkgDir) if !$keepSource
FileUtils.rm_f("#{pkg}.tar")
FileUtils.rm_rf(buildDir)
