#!/usr/bin/env ruby -s
#
# A script to inject strings from strings.json into a service's manifest.
#
# 1. parse strings.json right next to this script
# 2. parse manifest.json from the service
# 3. extract new required keys
# 4. replace strings in parsed manifest.json
# 5. write manifest.json.updated

require "yaml"
require "erb"
require "fileutils"

# modify load path to find json.rb
$:.push(File.dirname(File.expand_path(__FILE__)))
require "json/json.rb"

if ARGV[0] =~ /help/ || $path == nil || $service == nil
  puts "A script to inject strings for services into strings.json"
  puts "Usage: ruby -s #{$0} [-verbose=true] -service=<name of service> -path=<path to service>"
  exit 1
end

puts "injecting strings for service: #{$name}" if $verbose

raise "service dir doesn't exist: #{$path}" if !File.directory? $path

begin
  # 1 parse strings.json
  stringsJson = File.join(File.dirname(File.expand_path(__FILE__)), "strings.json")
  puts "parsing strings.json: #{stringsJson}" if ($verbose) 
  raise "missing strings.json: #{stringsJson}" if !File.file? stringsJson
rescue Exception => x
  puts "Error: #{x}"
  exit 1
end

data = JSON.parse(File.read(stringsJson))

# 2 parse manifest.json
manifest = File.join($path, "manifest.json")

if !File.file? manifest
  puts "service manifest doesn't exist: #{manifest}"
  exit 1
end

manifestData = JSON.parse(File.read(manifest))

# 3 extract new required keys


kTitle = "#{$service}::title"
kSummary = "#{$service}::summary"

newLocalizations = Hash.new

data.each { |locale,d|
  next if !d.has_key? 'services'
  if data[locale].has_key? 'services'
    t = nil
    s = nil
    if data[locale]['services'].has_key? kTitle
      t = data[locale]['services'][kTitle]
    end
    if data[locale]['services'].has_key? kSummary
      s = data[locale]['services'][kSummary]
    end

    if t && s
      newLocalizations[locale] = {
        'summary' => s,
        'title' => t
      }
    end
  end
}

if newLocalizations.size > 0
  manifestData['strings'] = newLocalizations

  s = JSON.generator::State.new
  s.configure({ :indent => "    ", :array_nl => "\n", :object_nl => "\n"})
  File.open(File.join($path, "manifest.json.updated"), 'w') { |f|
    f.write(manifestData.to_json(s))
  }
  puts "manifest.json.updated written with new data"
end
