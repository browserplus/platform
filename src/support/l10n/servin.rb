#!/usr/bin/env ruby -s
require 'fileutils'
require 'webrick'
include WEBrick

# Development mode script that web serves (def: port 3003) the generated output from 
# localizeUI.rb.  Anytime either L10n/strings.json or the "indir" is modified, the files
# are regenerated.

# Set include path for local files
$:.push(File.dirname(File.expand_path(__FILE__)))
require "watcher/filesystemwatcher"

#
# Watcher
#
class Watcher

  # CTR
  def initialize(localizeUI, path, port)
    @localizeUI = localizeUI
    @port = port
    @path = path
  end

  # (Re)generate all of the localized files.  Method is called at startup and everytime
  # a file changes in strings.json or $indir.
  def generate()
    FileUtils.rm_rf($outdir)
    puts "Generating localized files in #{$outdir}."
    system(@localizeUI)
    exit 1 if $?.exitstatus != 0
  end

  # Starts Webrick and opens Safari. Called from "main".
  def start_server()  
    puts "==== Web server starting on port #{@port} ==="
    server = HTTPServer.new(
      :Port            => @port,
      :DocumentRoot    => $outdir
    )

    ['INT','TERM'].each { |signal|
      trap(signal) {
        puts "SINGAL SHUTDOWN"
        server.shutdown
      }
    }

    Thread.new(server) do
      server.start
      # start returns after ctrl-c
      puts "Done."
      exit 0
    end
  end

  # Use filesystemwatcher to monitor relavant files.  Everytime a file is changed
  # ( CREATED / MODIFIED / DELTETED ), regenerate the localized files.
  def watch()

    watcher = FileSystemWatcher.new
    watcher.addDirectory($indir)
    watcher.addFile("#{@path}/strings.json")
    
    puts "Watching '#{$indir}' and '#{@path}/strings.json'"

    watcher.sleepTime = 1
    watcher.start { |status,file|
      if (status == FileSystemWatcher::CREATED || 
          status == FileSystemWatcher::MODIFIED || 
          status == FileSystemWatcher::DELETED) then
          
        # file in indir change, regenerate outdir
        generate()
      end
    }

    watcher.join()
  end
end

#--- main program ----
if __FILE__ == $0
  if ARGV[0] =~ /help/ || $indir == nil || $outdir == nil
    puts "Usage: ruby -s #{$0} -indir=<input dir> -outdir=<output dir> [-port=3003] [-watch]"
    exit 1
  end

  # If outdir exists, give the user an option to delete it.  Because that directory 
  # is going to be automatically deleted continuously during the execution of this
  # script, we want to the user to VERIFY the directory to be deleted.
  if File.directory? $outdir
    print "Output directory #{$outdir} already exists.  Delete it (y/n)? "
    STDOUT.flush
    ans = gets.chomp
    if ans == 'y'
      puts "OK, deleting it."
    else
      puts "Exiting.  Note that the directory must be deleted to run this script."
    end
  end
  
  # default port
  port = $port || 3003

  # path to *this* directory
  path = File.dirname(File.expand_path(__FILE__))
  localizeUI = "#{path}/localizeUI.rb -indir=\"#{$indir}\" -outdir=\"#{$outdir}\""

  w = Watcher.new(localizeUI, path, port)
  w.generate
  if $watch == nil
    w.start_server
    sleep 0.5
    system("open -a Safari \"http://localhost:#{port}/en-US/\"")
  else 
    ['INT','TERM'].each { |signal|
      trap(signal) {
        puts "Done."
        exit 0
      }
    }
  end
    
  w.watch

end
