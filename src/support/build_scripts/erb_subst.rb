# ruby -s ./erb_subst.rb

require 'fileutils'
require 'rbconfig'
include Config
require 'pp'

throw "I require a 'context' command line param!" if !$context
throw "context file doesn't exist: #{$context}" if !File.exist?($context)

throw "I require a 'l10n' command line param!" if !$l10n
throw "l10n file doesn't exist: #{$l10n}" if !File.exist?($l10n)

throw "I require a 'output_dir' command line param!" if !$output_dir
throw "output_dir dir doesn't exist: #{$output_dir}" if !File.exist?($output_dir)

require "erb"
$:.push(File.dirname(__FILE__))
begin
  require 'json'
rescue LoadError
  require "json/json.rb"
end

$cmake = eval(File.read($context))
$l10n = JSON.parse(File.read($l10n))

files = []
if $input_file != nil
  throw "input_file doesn't exist: #{$input_file}" if !File.exist?($input_file)
  files.push($input_file)
else
  files = Dir.glob(File.join("**", "*.erb"))
end

lastCMakeRun = File.mtime(File.join($output_dir, "time.stamp"))

files.each { |from|
  to = from.sub(/\.erb/, '')

  # substitute into the the build/ directory, rather than inside
  # source directory.
  to = File.join($output_dir, to)

  if !File.exist?(to) ||
      File.mtime(to) < File.mtime(from) ||
      File.mtime(to) < lastCMakeRun ||
      File.size(to) == 0 || 
      $force
    puts "Generating: #{to}"
    FileUtils.mkdir_p(File.dirname(to))
    File.open(to, "w") { |o|
      t = ERB.new(File.read(from))
      o.write t.result()
    }
  end
}
