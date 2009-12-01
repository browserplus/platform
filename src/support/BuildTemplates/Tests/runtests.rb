#!/usr/bin/env ruby
#
# A simple ruby script to run the BuiltTemplates tests.
#
# if a "runtest.rb" file exists in the subdirectory, that file is
# executed and the return code is checked, if not, we run cmake in 
# DIRECTORY/work with "cmake ../CMakeLists.txt" and check the return
# code.
#
# If a directory name ends with _f, we test to ensure that cmake fails
#

require 'test/unit'
require 'pathname'
require 'fileutils'
require 'rbconfig'
require 'ybttesttools'
include Config

class CMakeTests < Test::Unit::TestCase
  include YBTTestTools

  TEST_SCRIPT_NAME = "runtest.rb"

  def test_runsubtests

    # find all subdirs
    Dir.glob("*").each do |file|
      if File.directory? file
        puts "Testing \"#{file}\""
        errorMsg = "Test failed: #{file}"
        if (File.executable?(File.join(file, TEST_SCRIPT_NAME)) ||
	    (CONFIG["arch"] =~ /win32/ && File.exist?(File.join(file, TEST_SCRIPT_NAME))))
          # ok, run the ruby test script
          pwd = Dir.pwd
          Dir.chdir(file)
          x = false
          begin
            x,output = runCommand("ruby " + File.join(".", TEST_SCRIPT_NAME))
            errorMsg = errorMsg + ":\n>>>\n" + output + "\n<<<\n" 
            assert_equal(true, x, errorMsg)
          ensure
            Dir.chdir(pwd)
          end
        else
          # build
          workdir = File.join(file, WORKDIR_NAME)
          pwd = Dir.pwd
          Dir.chdir(file)
          sourceDir = Dir.pwd
          begin
            x,output = runCMake
            errorMsg = errorMsg + ":\n>>>\n" + output + "\n<<<\n" 
            if file =~ /_f$/ 
              assert_equal(false, x, errorMsg)
            else 
              assert_equal(true, x, errorMsg)
            end
          ensure
            Dir.chdir(sourceDir)
            cleanUp
            Dir.chdir(pwd)            
          end
        end
      end
    end
  end
end



