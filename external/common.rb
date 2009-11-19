#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
require 'pathname'
require 'open-uri'
require 'digest'
require 'digest/md5'
include Config

topDir = File.dirname(File.expand_path(__FILE__))

# Figure out what platform we're on and set any platform specific stuff
#
$patchCmd = "patch"
$libSuffix = "a"
if CONFIG['arch'] =~ /mswin/
    $platform = "Windows"
    $sevenZCmd = "#{topDir}\\WinTools\\bin\\7z.exe"
    $patchCmd = "#{topDir}\\WinTools\\bin\\patch.exe"
    $libSuffix = "lib"
    $cmakeGenerator = "-G \"Visual Studio 9 2008\""

    if $platform == "Windows"
        rv = system("devenv /? > devenv.out 2>&1")
        FileUtils.rm_f("devenv.out")
        if !rv
            puts ""
            puts "******************************************"
            puts "devenv not in search path, run vsvars.bat"
            puts "******************************************"
            puts ""
            exit -1
        end
    end

elsif CONFIG['arch'] =~ /darwin/
    $platform = "Darwin"

    # Compiler/linker flags needed for 10.4 compatibility.  The surrounding
    # spaces are important, don't be tempted to remove them.
    #
    # 10.4 compatibility is painful, see 
    # http://developer.apple.com/releasenotes/Darwin/SymbolVariantsRelNotes/index
    # In general, we must get these flags to the compiler and linker to tell it
    # what sdk to use.  In addition, source which defines any of the preprocessor
    # symbols mentioned in the above article will be problematic.
    #
    $darwinCompatCompileFlags = " -isysroot /Developer/SDKs/MacOSX10.4u.sdk "
    $darwinCompatCompileFlags += " -mmacosx-version-min=10.4 "
    $darwinCompatLinkFlags = $darwinCompatCompileFlags
    $darwinCompatCompileFlags += " -arch i386 "
    if CONFIG['arch'] !~ /darwin8/
        # this flag only exists on 10.5 and later
        $darwinCompatLinkFlags += " -syslibroot,/Developer/SDKs/MacOSX10.4u.sdk "
    end
    # configure based stuff often uses LDFLAGS and CFLAGS
    ENV['LDFLAGS'] = ENV['LDFLAGS'].to_s + $darwinCompatLinkFlags
    ENV['CFLAGS'] = ENV['CFLAGS'].to_s + $darwinCompatCompileFlags

    # xcode uses SDKROOT
    ENV['SDKROOT'] = "/Developer/SDKs/MacOSX10.4u.sdk"
    $cmakeGenerator = ""

    # make sure we're using gcc 4.0
    ENV['CC'] = 'gcc-4.0'
    ENV['CXX'] = 'g++-4.0'
elsif CONFIG['arch'] =~ /linux/
    $platform = "Linux"
    $cmakeGenerator = ""
else
    puts "unsupported platform: #{CONFIG['arch']}"
    exit -1
end

# Common target dirs
#
$installDir = File.join(topDir, $platform)
$headerInstallDir = File.join($installDir, "include")
$libInstallDir = File.join($installDir, "lib")
$binInstallDir = File.join($installDir, "bin")

# make sure target dirs exist
#
FileUtils.mkdir_p($installDir)
FileUtils.mkdir_p($headerInstallDir)
FileUtils.mkdir_p($libInstallDir)
FileUtils.mkdir_p("#{$libInstallDir}/Debug")
FileUtils.mkdir_p("#{$libInstallDir}/Release")
FileUtils.mkdir_p($binInstallDir)

# Spit out a usage clause and exit
#
def usage()
    puts "usage: ruby build.rb [verbose] [clean] [rebuild] "
    puts "           [keepSource] [-buildType=[debug|release|all]"
    puts "       build.rb clean will just clean artifacts and exit"
    exit -1
end

# Parse command line arguments, setting globals accordingly
#
$clean = false
$rebuild = false
$buildTypes = ["Debug", "Release"]
$verbose = false
$keepSource = false
def parseArgs()
    ARGV[0..ARGV.length].each do |arg|
        if arg == "help"
            usage()
        elsif arg == "verbose"
            $verbose = true
        elsif arg == "clean"
            $clean = true
        elsif arg == "rebuild"
            $rebuild = true
        elsif arg == "keepSource"
            $keepSource = true
        else
            v = arg.split('=')
            if v.length == 2
                case v[0]
                when '-buildType'
                    case v[1]
                    when 'debug'
                        $buildTypes = ["Debug"]
                    when 'release'
                        $buildTypes = ["Release"]
                    when 'all'
                        $buildTypes = ["Debug" "Release"]
                    else
                        usage()
                    end
                else
                    usage()
                end
            end
        end
    end
end


def buildNeeded?(path)
    return $clean || $rebuild || !File.exist?(path)
end

# Fetch from "url" and put result into "tarball"
#
def fetch(tarball, url, md5)
    if !File.exist?(tarball)
        puts "fetching from #{url}"
        puts "progress messages should appear..."
        perms = $platform == "Windows" ? "wb" : "w"
        totalSize = 0
        lastPercent = 0
        interval = 5
        f = File.new(tarball, perms)
        f.write(open(url,
                     :content_length_proc => lambda {|t|
                         if (t && t > 0)
                             totalSize = t
                             STDOUT.printf("expect %d bytes, percent downloaded: ",
                                           totalSize)
                             STDOUT.flush
                         else 
                             STDOUT.print("unknown size to download: ")
                         end
                     },
                     :progress_proc => lambda {|s|
                         if (totalSize > 0)
                             percent = ((s.to_f / totalSize) * 100).to_i
                             if (percent/interval > lastPercent/interval)
                                 lastPercent = percent
                                 STDOUT.printf("%d ", percent)
                                 STDOUT.printf("\n") if (percent == 100)
                             end
                         else
                             STDOUT.printf(".")
                         end
                         STDOUT.flush
                     }).read)
        f.close()
        s = File.size(tarball)
        if (s == 0 || (totalSize > 0 && s != totalSize))
            puts "download failed"
            FileUtils.rm_f(tarball)
            exit 1
        end

        # now let's check the md5 sum
        calculated_md5 = Digest::MD5.hexdigest(File.open(tarball, "rb") {
                                                   |f| f.read
                                               })
        if calculated_md5 != md5
            puts "md5 mismatch, tarball is bogus, delete and retry"
            puts "(got #{calculated_md5}, wanted #{md5})"
            exit 1
        else
            puts "md5 validated! (#{calculated_md5} == #{md5})"
        end
    end
end


# Unpack a distro
#
def unpack(path)
    puts "unpacking #{path}..."
    if path =~ /.tar./
        if $platform == "Windows"
            system("#{$sevenZCmd} x #{path}")
            tarPath = path[0, path.rindex('.')]
            system("#{$sevenZCmd} x #{tarPath}")
            FileUtils.rm_f(tarPath)
        else
            if File.extname(path) == ".bz2"
                system("bzcat #{path} | tar xf -")
            elsif File.extname(path) == ".gz"
                system("tar xzf #{path}")
            else
                puts "unrecognized format for #{path}"
                exit -1
            end
        end
    elsif path =~ /.tgz/
        if $platform == "Windows"
            system("#{$sevenZCmd} x #{path}")
            tarPath = File.basename(path, ".*") + ".tar"
            puts "untarring #{tarPath}..."
            system("#{$sevenZCmd} x #{tarPath}")
            FileUtils.rm_f(tarPath)
        else
            system("tar xzf #{path}")
        end
    elsif path =~ /.zip/
        if $platform == "Windows"
            system("#{$sevenZCmd} x #{path}")
        else
            system("tar xzf #{path}")
        end
    else
        puts "unrecognized format for #{path}"
        exit -1
    end
end


# Apply any patches.  Patches must be named "xxx_{win32|osx}.patch"
# Patches named xxx_all.patch will be applied to all platforms.
# Patches should be created from within parent of "dir.orig" and "dir" via:
#   ..\WinTools\bin\diff -PNaur dir.orig dir > xxx_{win32|osx|all}.patch
#
def applyPatches(pkgDir)
    Dir.chdir(pkgDir) do
        patchSel = ""
        if $platform == "Windows"
            patchSel = [ "_win32" ]
        elsif $platform == "Darwin"
            patchSel = ["_osx", "_unix"]
        elsif $platform == "Linux"
            patchSel = ["_linux", "_unix"]
        else
            puts "applyPatches() not yet supported for #{CONFIG['arch']}"
            exit -1
        end
        (patchSel + ["_all"]).each() do |sel|
            Dir.glob("../*#{sel}.patch") do |p|
                puts "applying patch #{p}"
                system("#{$patchCmd} -p1 < #{p}")
            end
        end
    end
end
