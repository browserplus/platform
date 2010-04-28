#!/usr/bin/env ruby
#
# the bootstrapping build.rb.  This file is capable of attaining
# the "bakery", a ports system for software projects, and using it to 
# build the 3rd party source required by BrowserPlus, as specified in 
# the receipe file beside this file. 
#
# to change the commit of the bakery that platform uses, simply
# fiddle this sha256

# BEGIN user serviceable parts

# if you want to update to a newer version of the bakery, fiddle this
# sha256 to point to the commit you want
bakery_commit = "0ac2a658dc05b4596430836f1e59bd9d0930473a"
# END user serviceable parts

require 'rbconfig'
require 'fileutils'
require 'pathname'
require 'open-uri'
require 'timeout'
include Config

# configure thyself
if CONFIG['arch'] =~ /mswin/
    $platform = "Windows"
    $sevenZCmd = "7z.exe"
elsif CONFIG['arch'] =~ /darwin/
    $platform = "Darwin"
elsif CONFIG['arch'] =~ /linux/
    $platform = "Linux"
    $cmakeGenerator = ""
    $cmakeArgs = ""
else
  puts "unsupported platform: #{CONFIG['arch']}"
  exit -1
end

TOPDIR = File.dirname(File.expand_path(__FILE__))
url = "http://github.com/lloyd/bakery/tarball/#{bakery_commit}"
bakeryVersionFile = File.join(TOPDIR, "bakery_version.txt")
bakeryUnpackPath = File.join(TOPDIR, "bakery")

# throughout we'll now we're in external/ dir
Dir.chdir(TOPDIR)

# once the bakery is attained, here's the "recipe" file
def runRecipe
  require File.join(TOPDIR, "bakery/ports/bakery")

  $order = {
    :output_dir => File.join(TOPDIR, $platform),
    :packages => [
                  "boost",
                  "bzip2",
                  "cppunit",
                  "easylzma",
                  "icu",
                  "jsmin",
                  "libarchive",
                  "libedit",
                  "mongoose",
                  "npapi",
                  "openssl", 
                  "wtl",
                  "yajl",
                  "yui-compressor",
                  "zlib"
                 ],
    :use_recipe => {
      "npapi" => "npapi/recipe.rb",
      "wtl" =>   "wtl/recipe.rb",
      "yui-compressor" => "yui-compressor/recipe.rb"
    },
    :use_source => {
      "npapi" => "npapi",
      "wtl"   => "wtl",
      "yui-compressor" => "yui-compressor"

    },
    :verbose => true
  }

  b = Bakery.new $order
  # let's check the bakery state quickly
  s = b.check
  issues = s[:info].length + s[:warn].length +  s[:error].length
  puts "Bakery consistency check complete (#{issues} interesting issues found)"
  s.each { |k,v|
    v.each { |msg|
      puts "#{k.to_s.upcase}: #{msg}"
    }
  }
  raise "refusing to build bakery, in an inconsistent state" if s[:error].length > 0
  b.build

  exit 0
end


# Make a stab at determining if we need to build.
if File.directory? bakeryUnpackPath
  if File.exists? bakeryVersionFile
    commitGot = File.open(bakeryVersionFile, "r") { |f| f.read }.chomp 
    if commitGot == bakery_commit
      puts "...bakery doesn't need to be fetched..."
      # invoke bakery with our recipe
      runRecipe
      exit 0
    end
  end
  FileUtils.rm_f bakeryVersionFile
  FileUtils.rm_rf bakeryUnpackPath
end

def fetch(tarball, url)
  puts "fetching bakery (#{url})"
  puts "progress messages should appear..."
  perms = $platform == "Windows" ? "wb" : "w"
  totalSize = 0
  lastPercent = 0
  interval = 5
  f = File.new(tarball, perms)
  # added by garymd 04/06/2010.
  # grabbing large tarballs (boost) times out on slow net connections like build machines.
  retries = 10
  begin
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
  rescue Timeout::Error
    retries -= 1
    if retries > 0
      sleep 0.42 and retry
    else
      raise
    end
  end
  f.close()
  s = File.size(tarball)
  if (s == 0 || (totalSize > 0 && s != totalSize))
    puts "download failed"
    FileUtils.rm_f(tarball)
    exit 1
  end
end


# Unpack a distro
#
def unpack(path)
  puts "unpacking #{path}..."
  if path =~ /.tgz/
    if $platform == "Windows"
      system("#{$sevenZCmd} x -y #{path}")
      tarPath = File.basename(path, ".*") + ".tar"
      puts "untarring #{tarPath}..."
      system("#{$sevenZCmd} x -y #{tarPath}")
      FileUtils.rm_f(tarPath)
    else
      system("tar xzf #{path}")
    end
  else 
    throw "I don't know how to unpack this: #{path}"
  end
end

# fetch bakery
fetch("bakery.tgz", url)

# unpack bakery
unpack("bakery.tgz")

# rename the bakery 
whackyName = Dir.glob("lloyd-bakery-*")[0]
if whackyName && File.directory?(whackyName)
  FileUtils.rm_rf("bakery.tgz")
  FileUtils.mv(whackyName, bakeryUnpackPath)
  
  # now on winblows, let's go and fixup all .patch files with silly
  # line endings as required by patch.
  if $platform == "Windows"
    Dir.chdir(bakeryUnpackPath) {
      Dir.glob(File.join("**", "*.patch")).each { |f|
        ptch = File.read(f)
        ptch.split("\n").collect { |l| l.sub(/\r$/, "") }.join("\r\n")
        File.open(f, "w+") { |f| f.write ptch }
      }
    }
  end

  # now let's record the version of the bakery that we've successfully
  # unpacked
  File.open(bakeryVersionFile, "w+") { |f|
    f.puts bakery_commit 
  }
else
  throw "oh noes, I can't figure out what we just downloaded!  halp!"
end

# invoke bakery with our recipe
runRecipe
