######################################################################
# customtest.rb encapsulates common processes for running cmake in
# a work dir, and invoking native build tools
#
# subclass it, and run "doBuild".
# doBuild will return the exit code of the build process (devenv or
# gmake).  after it's complete, you'll be sitting in the generated dist/
# directory at which point you can verify the build results in whatever
# way is apropriate for the test
######################################################################

require 'test/unit'
require 'pathname'
require 'fileutils'
require 'rbconfig'
include Config

module YBTTestTools
  WORKDIR_NAME = "work" 
  OUTPUT_FILE_NAME = "build.log" 

  # assume we're in the source directory.  run cmake in a work directory 
  # upon completion, we'll be left in the work directory.
  # returns an array, the first argument is the return code, the
  # second the output of the cmake command
  def runCMake
    workdir = WORKDIR_NAME

    # kill the working directory (perhaps from a previous build)
    cleanUp workdir

    # create the working directory and change into it
    FileUtils.mkdir(workdir)
    Dir.chdir(workdir) do
      x,output = runCommand("cmake ..")
    end
  end

  def cleanUp(workdir=WORKDIR_NAME)
    FileUtils.rm_rf(workdir)
  end

  # actually run a build!  we assume pwd is a work dir where you've
  # run cmake
  def doBuild(workdir=WORKDIR_NAME)
    Dir.chdir(workdir) do
      runCommand(discernBuildCommand())        
    end
  end
  
  def discernBuildCommand
    compileCommand = nil
    if CONFIG["arch"] =~ /win32/i  
      solutions = Dir.glob("*.sln")
      if solutions.length == 1
        # Use devenv.com rather than devenv.exe gets output to stdout.
        compileCommand = "devenv.com #{File.expand_path(solutions[0])}"
        compileCommand = compileCommand + " /build Debug"
      else
        puts "There are too many *.sln files in #{Dir.pwd}"
        raise
      end
    else
      compileCommand = `which gmake`.strip
      if File.executable?(compileCommand) == false
        compileCommand = `which make`.strip
      end

      if not File.executable?(compileCommand) 
        puts "No viable make program found!"
        raise
      end
    end
    compileCommand
  end

  def runCommand (command)
    command += " 2>&1" unless command =~ /2>&1/    
    output = File.new(OUTPUT_FILE_NAME, 'w')
    cmd = IO.popen(command, 'r')
    cmd.each { |line| output.puts line }
    status = Process.waitpid2(cmd.pid).at(1).exitstatus
    cmd.close
    output.close
    [status == 0, File.open(OUTPUT_FILE_NAME).read()]
  end
end
