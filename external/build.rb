#!/usr/bin/env ruby

# XXX when 10.4 support dropped, get rid of this logic
if ARGV.length == 1 && ARGV[0] == "osx10.4"
  puts '*** DOING OSX10.4 BUILD **'
  ENV['BP_OSX_TARGET'] = '10.4'
else
  ENV['BP_OSX_TARGET'] = ''
end

require "./bakery/ports/bakery"

topDir = File.dirname(File.expand_path(__FILE__));
$order = {
  :output_dir => File.join(topDir, "dist"),
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
  :verbose => true,
  :use_source => {
    "npapi" => File.join(topDir, "npapi"),
    "wtl" => File.join(topDir, "wtl"),
    "yui-compressor" => File.join(topDir, "yui-compressor")
  },
  :use_recipe => {
    "npapi" => File.join(topDir, "npapi", "recipe.rb"),
    "wtl" => File.join(topDir, "wtl", "recipe.rb"),
    "yui-compressor" => File.join(topDir, "yui-compressor", "recipe.rb")
  }
}

b = Bakery.new $order
b.build
