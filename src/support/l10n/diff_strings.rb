#!/usr/bin/env ruby
#
# A script to generate a concise diff between two strings.json files
#
# 1. parse lhs & generate set of lhs
# 2. parse rhs & generate set of rhs
# 2. rip through all locales and nested keys, building up a complete
#    set of nested hashes
# 3. 

require "pp"
require 'tmpdir'

def mytimestamp time = Time.now
  usec = "#{ time.usec }"
  usec << ('0' * (6 - usec.size)) if usec.size < 6
  time.strftime('%Y-%m-%d %H:%M:%S.') << usec
end

def mytmpnam dir = Dir.tmpdir, seed = File::basename($0)
  pid = Process.pid
  path = "%s_%s_%s_%d" %
    [seed, pid, mytimestamp.gsub(/\s+/o,'_'), rand(101010)]
  f = File::join(dir, path)
  (File.exist?(f) ? tmpnam() : f)
end

$:.push(File.dirname(File.expand_path(__FILE__)))
require "json/json.rb"

def usage
  puts "#{$0}: diff two strings.json files"
  puts ""
  puts "Usage: #{$0} <lhs.json> <rhs.json>"
  exit 1    
end

usage() if ARGV.length != 2

lhsPath = ARGV[0]
rhsPath = ARGV[1]

# 1 parse strings.json
lhsPath = File.join(File.dirname(File.expand_path(__FILE__)),
                    lhsPath)
throw "missing strings.json: #{stringsJson}" if !File.file? lhsPath

def parseFile(jsonPath, tmpfile)
  fullData = JSON.parse(File.read(jsonPath))

  arr = Array.new
  # 2. rip through all locales and nested keys, building up a complete
  #    set of nested hashes
  fullData.each { |locale, data|
    data.each { |toplevel,middle|
      middle.each {|k,v|
        arr.push "#{locale}.#{toplevel}.#{k} = #{v}"
      }
    }
  }
  arr.sort!
  File.open(tmpfile, "w") { |f|
    arr.each{ |x| f.puts x }
  }
end

lhsTemp = mytmpnam
parseFile lhsPath, lhsTemp
rhsTemp = mytmpnam
parseFile rhsPath, rhsTemp
system("diff -uw #{lhsTemp} #{rhsTemp}")
system("rm -f #{lhsTemp} #{rhsTemp}")
