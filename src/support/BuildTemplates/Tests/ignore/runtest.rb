#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_deep_sample
    pwd = Dir.pwd
    x=false
    output=""
    x, output = runCMake
    assert_equal(x, true, "cmake failed: >>>\n" + output + "<<<\n");
    x, output = doBuild
    assert_equal(x, true, "build failed: >>>\n" + output + "<<<\n");
    Dir.chdir(pwd)
    cleanUp
  end
end
