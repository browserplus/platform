#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_deep_sample
    x=false
    output=""
    x, output = runCMake
    assert_equal(x, true, "cmake failed: >>>\n" + output + "<<<\n");
    x, output = doBuild
    assert_equal(x, true, "build failed: >>>\n" + output + "<<<\n");

    # now we've got a built tree.  let's assert that the files
    # we expect to be there are actually there 
    expected = Set.new [
                        "work/dist/samples/deepSample/CMakeLists.txt",
                        "work/dist/samples/deepSample/deepsample.cpp",
                        "work/dist/samples/deepSample/level1/level1.cpp",
                        "work/dist/samples/deepSample/level1/level1.h",
                        "work/dist/samples/deepSample/level1/level2/level2.h",
                        "work/dist/samples/deepSample/level1/level2/level2.cpp"
                       ];
    actual = Set.new
    Dir.glob(File.join("work", "dist", "**", "*")).each { |x| actual.add(x) }
    
    assert(expected.subset?(actual),
           "expected set:\n\n#{expected.inspect}\n\n" +
           "not in actual set\n\n#{actual.inspect}\n\n")

    cleanUp
  end
end
