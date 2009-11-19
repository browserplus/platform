#!/usr/bin/env ruby

# a mac specific ruby script that builds a distributable dmg image of
# our osx installer.  This script also takes binaries and scripts
# present in the installer/ dir and turns them into a nice .app bundle that
# the end user can just click on.

require 'fileutils'
require 'pathname'

# process some arguments
if ARGV[0] =~ /help/ 
  puts "Usage: ruby -s #{$0} [-outDir=<path>] [-fname=<name of outputted binary>]"
  puts "                     [-verbose=true]"
  exit 1
end

#default argument values
$fname ||= "BrowserPlusInstaller.exe"
$outDir ||= File.dirname(File.expand_path(__FILE__))
instDir = File.dirname(File.expand_path(__FILE__))

# does outdir exist?
if !File.directory? $outDir
  puts "fail: cannot write to output directory: #{$outDir}"
  exit 1
end

# First we'll copy the binary
bootstrapperExe = File.join(instDir, "buildtools",
                            "BrowserPlusBootstrapper.exe")
tgtExe = File.join($outDir, $fname)

if File.exists? tgtExe
  throw "cannot create executable, file already exists: #{tgtExe}" 
end

FileUtils.install(bootstrapperExe, tgtExe, :mode => 0755, :verbose => $verbose)

# now make tgtExe path absolute
puts "tgtPath - #{tgtExe}"
tgtExe = Pathname.new(tgtExe).realpath
puts "tgtExe - #{tgtExe}"

# now we'll need to make a tarfile of the pertinent files
tarFile = "installer.tar"
lzipFile = "#{tarFile}.lz"
FileUtils.cd(instDir) { 
  FileUtils.rm_f(tarFile)
  FileUtils.rm_f(lzipFile)

  tarExePath = File.join("buildtools", "bptar.exe")
  elzmaExePath = File.join("buildtools", "elzma.exe")

  # grab any .bpkg files that have been dropped into this directory
  bpkgFiles = ""
  Dir.glob("*.bpkg") { |x| bpkgFiles += x }

  system("#{tarExePath} c #{tarFile} ui strings.json installer.config BrowserPlus.crt BrowserPlusInstaller.exe #{bpkgFiles}")

  # now let's lzma compress at highest compression
  system("#{elzmaExePath} -9 --lzip #{tarFile}")

  # now we'll append the lzma'd bits to the binary
  puts "opening #{tgtExe} for writing"
  File.open(tgtExe, File::APPEND | File::WRONLY | File::BINARY) { |o|
    sz = 0
    File.open(lzipFile, File::RDONLY | File::BINARY) { |i|
      x = i.read
      sz = x.length
      puts "appending #{sz} bytes"
      o.write x
    }
    # now let's append a little endian integer... 
    o.write [sz].pack("I")

    # and we'll write the end of payload magic string, note, this magic
    # string makes us robust under other processes that can append to the
    # binary, such as signing.
    o.write("Yahoo!BrowserPlusEndOfInstallerPayloadMarker");
  }
}
