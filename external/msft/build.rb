#!/usr/bin/env ruby

require '../common'

if $platform != "Windows"
    puts "msft is only built on Windows"
    exit 0
end

# Get command line args
parseArgs()

topDir = File.dirname(File.expand_path(__FILE__))

subdirs = %(wtl)

args = ""
ARGV[0..ARGV.length].each do |arg|
    args += " " + arg
end
subdirs.split($/).each() do |dir|
    Dir.chdir(dir) do
        puts "**************************************************"
        puts "Building #{dir}..."
        puts ""
        system("ruby build.rb #{args}")
    end 
end
    

