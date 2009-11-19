#!/usr/bin/env ruby

require 'common'

begin
    parseArgs()

    topDir = File.dirname(File.expand_path(__FILE__))

    if $rebuild
        puts "Removing previous build..."
        FileUtils.rm_rf("#{topDir}/#{$platform}")
    end

    externals = %w(boost cppunit easylzma icu libarchive openssl yajl npapi mongoose)
    externals.insert(-1, "msft") if $platform == "Windows"
    externals.insert(-1, "libedit") if $platform != "Windows"

    args = ""
    ARGV[0..ARGV.length].each do |arg|
        args += " " + arg
    end
    externals.each do |dir|
        Dir.chdir(dir) do
            puts ""
            puts "**************************************************"
            puts "Building #{dir}..."
            puts ""
            system("ruby build.rb #{args}")
        end 
    end

    if $platform == "Windows"
        # copy 7zip from wintools so that clients can
        # find it in externals/windows/bin
        puts ""
        puts "**************************************************"
        puts "copying 7z..."
        Dir.chdir("#{topDir}/WinTools/bin") do
            Dir.glob("7z.*").each() do |f|
                FileUtils.cp(f, "#{topDir}/#{$platform}/bin/#{f}", :verbose => $verbose)
            end
        end 
    end

    if $clean
        puts "Removing previous build..."
        FileUtils.rm_rf("#{topDir}/#{$platform}")
    end
rescue RuntimeError => err
    puts "Error running build: " + err
end
