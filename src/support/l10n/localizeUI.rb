#!/usr/bin/env ruby -s
#
# A script to take a directory as input and localize it to multiple output
# directories.  Functional overview is thus: 
#
# 1. parse strings.json right next to this script
# 2. if src/common dir exists, copy it into output directory
# 3. create one directory in output per supported locale (determined from
#    strings.json)  
# 4. for each src/<locale> directory, copy contained files into output
#    locale directory
# 5. if src/base exists, copy all contents into each supported locale
#    directory while providing input strings from strings.json for
#    substitution if files already exist, no copying occurs

require "yaml"
require "erb"
require "fileutils"

# modify load path to find json.rb
begin
  # json is built in in ruby 1.9
  require 'json'
rescue LoadError
  $:.push(File.dirname(File.expand_path(__FILE__)))
  require "json/json.rb"
end

if ARGV[0] =~ /help/ || $indir == nil || $outdir == nil
  puts "Usage: ruby -s #{$0} [-verbose=true] -indir=<input dir> -outdir=<output dir>"
  exit 1
end

begin
  raise "indir doesn't exist: #{$indir}" if !File.directory? $indir
  raise "outdir already exists: #{$outdir}" if File.directory? $outdir

  # 1 parse strings.json
  stringsJson = File.join(File.dirname(File.expand_path(__FILE__)), "strings.json")
  puts "parsing strings.json: #{stringsJson}" if ($verbose) 
  raise "missing strings.json: #{stringsJson}" if !File.file? stringsJson
rescue Exception => x
  puts "localizeUI Error: #{x}"
  exit 1
end

data = JSON.parse(File.read(stringsJson))

# 1.5 create output dir
FileUtils.mkdir($outdir)

# 2 copy common dir
commonDir = File.join($indir, "common")
if (File.directory? commonDir)
  puts "copied common/ directory" if ($verbose) 
  FileUtils.cp_r(commonDir, File.join($outdir, "common"))
else
  puts "no common/ directory in source, not copying" if ($verbose) 
end

# 3 create one destination directory per locale
data.each { |x,y|
  localeOutDir = File.join($outdir, x) 

  FileUtils.mkdir(localeOutDir)
  puts "creating locale directory #{x}/" if ($verbose) 
  
  # 4. for each src/<locale> directory, copy contained files into output
  #    locale directory
  Dir.glob(File.join($indir, x, "*")) do |i|
    outFile = File.basename(i)
    puts "copying #{i} to output dir #{x}/#{outFile}" if ($verbose)
    FileUtils.install(i, File.join(localeOutDir, outFile),
                      :mode => 0644, :verbose => $verbose)
  end
}

# a simple class which allows us to set up bindings for template execution
class Hash
  def method_missing(meth,*args)
    if /=$/=~(meth=meth.id2name) then
      self[meth[0...-1]] = (args.length<2 ? args[0] : args)
    else
      self[meth]
    end
  end
end

class TemplateContext
  def get_binding
    @b
  end
  
  def populate x
    @b = binding
    x.each {|i,j|
      defineTopLevel i, j, @b, x
    }
  end

  def defineTopLevel name, value, binding, localeData
    if (value.is_a? Hash)
      eval("#{name} = Hash.new", binding)
      value.each {|i,j|
        eval("#{name}['#{i}'] = <<ENDLOCALIZEDSTRING\n#{j}\nENDLOCALIZEDSTRING\n#{name}['#{i}'].chomp!\n", binding)
      }
    else
      eval("#{name} = <<ENDLOCALIZEDSTRING\n#{j}\nENDLOCALIZEDSTRING\n#{name}.chomp!\n", binding)
    end
  end
end

# 5. if src/base exists, copy all contents into each supported locale
#    directory while providing input strings from strings.json for
#    substitution if files already exist, no copying occurs
baseInDir = File.join($indir, "base")

Dir.glob(File.join(baseInDir, "*")) do |f|
  if (f =~ /\.js$/ || f =~ /\.css$/ || f =~ /\.html$/)
    puts "substituting #{f} into output directories" if ($verbose)
    data.each { |x,y|
      localeOutDir = File.join($outdir, x) 
      # now we must set perform ERB substitution
      t = ERB.new(File.read(f))
      tc = TemplateContext.new
      tc.populate y
      File.open(File.join(localeOutDir, File.basename(f)), "w") { |o|
        o.write t.result(tc.get_binding)
      }
    }
  else
    puts "installing #{f} to output directories" if ($verbose)      
    data.each { |x,y|
      localeOutDir = File.join($outdir, x) 
      FileUtils.install(f, localeOutDir, :mode => 0644, :verbose => $verbose)
    }
  end
end
