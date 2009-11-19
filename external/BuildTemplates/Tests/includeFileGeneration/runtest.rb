#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_include_file_generation
    x=false
    output=""
    Dir.chdir("useee") do
      x, output = runCMake
      assert_equal(x, true, "cmake failed: >>>\n" + output + "<<<\n");
      x, output = doBuild
      assert_equal(x, true, "build failed: >>>\n" + output + "<<<\n");

    end
    Dir.chdir("user") do
      x, output = runCMake
      assert_equal(x, true, "cmake failed: >>>\n" + output + "<<<\n");
      x, output = doBuild
      assert_equal(x, true, "build failed: >>>\n" + output + "<<<\n");
      cleanUp
    end
    Dir.chdir("useee") do
      cleanUp
    end
    cleanUp
  end
end
